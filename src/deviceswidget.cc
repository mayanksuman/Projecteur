// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
#include "deviceswidget.h"

#include "logging.h"
#include "spotlight.h"

#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QStackedLayout>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>

DECLARE_LOGGING_CATEGORY(preferences)

// ------------------------------------------------------------------------------------------------
namespace {
  QString descriptionString(const QString& name, const Spotlight::DeviceId& id) {
    return QString("%1 (%2:%3) [%4]").arg(name).arg(id.vendorId, 4, 16, QChar('0'))
                                     .arg(id.productId, 4, 16, QChar('0')).arg(id.phys);
  }

  const auto invalidDeviceId =  Spotlight::DeviceId(); // vendorId = 0, productId = 0
}

// ------------------------------------------------------------------------------------------------
DevicesWidget::DevicesWidget(Settings* /*settings*/, Spotlight* spotlight, QWidget* parent)
  : QWidget(parent)
  , m_devicesCombo(createDeviceComboBox(spotlight))
{
  const auto stackLayout = new QStackedLayout(this);
  const auto disconnectedWidget = createDisconnectedStateWidget();
  stackLayout->addWidget(disconnectedWidget);
  const auto deviceWidget = createDevicesWidget(spotlight);
  stackLayout->addWidget(deviceWidget);

  const bool anyDeviceConnected = spotlight->anySpotlightDeviceConnected();
  stackLayout->setCurrentWidget(anyDeviceConnected ? deviceWidget : disconnectedWidget);

  connect(spotlight, &Spotlight::anySpotlightDeviceConnectedChanged, this,
  [stackLayout, deviceWidget, disconnectedWidget](bool anyConnected){
    stackLayout->setCurrentWidget(anyConnected ? deviceWidget : disconnectedWidget);
  });
}

// ------------------------------------------------------------------------------------------------
const Spotlight::DeviceId& DevicesWidget::currentDevice() const
{
  return invalidDeviceId;
}

// ------------------------------------------------------------------------------------------------
QWidget* DevicesWidget::createDevicesWidget(Spotlight* /*spotlight*/)
{
  const auto dw = new QWidget(this);
  const auto vLayout = new QVBoxLayout(dw);
  const auto devHLayout = new QHBoxLayout();
  vLayout->addLayout(devHLayout);

  devHLayout->addWidget(new QLabel(tr("Device"), dw));
  devHLayout->addWidget(m_devicesCombo);
  devHLayout->setStretch(1, 1);

  vLayout->addSpacing(10);

  const auto tabWidget = new QTabWidget(dw);
  vLayout->addWidget(tabWidget);

  tabWidget->addTab(createInputMapperWidget(), tr("Input Mapping"));
  tabWidget->addTab(createDeviceInfoWidget(), tr("Device Info"));

  return dw;
}

// ------------------------------------------------------------------------------------------------
QWidget* DevicesWidget::createDeviceInfoWidget()
{
  const auto diWidget = new QWidget(this);
  const auto layout = new QHBoxLayout(diWidget);
  layout->addStretch(1);
  layout->addWidget(new QLabel(tr("Not yet implemented"), this));
  layout->addStretch(1);
  diWidget->setDisabled(true);
  return diWidget;
}

// ------------------------------------------------------------------------------------------------
QWidget* DevicesWidget::createInputMapperWidget()
{
  const auto imWidget = new QWidget(this);
  const auto layout = new QHBoxLayout(imWidget);
  layout->addStretch(1);
  layout->addWidget(new QLabel(tr("Not yet implemented"), this));
  layout->addStretch(1);
  imWidget->setDisabled(true);
  return imWidget;
}

// ------------------------------------------------------------------------------------------------
QComboBox* DevicesWidget::createDeviceComboBox(Spotlight* spotlight)
{
  const auto devicesCombo = new QComboBox(this);
  devicesCombo->setToolTip(tr("List of connected devices."));
  auto selDeviceId = spotlight->connectedDeviceId();
  auto scanResult = spotlight->scanForDevices();
  if (scanResult.devices.size() > 0){
    for (const auto& dev : scanResult.devices) {
      const auto data = QVariant::fromValue(dev.id);
      if (devicesCombo->findData(data) < 0) {
        devicesCombo->addItem(descriptionString(dev.name, dev.id), data);
        if (spotlight->anySpotlightDeviceConnected() && dev.id == selDeviceId){
          devicesCombo->setCurrentIndex(devicesCombo->findData(data));
        }
      }
    }
  }

  connect(spotlight, &Spotlight::deviceDisconnected, this,
  [devicesCombo](const Spotlight::DeviceId& id, const QString& /*name*/)
  {
    const auto idx = devicesCombo->findData(QVariant::fromValue(id));
    if (idx >= 0) {
      devicesCombo->removeItem(idx);
    }
  });

  connect(spotlight, &Spotlight::deviceConnected, this,
  [devicesCombo, spotlight](const Spotlight::DeviceId& id, const QString& name)
  {
    auto selDeviceId = spotlight->connectedDeviceId();
    const auto data = QVariant::fromValue(id);
    if (devicesCombo->findData(data) < 0) {
      devicesCombo->addItem(descriptionString(name, id), data);
      if (spotlight->anySpotlightDeviceConnected() && id == selDeviceId){
        devicesCombo->setCurrentIndex(devicesCombo->findData(data));
      }
    }
  });

  connect(devicesCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
  [this, devicesCombo](int index)
  {
    if (index < 0) {
      emit currentDeviceChanged(invalidDeviceId);
      return ;
    }

    const auto devId = qvariant_cast<Spotlight::DeviceId>(devicesCombo->itemData(index));
    emit currentDeviceChanged(devId);
  });

  return devicesCombo;
}

// ------------------------------------------------------------------------------------------------
QWidget* DevicesWidget::createDisconnectedStateWidget()
{
  const auto stateWidget = new QWidget(this);
  const auto hbox = new QHBoxLayout(stateWidget);
  const auto label = new QLabel(tr("No devices connected."), stateWidget);
  label->setToolTip(label->text());
  auto icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
  const auto iconLabel = new QLabel(stateWidget);
  iconLabel->setPixmap(icon.pixmap(16,16));
  hbox->addStretch();
  hbox->addWidget(iconLabel);
  hbox->addWidget(label);
  hbox->addStretch();
  return stateWidget;
}
