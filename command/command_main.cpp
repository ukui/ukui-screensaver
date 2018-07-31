#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QDebug>


int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Start command for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption({"l", QStringLiteral("lock")},
                                  QCoreApplication::translate("main", "lock the screen immediately"));
    parser.addOption(lockOption);
    parser.process(a);

    if(!parser.isSet(lockOption))
        return -1;

    QDBusInterface *interface = new QDBusInterface("cn.kylinos.ScreenSaver",
                                                   "/",
                                                   "cn.kylinos.ScreenSaver");
    QDBusMessage msg = interface->call("Lock");
    if(msg.type() == QDBusMessage::ErrorMessage)
        qDebug() << msg.errorMessage();

    return 0;
}

