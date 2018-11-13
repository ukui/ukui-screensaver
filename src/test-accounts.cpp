#include <QCoreApplication>
#include "users.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Users users;

    qDebug() << users.getUserByName("kylin");

    return 0;
}
