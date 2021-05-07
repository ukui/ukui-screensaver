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

#ifndef POWERMANAGER_H
#define POWERMANAGER_H
#include <QWidget>
#include <QListWidget>
#include <QTime>

#define ITEM_WIDTH 168
#define ITEM_HEIGHT ITEM_WIDTH
#define ITEM_SPACING (ITEM_WIDTH/8)

class QListWidget;
class QListWidgetItem;
class QDBusInterface;
class PowerManager:public QListWidget
{
    Q_OBJECT

public:
    PowerManager(QWidget *parent = 0);
    void showNormalSize();
    void showSmallSize();
private:
    void initUI();
    QListWidget *list;
    QWidget *lockWidget;
    QWidget *switchWidget;
    QWidget *logoutWidget;
    QWidget *rebootWidget;
    QWidget *shutdownWidget;
    QWidget *suspendWidget;
    QWidget *hibernateWidget;
    QTime lasttime;
    QDBusInterface *sessionInterface;
    QDBusInterface *loginInterface;
    bool    canSuspend;
    bool    canHibernate;
    int     m_count;
private:
    void lockWidgetClicked();
    void switchWidgetClicked();
    void logoutWidgetCliced();
    void rebootWidgetClicked();
    void shutdownWidgetClicked();
    void suspendWidgetClicked();
    void hibernateWidgetClicked();

private Q_SLOTS:
    void powerClicked(QListWidgetItem *item);

Q_SIGNALS:
    void switchToUser();
    void lock();
};

#endif // POWERMANAGER_H
