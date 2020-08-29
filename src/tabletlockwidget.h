#ifndef TABLETLOCKWIDGET_H
#define TABLETLOCKWIDGET_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include <QResizeEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class VirtualKeyboard;
class PowerManager;
class AuthDialog;
class Users;
class UserItem;
class DisplayManager;
class QMenu;
class WeatherManager;

class TabletLockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabletLockWidget(QWidget *parent = nullptr);
    ~TabletLockWidget();

    void closeEvent(QCloseEvent *event);
    void startAuth();
    void stopAuth();

Q_SIGNALS:
    void closed();
    void capsLockChanged();

private:
    void initUI();
    void initUserMenu();
    void setVirkeyboardPos();

    void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void onUserAdded(const UserItem &user);
    void onUserDeleted(const UserItem &user);
    void onUserMenuTrigged(QAction *action);
    void showVirtualKeyboard();
    void showPowerManager();
    void switchToGreeter();
    void getWeatherFinish(QString city, QString cond, QString tmp);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    AuthDialog          *authDialog;
    VirtualKeyboard     *vKeyboard;
    PowerManager        *powermanager;
    QTimer              *timer;
    QMenu               *usersMenu;
    Users               *users;
    DisplayManager      *displayManager;
    WeatherManager      *weatherManager;

    QWidget *m_widget_center;
    QVBoxLayout *m_layout_center; //中心布局：包括天气和时间的显示

    QWidget *m_widget_weather;
    QHBoxLayout *m_layout_weather;
    QLabel *m_label_weather_icon; //32*32
    QLabel *m_label_weather_location_category; //天气的位置和种类描述 116*38
    QLabel *m_label_weather_temperatrue; //42*31

    QLabel *m_label_time;
    QLabel *m_label_date;


    //底部布局
    QWidget *m_widget_buttom;
    QLabel *m_label_notice_icon;
    QLabel *m_label_notice_message;
    QHBoxLayout *m_layout_notice;
    QWidget *m_widget_notice;

    QLabel *m_label_unlock_guide;
    QPushButton *m_pb_up_unlock;

    QPushButton *m_pb_login;

    QGridLayout *m_layout_buttom;

    //底部布局2
    QWidget *m_widget_buttom2;

    QWidget *m_widget_top_of_buttom;


};

#endif // TABLETLOCKWIDGET_H
