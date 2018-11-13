#ifndef USERS_H
#define USERS_H

#include <QObject>


struct UserItem
{
    QString name;
    QString realName;
    QString icon;
    quint64 uid;
    QString path;   //accounts service path
    friend QDebug operator<<(QDebug stream, const UserItem &user);
};

class QDBusInterface;
class QDBusObjectPath;
class Users : public QObject
{
    Q_OBJECT
public:
    explicit Users(QObject *parent = nullptr);
    QList<UserItem> getUsers();
    UserItem getUserByName(const QString &name);
    QString getDefaultIcon();

private:
    void loadUsers();
    UserItem getUser(const QString &path);
    int findUserByPath(const QString &path);

private Q_SLOTS:
    void onUserAdded(const QDBusObjectPath& path);
    void onUserDeleted(const QDBusObjectPath& path);

Q_SIGNALS:
    void userAdded(const UserItem &user);
    void userDeleted(const UserItem &user);

private:
    QDBusInterface      *actService;
    QList<UserItem>     users;
    QString             defaultIcon;
};

#endif // USERS_H
