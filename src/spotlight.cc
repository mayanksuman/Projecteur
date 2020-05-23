// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
#include "spotlight.h"

#include "deviceinput.h"
#include "virtualdevice.h"
#include "logging.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QSocketNotifier>
#include <QTextStream>
#include <QTimer>
#include <QVarLengthArray>

#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <linux/hidraw.h>
#include <unistd.h>

LOGGING_CATEGORY(device, "device")

// Function declaration to check for extra devices, defintion in generated source
bool isExtraDeviceSupported(quint16 vendorId, quint16 productId);
QString getExtraDeviceName(quint16 vendorId, quint16 productId);

namespace {
  // -----------------------------------------------------------------------------------------------
  // List of supported devices
  const std::array<Spotlight::SupportedDevice, 2> supportedDefaultDevices {{
    {0x46d, 0xc53e, false, "Logitech Spotlight (USB)"},
    {0x46d, 0xb503, true, "Logitech Spotlight (Bluetooth)"},
  }};

  // -----------------------------------------------------------------------------------------------
  bool isDeviceSupported(quint16 vendorId, quint16 productId)
  {
    const auto it = std::find_if(supportedDefaultDevices.cbegin(), supportedDefaultDevices.cend(),
    [vendorId, productId](const Spotlight::SupportedDevice& d) {
      return (vendorId == d.vendorId) && (productId == d.productId);
    });
    return (it != supportedDefaultDevices.cend()) || isExtraDeviceSupported(vendorId, productId);
  }

  // -----------------------------------------------------------------------------------------------
  bool isAdditionallySupported(quint16 vendorId, quint16 productId, const QList<Spotlight::SupportedDevice>& devices)
  {
    const auto it = std::find_if(devices.cbegin(), devices.cend(),
    [vendorId, productId](const Spotlight::SupportedDevice& d) {
      return (vendorId == d.vendorId) && (productId == d.productId);
    });
    return (it != devices.cend());
  }

  // -----------------------------------------------------------------------------------------------
  // Return the defined device name for vendor/productId if defined in
  // any of the supported device lists (default, extra, additional)
  QString getUserDeviceName(quint16 vendorId, quint16 productId, const QList<Spotlight::SupportedDevice>& additionalDevices)
  {
    const auto it = std::find_if(supportedDefaultDevices.cbegin(), supportedDefaultDevices.cend(),
    [vendorId, productId](const Spotlight::SupportedDevice& d) {
      return (vendorId == d.vendorId) && (productId == d.productId);
    });
    if (it != supportedDefaultDevices.cend() && it->name.size()) return it->name;

    auto extraName = getExtraDeviceName(vendorId, productId);
    if (!extraName.isEmpty()) return extraName;

    const auto ait = std::find_if(additionalDevices.cbegin(), additionalDevices.cend(),
    [vendorId, productId](const Spotlight::SupportedDevice& d) {
      return (vendorId == d.vendorId) && (productId == d.productId);
    });
    if (ait != additionalDevices.cend() && ait->name.size()) return ait->name;
    return QString();
  }

