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

  //Initialize libhidapi
  int res = hid_init();
  if (res < 0)
    logError(device) << tr("hidapi library intialization failed.");

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
    if (anyConnectedBefore)
      removeDeviceConnection();
    return -1;
  } else {
    if (anyConnectedBefore){
      const bool connectedDeviceExist = std::all_of(scanResult.devices.cbegin(), scanResult.devices.cend(),
        [this](const Device& dev){
          return (dev.id == m_device->id);
      });
      if (!connectedDeviceExist) removeDeviceConnection();
      else return 0;
    }
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

    if (connectEventSubDevices()){
      m_device->hasHID = openHID();
      QTimer::singleShot(0, this, [this, anyConnectedBefore, id = m_device->id,
      devName = m_device->userName.isEmpty()?m_device->name:m_device->userName](){
        logInfo(device) << tr("Connected device: %1 (%2:%3)")
                           .arg(devName)
                           .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                           .arg(m_device->id.productId, 4, 16, QChar('0'));
        emit deviceConnected(m_device->id, devName);
        if (!anyConnectedBefore) emit anySpotlightDeviceConnectedChanged(true);
        if (m_device->hasHID)
          emit connectedDeviceSupportVibration(true);
        else
          emit connectedDeviceSupportVibration(false);
      });
    }
  }
  return 0;
}

