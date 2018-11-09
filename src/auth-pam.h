#ifndef AUTHPAM_H
#define AUTHPAM_H
#include "auth.h"
#include <QSocketNotifier>
#include <QList>

#include <security/pam_appl.h>

typedef struct pam_message PAM_MESSAGE;
typedef struct pam_response PAM_RESPONSE;

class AuthPAM : public Auth
{
    Q_OBJECT
public:
    AuthPAM(QObject *parent = nullptr);

    void authenticate(const QString &userName);
    void stopAuth();
    void respond(const QString &response);
    bool isAuthenticated();
    bool isAuthenticating();

private:
    void _authenticate(const char *userName);
    void _respond(const struct pam_response *response);

private Q_SLOTS:
    void onSockRead();

private:
    QString userName;
    pid_t pid;
    QSocketNotifier *notifier;
    int nPrompts;
    QStringList responseList;
    QList<PAM_MESSAGE> messageList;
    bool _isAuthenticated;  //认证结果
    bool _isAuthenticating;
};

#endif // AUTHPAM_H