  // -----------------------------------------------------------------------------------------------
  quint16 readUShortFromDeviceFile(const QString& filename)
  {
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
      return f.readAll().trimmed().toUShort(nullptr, 16);
    }
    return 0;
  }

  // -----------------------------------------------------------------------------------------------
  quint64 readULongLongFromDeviceFile(const QString& filename)
  {
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
      return f.readAll().trimmed().toULongLong(nullptr, 16);
    }
    return 0;
  }

  // -----------------------------------------------------------------------------------------------
  QString readStringFromDeviceFile(const QString& filename)
  {
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
      return f.readAll().trimmed();
    }
    return QString();
  }

  // -----------------------------------------------------------------------------------------------
  QString readPropertyFromDeviceFile(const QString& filename, const QString& property)
  {
    QFile f(filename);
    if (f.open(QIODevice::ReadOnly)) {
      auto contents = f.readAll();
      QTextStream in(&contents, QIODevice::ReadOnly);
      while (!in.atEnd())
      {
        const auto line = in.readLine();
        if (line.startsWith(property) && line.size() > property.size() && line[property.size()] == '=')
        {
          return line.mid(property.size() + 1);
        }
      }
    }
    return QString();
  }

  // -----------------------------------------------------------------------------------------------
  Spotlight::Device deviceFromUEventFile(const QString& filename)
  {
    QFile f(filename);
    Spotlight::Device spotlightDevice;
    static const QString hid_id("HID_ID");
    static const QString hid_name("HID_NAME");
    static const QString hid_phys("HID_PHYS");
    static const std::array<const QString*, 3> properties = { &hid_id, &hid_name, &hid_phys };

    if (!f.open(QIODevice::ReadOnly)) {
      return spotlightDevice;
    }

    auto contents = f.readAll();
    QTextStream in(&contents, QIODevice::ReadOnly);
    while (!in.atEnd())
    {
      const auto line = in.readLine();
      for (const auto property : properties)
      {
        if (line.startsWith(property) && line.size() > property->size() && line[property->size()] == '=')
        {
          const QString value = line.mid(property->size() + 1);

          if (property == hid_id)
          {
            const auto ids = value.split(':');
            const auto busType = ids.size() ? ids[0].toUShort(nullptr, 16) : 0;
            switch (busType)
            {
              case BUS_USB: spotlightDevice.busType = Spotlight::Device::BusType::Usb; break;
              case BUS_BLUETOOTH: spotlightDevice.busType = Spotlight::Device::BusType::Bluetooth; break;
            }
            spotlightDevice.id.vendorId = ids.size() > 1 ? ids[1].toUShort(nullptr, 16) : 0;
            spotlightDevice.id.productId = ids.size() > 2 ? ids[2].toUShort(nullptr, 16) : 0;
          }
          else if (property == hid_name)
          {
            spotlightDevice.name = value;
          }
          else if (property == hid_phys)
          {
            spotlightDevice.id.phys = value.split('/').first();
          }
        }
      }
    }
    return spotlightDevice;
  }

} // --- end anonymous namespace


// -------------------------------------------------------------------------------------------------
Spotlight::Spotlight(QObject* parent, Options options)
  : QObject(parent)
  , m_options(std::move(options))
  , m_activeTimer(new QTimer(this))
  , m_connectionTimer(new QTimer(this))
{
  m_activeTimer->setSingleShot(true);
  m_activeTimer->setInterval(600);

  connect(m_activeTimer, &QTimer::timeout, this, [this](){
    m_spotActive = false;
    emit spotActiveChanged(false);
  });

  if (m_options.enableUInput) {
    m_virtualDevice = VirtualDevice::create();
  }
  else {
    logInfo(device) << tr("Virtual device initialization was skipped.");
  }

  m_connectionTimer->setSingleShot(true);
  // From detecting a change from inotify, the device needs some time to be ready for open
  // TODO: This interval seems to work, but it is arbitrary - there should be a better way.
  m_connectionTimer->setInterval(800);

  connect(m_connectionTimer, &QTimer::timeout, this, [this]() {
    logDebug(device) << tr("New connection check triggered");
    connectDevice();
  });

  QMetaType::registerComparators<Spotlight::DeviceId>();

  // Try to find already attached device(s) and connect to it.
  connectDevice();
  setupDevEventInotify();
}

// -------------------------------------------------------------------------------------------------
Spotlight::~Spotlight() = default;

// -------------------------------------------------------------------------------------------------
bool Spotlight::anySpotlightDeviceConnected() const
{
  if (m_device)
    return true;
  else
    return false;
}


// -------------------------------------------------------------------------------------------------
// This function connect to device with the given DeviceId
// If the proper DeviceId is not given or do not match then it connect to first scanned device.
int Spotlight::connectDevice(DeviceId id)
{
  auto scanResult = scanForDevices(m_options.additionalDevices);
  const bool anyConnectedBefore = anySpotlightDeviceConnected();

  if (scanResult.devices.isEmpty()){
    return -1;
  } else {
    if (id.vendorId != 0 && id.productId != 0){
      for (auto dev : scanResult.devices)
      {
        if (dev.id == id){
          m_device = std::make_shared<Device>(dev);
          break;
        }
      }
    }
    if (!m_device)
      m_device = std::make_shared<Device>(scanResult.devices.first());

    m_device->eventIM = std::make_shared<InputMapper>(m_virtualDevice);

    if (connectSubDevices() > 0){
      logInfo(device) << tr("Connected device: %1 (%2:%3)")
                           .arg(m_device->name)
                           .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                           .arg(m_device->id.productId, 4, 16, QChar('0'));
      emit deviceConnected(m_device->id, m_device->name);
      if (!anyConnectedBefore) emit anySpotlightDeviceConnectedChanged(true);
    }
  }
  return 0;
}