// -------------------------------------------------------------------------------------------------
int Spotlight::connectEventSubDevices()
{
  int connectedEventSubDevices = 0;
  auto devName = m_device->userName.isEmpty()?m_device->name:m_device->userName;
  for (auto& subdev : m_device->eventSubDevices)
  {
    if (subdev.DeviceReadable
        && subdev.DeviceFile.size() > 0){
      // Check if a connection for the path exists...
      if (subdev.connection
          && subdev.connection->notifier
          && subdev.connection->notifier->isEnabled()) {
        connectedEventSubDevices++;
        continue;
      } else {
        subdev.connection = openEventSubDeviceConnection(subdev);
        subdev.connection->im = m_device->eventIM;
        if (subdev.connection
            && subdev.connection->notifier
            && subdev.connection->notifier->isEnabled()
            && addInputEventHandler(subdev))
        {
          connectedEventSubDevices++;

          logDebug(device) << tr("Connected event sub-device: %1 (%2:%3) %4")
                              .arg(devName)
                              .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                              .arg(m_device->id.productId, 4, 16, QChar('0'))
                              .arg(subdev.DeviceFile);
          emit subDeviceConnected(m_device->id, devName, subdev.DeviceFile);
        }else
        {
          logError(device) << tr("Connection failed for event sub-device: %1 (%2:%3) %4")
                              .arg(devName)
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
bool Spotlight::openHID()
{
  if (anySpotlightDeviceConnected() && m_device->eventSubDevices.size()){
    struct hid_device_info *cur_dev;
    struct hid_device_info *devices=NULL;
    hid_free_enumeration(devices);
    devices = hid_enumerate(m_device->id.vendorId, m_device->id.productId);
    cur_dev = devices;
    while (cur_dev) {
      if (cur_dev -> interface_number == 2){
        hid_device* hidrwNode = hid_open_path(cur_dev->path);
        if (!hidrwNode) {
          printf("Unable To Connect to Device");
          return false;
        } else {
          hid_set_nonblocking(hidrwNode, 1);
          m_device->hidrwNode = hidrwNode;
          return true;
        }
      }
      cur_dev = cur_dev->next;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------
std::shared_ptr<Spotlight::SubDeviceConnection> Spotlight::openEventSubDeviceConnection(EventSubDevice& subdev)
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

  auto connection = std::make_shared<SubDeviceConnection> ();

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
  if (!m_device) return;
  // Remove Event Sub devices
  if (m_device->eventSubDevices.size())
    for(auto& subdev: m_device->eventSubDevices)
      removeEventSubDeviceConnection(subdev.DeviceFile);
  //Remove HID handle and connection
  if (m_device->hasHID){
    hid_close(m_device->hidrwNode);
    //m_device->hidConnection->notifier->setEnabled(false);
  }
  //Log the disconnection
  auto devName = m_device->userName.isEmpty()?m_device->name:m_device->userName;
  logInfo(device) << tr("Disconnected device: %1 (%2:%3)")
                     .arg(devName)
                     .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                     .arg(m_device->id.productId, 4, 16, QChar('0'));
  // Disconnection Signal
  emit deviceDisconnected(m_device->id, devName);
  // Remove the device
  m_device = nullptr;
}

// -------------------------------------------------------------------------------------------------
void Spotlight::removeEventSubDeviceConnection(QString DeviceFile)
{
  if (!m_device || m_device->eventSubDevices.size() == 0) return;
  auto devName = m_device->userName.isEmpty()?m_device->name:m_device->userName;

  QList<EventSubDevice>::iterator it;
  for (it = m_device->eventSubDevices.begin();it != m_device->eventSubDevices.end(); it++) {
    if (it->DeviceFile == DeviceFile){
      emit subDeviceDisconnected(m_device->id, devName, it->DeviceFile);
      logDebug(device) << tr("Disconnected sub-device: %1 (%2:%3) %4")
                          .arg(devName)
                          .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                          .arg(m_device->id.productId, 4, 16, QChar('0'))
                          .arg(it->DeviceFile);
      it->connection->notifier->setEnabled(false);
      if (it->connection->fd){
        if(it->connection->grabbed) ioctl(it->connection->fd, EVIOCGRAB, 0);
        ::close(it->connection->fd);
      }
      it = m_device->eventSubDevices.erase(it);
      break;
    }
  }
}


// -------------------------------------------------------------------------------------------------
void Spotlight::onEventSubDeviceDataAvailable(int fd, SubDeviceConnection& connection, const EventSubDevice& dev)
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
          removeDeviceConnection();
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
bool Spotlight::addInputEventHandler(EventSubDevice& subdev)
{
  auto connection = subdev.connection;
  if (!connection || !connection->notifier || !connection->notifier->isEnabled()) {
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


// -------------------------------------------------------------------------------------------------
Spotlight::DeviceId Spotlight::connectedDeviceId() const
{
  if (anySpotlightDeviceConnected()) return m_device->id;
  return DeviceId(0, 0, "");
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

        EventSubDevice subDevice;
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

        rootDevice.eventSubDevices.push_back(subDevice);
      }
    }
  }

  for (const auto& dev : result.devices)
  {
    const bool allReadable = std::all_of(dev.eventSubDevices.cbegin(), dev.eventSubDevices.cend(),
    [](const EventSubDevice& subDevice){
      return (subDevice.DeviceFile.isEmpty() || subDevice.DeviceReadable);
    });

    const bool allWriteable = std::all_of(dev.eventSubDevices.cbegin(), dev.eventSubDevices.cend(),
    [](const EventSubDevice& subDevice){
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
  int res = sendDatatoDevice(vibration_data, 7);
  if (res < 0)
    logError(device) << tr("Attempt to vibrate device failed.");
  return res;
}

// -------------------------------------------------------------------------------------------------
int Spotlight::sendDatatoDevice(uint8_t data[], int data_len){
  if (!anySpotlightDeviceConnected()) return -1;
  auto devName = m_device->userName.isEmpty()?m_device->name:m_device->userName;
  int res = -1;
  if (m_device->hasHID){
    res = hid_write(m_device->hidrwNode, data, data_len);
    if (res < 0)
      logError(device) << tr("Write Failed on the hidraw node of device: %1 (%2:%3)")
                          .arg(devName)
                          .arg(m_device->id.vendorId, 4, 16, QChar('0'))
                          .arg(m_device->id.productId, 4, 16, QChar('0'));
  }
  return res;
}
