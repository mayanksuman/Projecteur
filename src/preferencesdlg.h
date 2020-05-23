// This file is part of Projecteur - https://github.com/jahnf/projecteur - See LICENSE.md and README.md
#pragma once

#include <QDialog>
#include <QToolButton>

class QComboBox;
class QGroupBox;
class Settings;
class Spotlight;

#include "projecteur-icons-def.h"

// -------------------------------------------------------------------------------------------------
class IconButton : public QToolButton
{
  Q_OBJECT

public:
  IconButton(Font::Icon symbol, QWidget* parent = nullptr);
};

// -------------------------------------------------------------------------------------------------
class PreferencesDialog : public QDialog
{
  Q_OBJECT

public:
  enum class Mode : uint8_t{
    ClosableDialog,
    MinimizeOnlyDialog
  };

  explicit PreferencesDialog(Settings* settings, Spotlight* spotlight,
                             Mode = Mode::ClosableDialog, QWidget* parent = nullptr);
  virtual ~PreferencesDialog() override = default;

  bool dialogActive() const { return m_active; }
  Mode mode() const { return m_dialogMode; }
  void setMode(Mode dialogMode);

signals:
  void dialogActiveChanged(bool active);
  void testButtonClicked();
  void testVibrationButtonClicked(uint8_t strength);
  void exitApplicationRequested();

protected:
  virtual bool event(QEvent* event) override;
  virtual void closeEvent(QCloseEvent* e) override;

private:
  void setDialogActive(bool active);
  void setDialogMode(Mode dialogMode);

  QWidget* createSettingsTabWidget(Settings* settings);
  QGroupBox* createShapeGroupBox(Settings* settings);
  QGroupBox* createSpotGroupBox(Settings* settings);
  QGroupBox* createDotGroupBox(Settings* settings);
  QGroupBox* createBorderGroupBox(Settings* settings);
  QGroupBox* createCursorGroupBox(Settings* settings);
  QGroupBox* createZoomGroupBox(Settings* settings);
  QWidget* createPresetSelector(Settings* settings);
#if HAS_Qt5_X11Extras
  QWidget* createCompositorWarningWidget();
#endif
  QWidget* createLogTabWidget();
  QWidget* createTimerTabWidget();

private:
  QPushButton* m_closeMinimizeBtn = nullptr;
  QPushButton* m_exitBtn = nullptr;
  bool m_active = false;
  Mode m_dialogMode = Mode::ClosableDialog;
  quint32 m_discardedLogCount = 0;
};