// -------------------------------------------------------------------------------------------------
int Spotlight::connectSubDevices(){
  if (!m_device || m_device->subDevices.size() == 0)
    return -1;

  int connectedEventSubDevice = ConnectEventSubDevices();
  int connectedHidrawSubDevice = ConnectHidrawSubDevices();
  // Ensures that at least one Event and Hidraw SubDevice are opened
  if (connectedEventSubDevice > 0 && connectedHidrawSubDevice > 0)
      return connectedEventSubDevice+connectedHidrawSubDevice;

  return 0;
}


// -------------------------------------------------------------------------------------------------
int Spotlight::ConnectHidrawSubDevices()
{
  int connectedHidrawSubDevice = 0;
  for (auto& subdev : m_device->subDevices)
  {
    if (subdev.type == SubDeviceType::Hidraw
        && subdev.DeviceWritable
        && subdev.DeviceReadable
        && subdev.DeviceFile.size() > 0){
      subdev.connection = openHidrawSubDeviceConnection(subdev);
      if (subdev.connection
          && subdev.connection->notifier
          && subdev.connection->notifier->isEnabled()
          && subdev.connection->fd
          && addInputHidrawHandler(subdev)){
        connectedHidrawSubDevice++;
        m_device->hidrwNode = subdev.connection->fd;
        break; // Only one readwrite hidraw device is enough.
      } else {
        logError(device) << tr("Connection failed for hidraw sub-device: %1 (%2:%3) %4")
                            .arg(m_device->name)
                            .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                            .arg(m_device->id.productId, 4, 16, QChar('0'))
                            .arg(subdev.DeviceFile);
      }
    }
  }
  return connectedHidrawSubDevice;
}


// -------------------------------------------------------------------------------------------------
int Spotlight::ConnectEventSubDevices()
{
  int connectedEventSubDevices = 0;
  for (auto& subdev : m_device->subDevices)
  {
    if (subdev.type == SubDeviceType::Event
        && subdev.DeviceReadable
        && subdev.DeviceFile.size() > 0){
      // Check if a connection for the path exists...
      if (subdev.connection
          && subdev.connection->notifier
          && subdev.connection->notifier->isEnabled()) {
        connectedEventSubDevices++;
        continue;
      }
      else {
        subdev.connection = openEventSubDeviceConnection(subdev);
        subdev.connection->im = m_device->eventIM;
        if (subdev.connection
            && subdev.connection->notifier
            && subdev.connection->notifier->isEnabled()
            && addInputEventHandler(subdev))
        {
          connectedEventSubDevices++;

          logDebug(device) << tr("Connected event sub-device: %1 (%2:%3) %4")
                              .arg(m_device->name)
                              .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                              .arg(m_device->id.productId, 4, 16, QChar('0'))
                              .arg(subdev.DeviceFile);
          emit subDeviceConnected(m_device->id, m_device->name, subdev.DeviceFile);
        }else
        {
          logError(device) << tr("Connection failed for event sub-device: %1 (%2:%3) %4")
                              .arg(m_device->name)
                              .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                              .arg(m_device->id.productId, 4, 16, QChar('0'))
                              .arg(subdev.DeviceFile);
        }
      }
    }
  }
  return connectedEventSubDevices;
}

// -------------------------------------------------------------------------------------------------
std::shared_ptr<Spotlight::SubDeviceConnection> Spotlight::openHidrawSubDeviceConnection(SubDevice& subdev)
{
  auto devicePath = subdev.DeviceFile;
  const int hrfd = ::open(devicePath.toLocal8Bit().constData(), O_RDWR, 0);

  if (hrfd < 0) {
    logDebug(device) << tr("Opening input event device failed:") << devicePath;
    return std::make_shared<SubDeviceConnection>();
  }

  auto connection = std::make_shared<SubDeviceConnection> (ConnectionMode::ReadOnly);
  connection->grabbed = false;

  fcntl(hrfd, F_SETFL, fcntl(hrfd, F_GETFL, 0) | O_NONBLOCK);
  if ((fcntl(hrfd, F_GETFL, 0) & O_NONBLOCK) == O_NONBLOCK) {
    subdev.deviceFlags |= DeviceFlag::NonBlocking;
  }
  connection->fd = hrfd;

  // Create socket notifier
  connection->notifier = std::make_unique<QSocketNotifier>(hrfd, QSocketNotifier::Read);
  QSocketNotifier* const notifier = connection->notifier.get();
  // Auto clean up and close descriptor on destruction of notifier
  connect(notifier, &QSocketNotifier::destroyed, [grabbed = connection->grabbed, notifier]() {
    if (grabbed) {
      ioctl(static_cast<int>(notifier->socket()), EVIOCGRAB, 0);
    }
    ::close(static_cast<int>(notifier->socket()));
  });

  return connection;
}

