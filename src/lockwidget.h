/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QWidget>
#include <QResizeEvent>


namespace Ui {
class LockWidget;
}

class VirtualKeyboard;
class PowerManager;
class AuthDialog;
class Users;
class UserItem;
class DisplayManager;
class QMenu;

class LockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LockWidget(QWidget *parent = 0);
    ~LockWidget();
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

private Q_SLOTS:
    void onUserAdded(const UserItem &user);
    void onUserDeleted(const UserItem &user);
    void onUserMenuTrigged(QAction *action);
    void showVirtualKeyboard();
    void showPowerManager();
    void switchToGreeter();

protected:
     bool eventFilter(QObject *obj, QEvent *event);
     void resizeEvent(QResizeEvent *event);
     void keyReleaseEvent(QKeyEvent *e);

private:
    Ui::LockWidget      *ui;
    AuthDialog          *authDialog;
    VirtualKeyboard     *vKeyboard;
    PowerManager        *powermanager;
    QTimer              *timer;
    QMenu               *usersMenu;
    Users               *users;
    DisplayManager      *displayManager;
    int			 timeType;
};

#endif // LOCKWIDGET_H
