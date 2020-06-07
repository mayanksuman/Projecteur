// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
#pragma once

#include <memory>

#include <QDataStream>
#include <QKeySequence>
#include <QObject>

class VirtualDevice;

// -------------------------------------------------------------------------------------------------
/// This is basically the input_event struct from linux/input.h without the time member
struct DeviceInputEvent
{
  DeviceInputEvent() = default;
  DeviceInputEvent(uint16_t type, uint16_t code, int32_t value) : type(type), code(code), value(value) {}
  DeviceInputEvent(const struct input_event& ie);
  DeviceInputEvent(const DeviceInputEvent&) = default;
  DeviceInputEvent(DeviceInputEvent&&) = default;

  DeviceInputEvent& operator=(const DeviceInputEvent&) = default;
  DeviceInputEvent& operator=(DeviceInputEvent&&) = default;

  uint16_t type;
  uint16_t code;
  int32_t  value;

  bool operator==(const DeviceInputEvent& o) const;
  bool operator!=(const DeviceInputEvent& o) const;
  bool operator==(const struct input_event& o) const;
  bool operator<(const DeviceInputEvent& o) const;
  bool operator<(const struct input_event& o) const;
};

// -------------------------------------------------------------------------------------------------
QDataStream& operator<<(QDataStream& s, const DeviceInputEvent& die);
QDataStream& operator>>(QDataStream& s, DeviceInputEvent& die);

// -------------------------------------------------------------------------------------------------
/// KeyEvent is a sequence of DeviceInputEvent.
using KeyEvent = std::vector<DeviceInputEvent>;

/// KeyEventSequence is a sequence of KeyEvents.
using KeyEventSequence = std::vector<KeyEvent>;
Q_DECLARE_METATYPE(KeyEventSequence);

// -------------------------------------------------------------------------------------------------
template<typename T>
QDataStream& operator<<(QDataStream& s, const std::vector<T>& container)
{
  s << quint32(container.size());
  for (const auto& item : container) {
    s << item;
  }
  return s;
}

template<typename T>
QDataStream& operator>>(QDataStream& s, std::vector<T>& container)
{
  quint32 size{};
  s >> size;
  container.resize(size);
  for (quint64 i = 0; i < size; ++i) {
    s >> container[i];
  }
  return s;
}

// -------------------------------------------------------------------------------------------------
QDebug operator<<(QDebug debug, const DeviceInputEvent &ie);
QDebug operator<<(QDebug debug, const KeyEvent &ke);

// -------------------------------------------------------------------------------------------------
class NativeKeySequence
{
public:
  enum Modifier : uint16_t {
    NoModifier  = 0,
    LeftCtrl    = 1 << 0,
    RightCtrl   = 1 << 1,
    LeftAlt     = 1 << 2,
    RightAlt    = 1 << 3,
    LeftShift   = 1 << 4,
    RightShift  = 1 << 5,
    LeftMeta    = 1 << 6,
    RightMeta   = 1 << 7,
  };

  NativeKeySequence();
  NativeKeySequence(NativeKeySequence&&) = default;
  NativeKeySequence(const NativeKeySequence&) = default;

  NativeKeySequence(const std::vector<int>& qtKeys, 
                    std::vector<uint16_t>&& nativeModifiers,
                    KeyEventSequence&& kes);

  NativeKeySequence& operator=(NativeKeySequence&&) = default;
  NativeKeySequence& operator=(const NativeKeySequence&) = default;
  bool operator==(const NativeKeySequence& other) const;
  bool operator!=(const NativeKeySequence& other) const;

  void swap(NativeKeySequence& other);
  int count() const;
  bool empty() const { return count() == 0; }
  const auto& keySequence() const { return m_keySequence; }
  const auto& nativeSequence() const { return m_nativeSequence; }
  QString toString() const;

  void clear();