// -------------------------------------------------------------------------------------------------
std::shared_ptr<Spotlight::SubDeviceConnection> Spotlight::openEventSubDeviceConnection(SubDevice& subdev)
{
  auto devicePath = subdev.DeviceFile;
  const int evfd = ::open(devicePath.toLocal8Bit().constData(), O_RDONLY, 0);

  if (evfd < 0) {
    logDebug(device) << tr("Opening input event device failed:") << devicePath;
    return std::make_shared<SubDeviceConnection>();
  }

  unsigned long bitmask = 0;
  if (ioctl(evfd, EVIOCGBIT(0, sizeof(bitmask)), &bitmask) < 0)
  {
    ::close(evfd);
    logInfo(device) << tr("Cannot get device properties: %1 (%2:%3)")
                        .arg(devicePath)
                        .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                        .arg(m_device->id.productId, 4, 16, QChar('0'));
    return std::make_shared<SubDeviceConnection>();
  }

  auto connection = std::make_shared<SubDeviceConnection> (ConnectionMode::ReadOnly);

  connection->grabbed = [this, evfd, &devicePath]()
  {
    // Grab device inputs if a virtual device exists.
    if (m_virtualDevice)
    {
      const int res = ioctl(evfd, EVIOCGRAB, 1);
      if (res == 0) { return true; }

      // Grab not successful
      logError(device) << tr("Error grabbing device: %1 (return value: %2)").arg(devicePath).arg(res);
      ioctl(evfd, EVIOCGRAB, 0);
    }
    return false;
  }();

  if (!!(bitmask & (1 << EV_SYN))) subdev.deviceFlags |= DeviceFlag::SynEvents;
  if (!!(bitmask & (1 << EV_REP))) subdev.deviceFlags |= DeviceFlag::RepEvents;
  if (!!(bitmask & (1 << EV_REL)))
  {
    unsigned long relEvents = 0;
    ioctl(evfd, EVIOCGBIT(EV_REL, sizeof(relEvents)), &relEvents);
    const bool hasRelXEvents = !!(relEvents & (1 << REL_X));
    const bool hasRelYEvents = !!(relEvents & (1 << REL_Y));
    if (hasRelXEvents && hasRelYEvents) {
      subdev.deviceFlags |= DeviceFlag::RelativeEvents;
    }
  }

  fcntl(evfd, F_SETFL, fcntl(evfd, F_GETFL, 0) | O_NONBLOCK);
  if ((fcntl(evfd, F_GETFL, 0) & O_NONBLOCK) == O_NONBLOCK) {
    subdev.deviceFlags |= DeviceFlag::NonBlocking;
  }
  connection->fd = evfd;

  // Create socket notifier
  connection->notifier = std::make_unique<QSocketNotifier>(evfd, QSocketNotifier::Read);
  QSocketNotifier* const notifier = connection->notifier.get();
  // Auto clean up and close descriptor on destruction of notifier
  connect(notifier, &QSocketNotifier::destroyed, [grabbed = connection->grabbed, notifier]() {
    if (grabbed) {
      ioctl(static_cast<int>(notifier->socket()), EVIOCGRAB, 0);
    }
      ::close(static_cast<int>(notifier->socket()));
  });

  return connection;
}


// -------------------------------------------------------------------------------------------------
void Spotlight::removeDeviceConnection()
{
  m_device = std::make_unique<Device>();
}

// -------------------------------------------------------------------------------------------------
void Spotlight::removeSubDeviceConnection(QString DeviceFile)
{
  if (!m_device || m_device->subDevices.size() == 0)
    return;
  QList<SubDevice>::iterator it = m_device->subDevices.begin();
  while (it != m_device->subDevices.end()) {
    if (it->DeviceFile == DeviceFile){
      it = m_device->subDevices.erase(it);
      break;
    }
  }
}


