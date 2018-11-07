#ifndef AUTH_H
#define AUTH_H

#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#include <QObject>

class Auth : public QObject
{
    Q_OBJECT

    Q_ENUMS(PromptType MessageType)
public:
    explicit Auth(QObject *parent = nullptr)
        : QObject(parent)
    {

    }

    enum PromptType {
        PromptTypeQuestion,
        PromptTypeSecret
    };
    enum MessageType {
        MessageTypeInfo,
        MessageTypeError
    };


Q_SIGNALS:
    void showPrompt(const QString &prompt, Auth::PromptType type);
    void showMessage(const QString &message, Auth::MessageType type);
    void authenticateComplete();

public:
    virtual void authenticate(const QString &userName) = 0;
    virtual void respond(const QString &response) = 0;
    virtual bool isAuthenticated() = 0;
};

#endif // AUTH_H