  friend QDataStream& operator>>(QDataStream& s, NativeKeySequence& ks) {
    return s >> ks.m_keySequence >> ks.m_nativeSequence >> ks.m_nativeModifiers;
  }

  friend QDataStream& operator<<(QDataStream& s, const NativeKeySequence& ks) {
    return s << ks.m_keySequence << ks.m_nativeSequence << ks.m_nativeModifiers;
  }

  static QString toString(int qtKey, uint16_t nativeModifiers);
  static QString toString(const std::vector<int>& qtKey,
                          const std::vector<uint16_t>& nativeModifiers);

  struct predefined {
    static const NativeKeySequence& altTab();
    static const NativeKeySequence& altF4();
    static const NativeKeySequence& meta();
  };

private:
  QKeySequence m_keySequence;
  KeyEventSequence m_nativeSequence;
  std::vector<uint16_t> m_nativeModifiers;
};
Q_DECLARE_METATYPE(NativeKeySequence)

// -------------------------------------------------------------------------------------------------
struct Action
{
  enum class Type { KeySequence = 1, CyclePresets = 2 };
  virtual Type type() const = 0;
  virtual QDataStream& save(QDataStream&) const = 0;
  virtual QDataStream& load(QDataStream&) = 0;
  virtual bool empty() const = 0;
};

// -------------------------------------------------------------------------------------------------
struct KeySequenceAction : public Action
{
  KeySequenceAction() = default;
  KeySequenceAction(const NativeKeySequence& ks) : keySequence(ks) {}
  Type type() const override { return Type::KeySequence; }
  QDataStream& save(QDataStream& s) const override { return s << keySequence; }
  QDataStream& load(QDataStream& s) override { return s >> keySequence; }
  bool empty() const override { return keySequence.empty(); }
  bool operator==(const KeySequenceAction& o) const { return keySequence == o.keySequence; }

  NativeKeySequence keySequence;
};

// -------------------------------------------------------------------------------------------------
struct CyclePresetsAction : public Action
{
  Type type() const override { return Type::CyclePresets; }
  QDataStream& save(QDataStream& s) const override { return s; } // TODO
  QDataStream& load(QDataStream& s) override { return s; } // TODO
  bool empty() const override { return true; } // TODO
  bool operator==(const CyclePresetsAction&) const { return true; }
  // TODO...
};

// -------------------------------------------------------------------------------------------------
struct MappedAction
{
  bool operator==(const MappedAction& o) const;
  std::shared_ptr<Action> action;
};
Q_DECLARE_METATYPE(MappedAction);

QDataStream& operator>>(QDataStream& s, MappedAction& mia);
QDataStream& operator<<(QDataStream& s, const MappedAction& mia);

// -------------------------------------------------------------------------------------------------
class InputMapConfig : public std::map<KeyEventSequence, MappedAction>{};

// -------------------------------------------------------------------------------------------------
class InputMapper : public QObject
{
  Q_OBJECT

public:
  InputMapper(std::shared_ptr<VirtualDevice> virtualDevice, QObject* parent = nullptr);
  ~InputMapper();

  void resetState(); // Reset any stored sequence state.

  // input_events = complete sequence including SYN event
  void addEvents(const struct input_event input_events[], size_t num);

  bool recordingMode() const;
  void setRecordingMode(bool recording);

  int keyEventInterval() const;
  void setKeyEventInterval(int interval);

  std::shared_ptr<VirtualDevice> virtualDevice() const;
  bool hasVirtualDevice() const;

  void setConfiguration(const InputMapConfig& config);
  void setConfiguration(InputMapConfig&& config);
  const InputMapConfig& configuration() const;

signals:
  void configurationChanged();
  void recordingModeChanged(bool recording);
  void keyEventRecorded(const KeyEvent&);
  // Right befor first key event recorded:
  void recordingStarted();
  // After key sequence interval timer timout or max sequence length reached
  void recordingFinished(bool canceled); // canceled if recordingMode was set to false instead of interval time out

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};