// -------------------------------------------------------------------------------------------------
void Spotlight::onEventSubDeviceDataAvailable(int fd, SubDeviceConnection& connection, const SubDevice& dev)
{
  const bool isNonBlocking = !!(dev.deviceFlags & DeviceFlag::NonBlocking);
  while (true)
  {
    auto& buf = connection.inputBuffer;
    auto& ev = buf.current();
    if (::read(fd, &ev, sizeof(ev)) != sizeof(ev))
    {
      if (errno != EAGAIN)
      {
        connection.notifier->setEnabled(false);
        if (!anySpotlightDeviceConnected()) {
          emit anySpotlightDeviceConnectedChanged(false);
        }
        QTimer::singleShot(0, this, [this, devicePath=dev.DeviceFile](){
          removeSubDeviceConnection(devicePath);
        });
      }
      break;
    }
    ++buf;

    if (ev.type == EV_SYN)
    {
      // Check for relative events -> set Spotlight active
      const auto &first_ev = buf[0];
      const bool isMouseMoveEvent = first_ev.type == EV_REL
                                    && (first_ev.code == REL_X || first_ev.code == REL_Y);
      if (isMouseMoveEvent) {
        if (!connection.im->recordingMode()) // skip activation of spot in recording mode
        {
          if (!m_activeTimer->isActive()) {
            m_spotActive = true;
            emit spotActiveChanged(true);
          }
          m_activeTimer->start();
        }
        // Skip input mapping for mouse move events completely
        if (m_virtualDevice) m_virtualDevice->emitEvents(buf.data(), buf.pos());
      }
      else
      { // Forward events to input mapper for the device
        connection.im->addEvents(buf.data(), buf.pos());
      }
      buf.reset();
    }
    else if(buf.pos() >= buf.size())
    { // No idea if this will ever happen, but log it to make sure we get notified.
      logWarning(device) << tr("Discarded %1 input events without EV_SYN.").arg(buf.size());
      connection.im->resetState();
      buf.reset();
    }

    if (!isNonBlocking) break;
  } // end while loop
}


// -------------------------------------------------------------------------------------------------
bool Spotlight::addInputEventHandler(SubDevice& subdev)
{
  auto connection = subdev.connection;
  if (!connection ||subdev.type != SubDeviceType::Event || !connection->notifier || !connection->notifier->isEnabled()) {
    return false;
  }

  QSocketNotifier* const notifier = connection->notifier.get();
  connect(notifier, &QSocketNotifier::activated, this,
  [this, connection, subdev](int fd) {
    onEventSubDeviceDataAvailable(fd, *connection, subdev);
  });

  return true;
}


// -------------------------------------------------------------------------------------------------
bool Spotlight::addInputHidrawHandler(SubDevice& subdev)
{
  auto connection = subdev.connection;
  if (!connection ||subdev.type != SubDeviceType::Hidraw || !connection->notifier || !connection->notifier->isEnabled()) {
    return false;
  }

  QSocketNotifier* const notifier = connection->notifier.get();
  connect(notifier, &QSocketNotifier::activated, this, [](){}); // notifier do not do anything for now.
                                                                // According to HID++ 1.0 documentation: No input come on hidraw device,
                                                                // so it should not be mapped to input device.
                                                                // https://drive.google.com/folderview?id=0BxbRzx7vEV7eWmgwazJ3NUFfQ28&usp=sharing
                                                                // In future, new interface to process these command will be needed.

  return true;
}


