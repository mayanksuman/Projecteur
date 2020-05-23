// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
#pragma once

#include <QObject>
#include <QSocketNotifier>

#include <memory>
#include <map>
#include <vector>
#include <set>
#include <linux/input.h>

#include "enum-helper.h"

class InputMapper;
class QSocketNotifier;
class QTimer;
class VirtualDevice;

enum class DeviceFlag : uint32_t {
  NoFlags = 0,
  NonBlocking    = 1 << 0,
  SynEvents      = 1 << 1,
  RepEvents      = 1 << 2,
  RelativeEvents = 1 << 3,
};
ENUM(DeviceFlag, DeviceFlags)

// -----------------------------------------------------------------------------------------------
template<int Size, typename T = input_event>
struct InputBuffer {
  auto pos() const { return pos_; }
  void reset() { pos_ = 0; }
  auto data() { return data_.data(); }
  auto size() const { return data_.size(); }
  T& current() { return data_.at(pos_); }
  InputBuffer& operator++() { ++pos_; return *this; }
  T& operator[](size_t pos) { return data_[pos]; }
  T& first() { return data_[0]; }
private:
  std::array<T, Size> data_;
  size_t pos_ = 0;
};


/// Class to notify the application if the Logitech Spotlight and other supported devices
/// are connected and sending mouse move events. Used to turn the applications spot on or off.
class Spotlight : public QObject
{
  Q_OBJECT

public:
  struct SupportedDevice {
    quint16 vendorId;
    quint16 productId;
    bool isBluetooth = false;
    QString name = {};
  };

  struct Options {
    bool enableUInput = true; // enable virtual uinput device
    QList<Spotlight::SupportedDevice> additionalDevices;
  };

  explicit Spotlight(QObject* parent, Options options);
  virtual ~Spotlight();

  bool spotActive() const { return m_spotActive; }
  bool anySpotlightDeviceConnected() const;
  enum class ConnectionMode : uint8_t { ReadOnly, WriteOnly, ReadWrite };
  struct SubDeviceConnection {
    SubDeviceConnection() = default;
    SubDeviceConnection(const ConnectionMode mode)
            : mode(mode){};

    ConnectionMode mode;
    int fd = 0;
    bool grabbed = false;
    InputBuffer<12> inputBuffer;
    std::shared_ptr<InputMapper> im; // each device connection for a device shares the same input mapper
    std::unique_ptr<QSocketNotifier> notifier;
  };

  enum class SubDeviceType : uint8_t { Event, Hidraw };
  struct SubDevice {
    QString DeviceFile;
    QString phys;
	SubDeviceType type;
	DeviceFlags deviceFlags = DeviceFlags::NoFlags;
    std::shared_ptr<SubDeviceConnection> connection;
    bool hasRelativeEvents = false;
    bool DeviceReadable = false;
    bool DeviceWritable = false;
  };

  struct DeviceId {
    DeviceId() = default;
    DeviceId(const uint16_t vid, const uint16_t pid, QString phys)
          : vendorId(vid), productId(pid), phys(phys){};

    uint16_t vendorId = 0;
    uint16_t productId = 0;
    QString phys; // should be sufficient to differentiate between two devices of the same type
                  // - not tested, don't have two devices of any type currently.

    inline bool operator==(const DeviceId& rhs) const {
      return std::tie(vendorId, productId, phys) == std::tie(rhs.vendorId, rhs.productId, rhs.phys);
    }

    inline bool operator!=(const DeviceId& rhs) const {
      return std::tie(vendorId, productId, phys) != std::tie(rhs.vendorId, rhs.productId, rhs.phys);
    }

    inline bool operator<(const DeviceId& rhs) const {
      return std::tie(vendorId, productId, phys) < std::tie(rhs.vendorId, rhs.productId, rhs.phys);
    }
  };

  struct Device {
    enum class BusType : uint16_t { Unknown, Usb, Bluetooth };
    QString name;
    QString userName;
    DeviceId id;
    BusType busType = BusType::Unknown;
    QList<SubDevice> subDevices;
    std::shared_ptr<InputMapper> eventIM; // Sub-devices shares this input mapper
    int hidrwNode;
  };

  struct ScanResult {
    QList<Device> devices;
    quint16 numDevicesReadable = 0;
    quint16 numDevicesWritable = 0;
    QStringList errorMessages;
  };

  DeviceId connectedDeviceId() const;

  /// scan for supported devices and check if they are accessible
  static ScanResult scanForDevices(const QList<SupportedDevice>& additionalDevices = {});
  int vibrateDevice(uint8_t strength);
  int sendDatatoDevice(uint8_t data[], int data_len);

signals:
  void error(const QString& errMsg);
  void deviceConnected(const DeviceId& id, const QString& name);
  void deviceDisconnected(const DeviceId& id, const QString& name);
  void subDeviceConnected(const DeviceId& id, const QString& name, const QString& path);
  void subDeviceDisconnected(const DeviceId& id, const QString& name, const QString& path);
  void anySpotlightDeviceConnectedChanged(bool connected);
  void spotActiveChanged(bool isActive);

private:
  enum class ConnectionResult { CouldNotOpen, NotASpotlightDevice, Connected };
  ConnectionResult connectSpotlightDevice(const QString& devicePath, bool verbose = false);

  std::shared_ptr<SubDeviceConnection> openEventSubDeviceConnection(SubDevice&);
  std::shared_ptr<SubDeviceConnection> openHidrawSubDeviceConnection(SubDevice&);
  bool addInputEventHandler(SubDevice&);
  bool addInputHidrawHandler(SubDevice&);

  bool setupDevEventInotify();
  int connectDevice(DeviceId id = DeviceId(0, 0, ""));
  int connectSubDevices();
  int ConnectEventSubDevices();
  int ConnectHidrawSubDevices();
  void removeDeviceConnection();
  void removeSubDeviceConnection(QString DeviceFile);
  void onEventSubDeviceDataAvailable(int fd, SubDeviceConnection& connection, const SubDevice& dev);

  const Options m_options;

  QTimer* m_activeTimer = nullptr;
  QTimer* m_connectionTimer = nullptr;
  bool m_spotActive = false;
  std::shared_ptr<Device> m_device;
  std::shared_ptr<VirtualDevice> m_virtualDevice;
};

Q_DECLARE_METATYPE(Spotlight::DeviceId);
