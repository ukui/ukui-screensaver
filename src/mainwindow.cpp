#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QFile>
#include <QPainter>

#include "authdialog.h"
#include "virtualkeyboard.h"

//QString screenStates[] = {"UNDEFINED", "LOCKSCREEN", "XSCREENSAVER", "XSCREENSAVER_BY_IDLE"};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUI();

    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint
                        | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_DeleteOnClose);

    configuration = new Configuration();
//    monitorWatcher = new MonitorWatcher();
    authDialog = new AuthDialog(this);
    connect(authDialog, &AuthDialog::authenticateCompete, this, &MainWindow::close);

//    connect(monitorWatcher, &MonitorWatcher::monitorCountChanged, this, &MainWindow::onScreenCountChanged);

//    QDesktopWidget *desktop = QApplication::desktop();
//    connect(desktop, &QDesktopWidget::workAreaResized, this, &MainWindow::onScreenResized);
//    connect(desktop, &QDesktopWidget::resized, this, &MainWindow::onScreenResized);

//    monitorWatcher->start();
    pixmap.load(configuration->getBackground());

    lockscreenFollowCursor(cursor().pos());



    QFile qssFile(":/qss/assets/authdialog.qss");
    if(qssFile.open(QIODevice::ReadOnly)) {
        setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    //铺满所有显示器
    int totalWidth = 0, totalHeight = 0;
    for(auto screen : QGuiApplication::screens()) {
        totalWidth += screen->geometry().width();
        totalHeight += screen->geometry().height();
    }
    setGeometry(0, 0, totalWidth, totalWidth); /* Full screen */

    //显示系统时间
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]{
        QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
        ui->lblTime->setText(time);
    });

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->lblTime->setText(time);
    ui->lblTime->setStyleSheet("QLabel{color:white; font-size: 55px;}");
    ui->lblTime->adjustSize();
    timer->start(1000);

    QString date = QDate::currentDate().toString("yyyy/MM/dd dddd");
    qDebug() << "current date: " << date;
    ui->lblDate->setText(date);
    ui->lblDate->setStyleSheet("QLabel{color:white; font-size: 20px;}");
    ui->lblDate->adjustSize();

    //虚拟键盘
    vKeyboard = new VirtualKeyboard(this);
    vKeyboard->hide();
    connect(vKeyboard, &VirtualKeyboard::aboutToClose, vKeyboard, &VirtualKeyboard::hide);

    ui->btnKeyboard->setIcon(QIcon(":/image/assets/keyboard.png"));
    ui->btnKeyboard->setFixedSize(39, 39);
    ui->btnKeyboard->setIconSize(QSize(39, 39));
    ui->btnKeyboard->setFocusPolicy(Qt::NoFocus);
    connect(ui->btnKeyboard, &QPushButton::clicked, vKeyboard, &VirtualKeyboard::show);
}

/* lockscreen follows cursor */
void MainWindow::lockscreenFollowCursor(QPoint cursorPoint)
{
    QScreen *screen = NULL;
    for(auto s : QGuiApplication::screens()) {
        if (s->geometry().contains(cursorPoint)) {
            screen = s;
            break;
        }
    }
    //认证窗口
    int x = screen->geometry().x() + (screen->geometry().width() -
                authDialog->geometry().width()) / 2;
    int y = screen->geometry().height();
    authDialog->setGeometry(x, 0, authDialog->width(), y);
    qDebug() << authDialog->geometry();

    //系统时间
    x = screen->geometry().x();
    y = screen->geometry().y() + screen->geometry().height() - 150;
    ui->widgetTime->move(x, y);

    //虚拟键盘按钮
    x = x + screen->geometry().width() - 100;
    y = screen->geometry().y() + screen->geometry().height() - 100;
    ui->btnKeyboard->move(x, y);
}

/* Draw background image */
void MainWindow::paintEvent(QPaintEvent *event)
{
    (void)event;
    /*
     * MainWindow is stretched to all screens. We can draw background image
     * on it based on geometry of each screen.
     */
    for(QScreen *screen : QGuiApplication::screens()) {
        QPainter painter(this);
        painter.drawPixmap(screen->geometry(), pixmap);
    }
    return QWidget::paintEvent(event);
}