// -------------------------------------------------------------------------------------------------
bool Spotlight::setupDevEventInotify()
{
  int fd = -1;
#if defined(IN_CLOEXEC)
  fd = inotify_init1(IN_CLOEXEC);
#endif
  if (fd == -1)
  {
    fd = inotify_init();
    if (fd == -1) {
      logError(device) << tr("inotify_init() failed. Detection of new attached devices will not work.");
      return false;
    }
  }
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  const int wd = inotify_add_watch(fd, "/dev/input", IN_CREATE | IN_DELETE);

  if (wd < 0) {
    logError(device) << tr("inotify_add_watch for /dev/input returned with failure.");
    return false;
  }

  const auto notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
  connect(notifier, &QSocketNotifier::activated, [this](int fd)
  {
    int bytesAvaibable = 0;
    if (ioctl(fd, FIONREAD, &bytesAvaibable) < 0 || bytesAvaibable <= 0) {
      return; // Error or no bytes available
    }
    QVarLengthArray<char, 2048> buffer(bytesAvaibable);
    const auto bytesRead = read(fd, buffer.data(), static_cast<size_t>(bytesAvaibable));
    const char* at = buffer.data();
    const char* const end = at + bytesRead;
    while (at < end)
    {
      const auto event = reinterpret_cast<const inotify_event*>(at);

      if ((event->mask & (IN_CREATE)) && QString(event->name).startsWith("event"))
      {
        // Trigger new device scan and connect if a new event device was created.
        m_connectionTimer->start();
      }

      at += sizeof(inotify_event) + event->len;
    }
  });

  connect(notifier, &QSocketNotifier::destroyed, [notifier]() {
    ::close(static_cast<int>(notifier->socket()));
  });
  return true;
}

Spotlight::DeviceId Spotlight::connectedDeviceId() const
{
  DeviceId res;
  if (anySpotlightDeviceConnected())  res = m_device->id;
  return res;
}


