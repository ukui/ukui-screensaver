#include "digitalkeyboard.h"

#include <QGridLayout>
#include <QVariant>
#include <QDebug>

DigitalKeyBoard::DigitalKeyBoard(QWidget *parent):
    QWidget(parent)
{
    initUI();
    initConnect();
    setQSS();
}

void DigitalKeyBoard::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    QGridLayout* mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    for(int i = 1;i <= 9;i++)
    {
        m_pNumerPressBT[i] = new QPushButton(this);
        m_pNumerPressBT[i]->setText(QString(QChar(i + '0')));
        m_pNumerPressBT[i]->setProperty("class", "numberPressBT");
        m_pNumerPressBT[i]->setFocusPolicy(Qt::NoFocus);
        mainLayout->addWidget(m_pNumerPressBT[i], (i - 1) / 3, (i - 1) % 3);
    }
    m_pNumerPressBT[0] = new QPushButton(this);
    m_pNumerPressBT[0]->setText(QString(QChar('0')));
    m_pNumerPressBT[0]->setFocusPolicy(Qt::NoFocus);
    m_pNumerPressBT[0]->setProperty("class", "numberPressBT");
    mainLayout->addWidget(m_pNumerPressBT[0], 3, 1);

    m_pNumerPressBT[10] = new QPushButton(this);
    m_pNumerPressBT[10]->setIcon(QPixmap(":/image/assets/intel/icon-backspace.png"));
    m_pNumerPressBT[10]->setIconSize(QSize(30, 30));
    m_pNumerPressBT[10]->setFocusPolicy(Qt::NoFocus);
    m_pNumerPressBT[10]->setProperty("class", "numberPressBT");
    mainLayout->addWidget(m_pNumerPressBT[10], 3, 2);

    m_pNumerPressBT[11] = new QPushButton(this);
    m_pNumerPressBT[11]->setText("清空");
    m_pNumerPressBT[11]->setFocusPolicy(Qt::NoFocus);
    m_pNumerPressBT[11]->setProperty("class", "numberPressBT");
    mainLayout->addWidget(m_pNumerPressBT[11], 3, 0);

    mainLayout->setSpacing(16);
    mainLayout->setVerticalSpacing(16);
}

void DigitalKeyBoard::initConnect()
{
    connect(m_pNumerPressBT[0], &QPushButton::clicked, this, [=](){ qDebug() << "DigitalKeyBoard press";emit numbersButtonPress(0); });
    connect(m_pNumerPressBT[1], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(1); });
    connect(m_pNumerPressBT[2], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(2); });
    connect(m_pNumerPressBT[3], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(3); });
    connect(m_pNumerPressBT[4], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(4); });
    connect(m_pNumerPressBT[5], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(5); });
    connect(m_pNumerPressBT[6], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(6); });
    connect(m_pNumerPressBT[7], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(7); });
    connect(m_pNumerPressBT[8], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(8); });
    connect(m_pNumerPressBT[9], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(9); });
    connect(m_pNumerPressBT[10], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(10); });
    connect(m_pNumerPressBT[11], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(11); });
}

void DigitalKeyBoard::setQSS()
{
    setStyleSheet(".numberPressBT{"
                      "font-size:20px;"
                      "min-width:96px;"
                      "min-height:64px;"
                      "max-width:96px;"
                      "max-height:64px;"
                      "background:rgba(255, 255, 255, 15);"
                      "border-radius:16px;"
                      "}"
                      ".numberPressBT:hover{"
                      "background:rgba(255, 255, 255, 35);"
                      "}"
                      ".numberPressBT:pressed{"
                      "background:rgba(255, 255, 255, 5);"
                      "}");
}


void DigitalKeyBoard::onKeyReleaseEvent(QKeyEvent *event)
{
//    switch(event->key())
//    {
//    case Qt::Key_0:
//        emit numbersButtonPress(0);
//        break;
//    case Qt::Key_1:
//        emit numbersButtonPress(1);
//        break;
//    case Qt::Key_2:
//        emit numbersButtonPress(2);
//        break;
//    case Qt::Key_3:
//        emit numbersButtonPress(3);
//        break;
//    case Qt::Key_4:
//        emit numbersButtonPress(4);
//        break;
//    case Qt::Key_5:
//        emit numbersButtonPress(5);
//        break;
//    case Qt::Key_6:
//        emit numbersButtonPress(6);
//        break;
//    case Qt::Key_7:
//        emit numbersButtonPress(7);
//        break;
//    case Qt::Key_8:
//        emit numbersButtonPress(8);
//        break;
//    case Qt::Key_9:
//        emit numbersButtonPress(9);
//        break;
//    case Qt::Key_Backspace:
//        emit numbersButtonPress(10);
//        break;
//    default:
//        break;
//    }
}

