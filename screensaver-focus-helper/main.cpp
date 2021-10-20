#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QString>
#include <QCursor>
#include <QPalette>
#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget w;
    w.resize(QApplication::desktop()->size());
    w.setWindowFlags(Qt::X11BypassWindowManagerHint);

    QPalette pal(w.palette());
    pal.setColor(QPalette::Background, Qt::black); //设置背景黑色
    w.setAutoFillBackground(true);
    w.setPalette(pal);
    w.setCursor(Qt::BlankCursor);
    w.showFullScreen();
    a.exec();

    return 0;
}