// -------------------------------------------------------------------------------------------------
Spotlight::ScanResult Spotlight::scanForDevices(const QList<SupportedDevice>& additionalDevices)
{
  constexpr char hidDevicePath[] = "/sys/bus/hid/devices";

  ScanResult result;
  const QFileInfo dpInfo(hidDevicePath);

  if (!dpInfo.exists()) {
    result.errorMessages.push_back(tr("HID device path '%1' does not exist.").arg(hidDevicePath));
    return result;
  }

  if (!dpInfo.isExecutable()) {
    result.errorMessages.push_back(tr("HID device path '%1': Cannot list files.").arg(hidDevicePath));
    return result;
  }


  QDirIterator hidIt(hidDevicePath, QDir::System | QDir::Dirs | QDir::Executable | QDir::NoDotAndDotDot);
  while (hidIt.hasNext())
  {
    hidIt.next();

    const QFileInfo uEventFile(QDir(hidIt.filePath()).filePath("uevent"));
    if (!uEventFile.exists()) continue;

    // Get basic information from uevent file
    Device newDevice = deviceFromUEventFile(uEventFile.filePath());
    const auto& deviceId = newDevice.id;
    // Skip unsupported devices
    if (deviceId.vendorId == 0 || deviceId.productId == 0) continue;
    if (!isDeviceSupported(deviceId.vendorId, deviceId.productId)
        && !(isAdditionallySupported(deviceId.vendorId, deviceId.productId, additionalDevices))) continue;

    // Check if device is already in list (and we have another sub-device for it)
    const auto find_it = std::find_if(result.devices.begin(), result.devices.end(),
    [&newDevice](const Device& existingDevice){
      return existingDevice.id == newDevice.id;
    });

    Device& rootDevice = [&find_it, &result, &newDevice, &additionalDevices]() -> Device&
    {
      if (find_it == result.devices.end())
      {
        newDevice.userName = getUserDeviceName(newDevice.id.vendorId, newDevice.id.productId, additionalDevices);
        result.devices.push_back(newDevice);
        return result.devices.last();
      }
      return *find_it;
    }();

    // Iterate over 'input' sub-dircectory, check for input-hid device nodes
    const QFileInfo inputSubdir(QDir(hidIt.filePath()).filePath("input"));
    if (inputSubdir.exists() || inputSubdir.isExecutable())
    {
      QDirIterator inputIt(inputSubdir.filePath(), QDir::System | QDir::Dirs | QDir::Executable | QDir::NoDotAndDotDot);
      while (inputIt.hasNext())
      {
        inputIt.next();

        if (readUShortFromDeviceFile(QDir(inputIt.filePath()).filePath("id/vendor")) != rootDevice.id.vendorId
            || readUShortFromDeviceFile(QDir(inputIt.filePath()).filePath("id/product")) != rootDevice.id.productId)
        {
          logDebug(device) << tr("Input device vendor/product id mismatch.");
          break;
        }

        SubDevice subDevice;
        subDevice.type = SubDeviceType::Event;
        QDirIterator dirIt(inputIt.filePath(), QDir::System | QDir::Dirs | QDir::Executable | QDir::NoDotAndDotDot);
        while (dirIt.hasNext())
        {
          dirIt.next();
          if (!dirIt.fileName().startsWith("event")) continue;
          subDevice.DeviceFile = readPropertyFromDeviceFile(QDir(dirIt.filePath()).filePath("uevent"), "DEVNAME");
          if (!subDevice.DeviceFile.isEmpty()) {
            subDevice.DeviceFile = QDir("/dev").filePath(subDevice.DeviceFile);
            break;
          }
        }

        if (subDevice.DeviceFile.isEmpty()) continue;
        subDevice.phys = readStringFromDeviceFile(QDir(inputIt.filePath()).filePath("phys"));

        // Check if device supports relative events
        const auto supportedEvents = readULongLongFromDeviceFile(QDir(inputIt.filePath()).filePath("capabilities/ev"));
        const bool hasRelativeEvents = !!(supportedEvents & (1 << EV_REL));
        // Check if device supports relative x and y event types
        const auto supportedRelEv = readULongLongFromDeviceFile(QDir(inputIt.filePath()).filePath("capabilities/rel"));
        const bool hasRelXEvents = !!(supportedRelEv & (1 << REL_X));
        const bool hasRelYEvents = !!(supportedRelEv & (1 << REL_Y));

        subDevice.hasRelativeEvents = hasRelativeEvents && hasRelXEvents && hasRelYEvents;

        const QFileInfo fi(subDevice.DeviceFile);
        subDevice.DeviceReadable = fi.isReadable();
        subDevice.DeviceWritable = fi.isWritable();

        rootDevice.subDevices.push_back(subDevice);
      }
    }
    else {
      // If input sub-directory was not there then
      // Iterate over 'hidraw' sub-dircectory, check for hidraw device node
      const QFileInfo hidrawSubdir(QDir(hidIt.filePath()).filePath("hidraw"));
      if (hidrawSubdir.exists() || hidrawSubdir.isExecutable()){
        QDirIterator hidrawIt(hidrawSubdir.filePath(), QDir::System | QDir::Dirs | QDir::Executable | QDir::NoDotAndDotDot);
        while (hidrawIt.hasNext())
        {
          hidrawIt.next();
          if (!hidrawIt.fileName().startsWith("hidraw")) continue;
          SubDevice subDevice;
          subDevice.type = SubDeviceType::Hidraw;
          subDevice.DeviceFile = readPropertyFromDeviceFile(QDir(hidrawIt.filePath()).filePath("uevent"), "DEVNAME");
          if (!subDevice.DeviceFile.isEmpty()) {
            subDevice.DeviceFile = QDir("/dev").filePath(subDevice.DeviceFile);
            const QFileInfo fi(subDevice.DeviceFile);
            subDevice.DeviceReadable = fi.isReadable();
            subDevice.DeviceWritable = fi.isWritable();
            rootDevice.subDevices.push_back(subDevice);
          }
        }
      }
    }
  }

  for (const auto& dev : result.devices)
  {
    const bool allReadable = std::all_of(dev.subDevices.cbegin(), dev.subDevices.cend(),
    [](const SubDevice& subDevice){
      return (subDevice.DeviceFile.isEmpty() || subDevice.DeviceReadable);
    });

    const bool allWriteable = std::all_of(dev.subDevices.cbegin(), dev.subDevices.cend(),
    [](const SubDevice& subDevice){
      return (subDevice.DeviceFile.isEmpty() || subDevice.DeviceWritable);
    });

    result.numDevicesReadable += allReadable;
    result.numDevicesWritable += allWriteable;
  }

  return result;
}

// -------------------------------------------------------------------------------------------------
int Spotlight::vibrateDevice(uint8_t strength){
  // send vibration packet to the device
  strength = (strength < 64)? 64 : strength;

  uint8_t vibration_data[] = {0x10, 0x01, 0x09, 0x11, 0x03, 0xe8, strength};
  return sendDatatoDevice(vibration_data, 7);
}

// -------------------------------------------------------------------------------------------------
int Spotlight::sendDatatoDevice(uint8_t data[], int data_len){
  if (!anySpotlightDeviceConnected()) return -1;
  int res;
  if (m_device->hidrwNode){
    res = ::write(m_device->hidrwNode, (char*)data, data_len);
    if (res < 0)
      logError(device) << tr("Failed to write on the hidraw device.");
    return res;
  }
  else return -1;
}
