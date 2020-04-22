#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGSettings>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QSplitterHandle>
#include <QSplitter>
#include "sleeptime.h"
#include "chinesedate.h"
#include "mbackground.h"
#include "checkbutton.h"
class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(QWidget *parent = 0);
    ~Screensaver();

private:
    void initUI();
    void setDatelayout();
    void setSleeptime();
    void setCenterWidget();
    void updateDate();
    void setUpdateBackground();
    void setUpdateCenterWidget();

    QLabel *dateOfWeek;
    QLabel *dateOfLocaltime;
    QLabel *dateOfDay;
    QLabel *dateOfLunar;

    QWidget *centerWidget;

    QLabel *ubuntuKylinlogo;

    QWidget *timeLayout;
    SleepTime *sleepTime;
    QTimer *timer;
    ChineseDate *date;

    QGSettings *settings;
    QPixmap background;
    QString defaultBackground;

    QPushButton *escButton;
    QPushButton *settingsButton;
    QPushButton *WallpaperButton;
    QWidget *buttonWidget;
    QSettings *qsettings;

    MBackground *m_background;
    QString m_backgroundPath;

    QLabel *centerlabel1;
    QLabel *centerlabel2;
    QLabel *authorlabel;

    checkButton *checkSwitch;
    QLabel *autoSwitchLabel;
    QFrame *autoSwitch;

    QFrame *vboxFrame;
    bool isAutoSwitch;
    QTimer *m_timer;
    QGSettings *defaultSettings;

    int flag;
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
    void updateTime();
    void setDesktopBackground();
    void updateBackground();
    void updateCenterWidget(int index);
};

#endif // MAINWINDOW_H
