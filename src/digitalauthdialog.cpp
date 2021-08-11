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
 * Authors: ZHAI Kang-ning <zhaikangning@kylinos.cn>
**/
#include "digitalauthdialog.h"
#include <QDebug>
#include <cmath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "auth-pam.h"
#include "permissioncheck.h"
#include "eduplatforminterface.h"

DigitalAuthDialog::DigitalAuthDialog(QWidget *parent) : QWidget(parent),
    m_bgColor("#000000"), //整个wigidt背景颜色
    m_outCircleColor("#000000"),
    m_tipLineColor("#ff0000"),
    m_circleRingColor("#ddffff"),
    m_radius(40),
    m_margin(5),
    m_pressed(false),
    m_showText(true),
    m_inputPassword(""),
    m_circleColor("#ffffff"),//按钮的颜色
    m_statusRadius(8),
    m_auth(new AuthPAM(this)),
    m_isAuthing(false),
    m_buttonWidth(96),
    m_buttonHeight(64),
    m_buttonRadius(16),
    m_buttonStep(16),
    m_resetPassword(""),
    m_resetOrAuthFailure(new QLabel(this)),
    m_digitalKeyBoard(new DigitalKeyBoard(this))
{
    //获取键盘事件 此操作会使得其他控件无法获取键盘事件,使用releaseKeyboard可释放
//    grabKeyboard();
    m_posInCircle.isIn = false;
//    qDebug() <<"-----------init width=" << width() << ",height=" << height();
    m_title = new QLabel(tr("LoginByUEdu"), this);
    m_title->setStyleSheet("font-size:30px;color:#ffffff");
    m_title->adjustSize();

    m_labelReset = new MyLabel(tr("ResetPWD?"), this);
    m_labelReset->setStyleSheet("QLabel{font-size:16px;color:rgba(255, 255, 255, 45);}"
                                "QLabel:hover{color:rgba(255, 255, 255, 100);}");
    m_labelReset->adjustSize();

    m_authMessage = new QLabel(this);
    m_authMessage->setStyleSheet("font-size:14px;color:#ffffff");

    //设置字体大小
    QFont f;
    f.setPixelSize(m_radius * 24/40);
    setFont(f);

    //调整适当的大小
    resize(350,600);
    m_title->setGeometry((this->width() - m_title->width())/2, 6, m_title->width(), m_title->height());
    m_labelReset->setGeometry((this->width()- m_labelReset->width())/2, this->height() - 120, m_labelReset->width(),m_labelReset->height());


    setMouseTracking(true);
    connect(m_digitalKeyBoard, &DigitalKeyBoard::numbersButtonPress, this, &DigitalAuthDialog::onNumerPress);
    connect(m_auth, &Auth::showMessage, this, &DigitalAuthDialog::onShowMessage);
    connect(m_auth, &Auth::showPrompt, this, &DigitalAuthDialog::onShowPrompt);
    connect(m_auth, &Auth::authenticateComplete, this, &DigitalAuthDialog::onAuthComplete);
    connect(m_labelReset, &MyLabel::onClick, this, [=]{
//        qDebug() << "-------------------onClick";
        m_title->setText(tr("SetNewUEduPWD"));
        m_title->adjustSize();
        m_resetOrAuthFailure->clear();
        m_title->setGeometry((this->width() - m_title->width())/2, 5, m_title->width(), m_title->height());
        m_inputPassword.clear();
        m_loginType = LoginType::RESET;
        m_labelReset->hide();
//        PermissionCheck *check = new PermissionCheck(this);
//        QVBoxLayout *v = new QVBoxLayout(this);
//        v->addWidget(check);
//        this->hide();
        Q_EMIT requestPasswordReset();
    });

//    m_timer = new QTimer(this);
//    connect(m_timer,&QTimer::timeout,this,[=]{
//        if(m_isAuthing)
//        {
//            qDebug() << "Auth respond timeout , restart";
//            startAuth();
//        }
//    });

    m_digitalKeyBoard->move(9,143);
}

DigitalAuthDialog::~DigitalAuthDialog()
{

}

void DigitalAuthDialog::onNumerPress(int btn_id)
{
    qDebug() << btn_id;
    //nothing
    if(m_authMessage)
        cleanMessage();
    if(m_resetOrAuthFailure)
        m_resetOrAuthFailure->clear();
    m_pressed = true;

    if(m_isAuthing)
    {
        qWarning() << "authenticating";
        return;
    }

    if(btn_id == 11)
    {
        //点击了清空按钮
        m_inputPassword.clear();
    } else if (btn_id == 10)
    {
        if (!m_inputPassword.isEmpty())
            m_inputPassword.remove(m_inputPassword.size() -1, 1);
    } else {
        int num = btn_id;
        m_inputPassword.append(QString::number(num == 11 ? 0 : num));
        if (m_inputPassword.size() >= 6)
        {
            checkPassword();
        }
    }
    repaint();
}

void DigitalAuthDialog::paintEvent(QPaintEvent *event)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
//    qDebug() <<"-----------width=" << width << ",height=" << height << ",side=" << side;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width / 2, height / 2);
    painter.scale(side / 200, side / 200);

//    drawBg(&painter);
    drawStatusCircle(&painter);
////    drawOutCircle(&painter);
//    drawFilledCircle(&painter);
////    drawTipLine(&painter);
//    drawInCircle(&painter);
////    drawJoinLine(&painter);
//    drawHoverCircle(&painter);
}

//画背景颜色
void DigitalAuthDialog::drawBg(QPainter *painter)
{
    painter->save();

    int width = this->width();
    int height = this->height();

    painter->setPen(Qt::NoPen);
    m_bgColor.setAlpha(0);
    painter->setBrush(m_bgColor);
    painter->drawRect(-width / 2, -height / 2, width, height);

    painter->restore();
}

//绘制按钮的颜色
void DigitalAuthDialog::drawFilledCircle(QPainter *painter)
{
    painter->save();
    m_circleColor.setAlphaF(0.15);
    QPen pen(m_circleColor);
    painter->setPen(pen);
    painter->setBrush(m_circleColor);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int x = -3 * (m_buttonWidth / 2) + j * m_buttonWidth + (j - 1) * m_buttonStep;
            int y = -2 * m_buttonHeight + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
            QRect rect(x, y, m_buttonWidth, m_buttonHeight);
//            painter->drawEllipse(rect);
            painter->drawRoundedRect(rect, m_buttonRadius, m_buttonRadius);
            if (m_showText)
            {
                painter->save();
                painter->setPen(QColor("#ffffff"));
                painter->setBrush(QColor("#ffffff"));

                int number = posToNumber(i, j);
                if (number == 11)
                {
                    number = 0;
                } else if(number == 10)
                {
                    // 数字10的地方是清空按钮
                    x = (j - 1) * m_buttonWidth + (j - 1) * m_buttonStep;;
                    y = -3 * (m_buttonHeight / 2) + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
                    QString text = tr("clear");
                    int textWidth = fontMetrics().width(text);
                    QFont font = painter->font();
                    font.setPixelSize(m_radius * 20/40);
                    painter->setFont(font);
                    painter->drawText(x - textWidth / 2 + 3, y + textWidth / 2 - 18, text);
                    painter->restore();
                    continue;
                }
                else if (number == 12)
                {
                    // 画删除符号(3,2)
                    x = m_buttonWidth / 2 + m_buttonStep;
                    y = m_buttonHeight + 3 * (m_buttonStep / 2);
                    painter->drawPixmap(x + m_buttonWidth/2 - 15,y + m_buttonHeight/2 - 15, QPixmap(":/image/assets/intel/delete.svg"));
                    painter->restore();
                    continue;
                }

                QString text = QString::number(number);
                x = (j - 1) * m_buttonWidth + (j - 1) * m_buttonStep;;
                y = -3 * (m_buttonHeight / 2) + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
                int textWidth = fontMetrics().width(text);
                int textHeight = fontMetrics().height();
                painter->drawText(x - textWidth / 2, y + textWidth / 2 + 4, text);
                painter->restore();
            }
        }
    }
    painter->restore();
}

//画外圆
void DigitalAuthDialog::drawOutCircle(QPainter *painter)
{
    painter->save();
    QPen pen(m_outCircleColor, 2);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int x = -3 * (m_buttonWidth / 2) + j * m_buttonWidth + (j - 1) * m_buttonStep;
            int y = -2 * m_buttonHeight + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
            QRect rect(x, y, m_buttonWidth, m_buttonHeight);
//            painter->drawEllipse(rect);
            painter->drawRoundedRect(rect, m_buttonRadius, m_buttonRadius);
//            painter->drawEllipse(x, y, m_radius * 2, m_radius * 2);
            if (m_showText)
            {
                int number = posToNumber(i, j);
                QString text = QString::number(number);
                x = -2 * m_radius + j * 2 * m_radius + (j - 1) * 16;
                y = -3 * m_radius + i * 2 * m_radius + 16 * i - 24;
                int textWidth = fontMetrics().width(text);
                int textHeight = fontMetrics().height();
                painter->drawText(x - textWidth / 2, y + textWidth / 2, text);
            }
        }
    }

    painter->restore();
}

void DigitalAuthDialog::drawTipLine(QPainter *painter)
{
    painter->save();

    int offset = 3;

    if (m_posInCircle.isIn)
    {
        if (!m_pressed)
        {
            painter->setPen(m_tipLineColor);
            painter->setBrush(Qt::NoBrush);
            int x = -3 * (m_buttonWidth / 2) + m_posInCircle.j * m_buttonWidth + (m_posInCircle.j - 1) * m_buttonStep - offset;
            int y = -2 * m_buttonHeight + m_posInCircle.i * m_buttonHeight + m_buttonStep * m_posInCircle.i - 3 * (m_buttonStep / 2) - offset;
            QRect rect(x, y, m_buttonWidth, m_buttonHeight);
            painter->drawRoundedRect(rect, m_buttonRadius, m_buttonRadius);
        }
    }

    painter->restore();
}

//绘制点击时的颜色
void DigitalAuthDialog::drawInCircle(QPainter *painter)
{
    painter->save();

    if (m_pressed)
    {
        if (m_posInCircle.isIn)
        {
            m_circleColor.setAlphaF(0.05);
            painter->setPen(m_circleColor);
            painter->setBrush(m_circleColor);
            int x = -3 * (m_buttonWidth / 2) + m_posInCircle.j * m_buttonWidth + (m_posInCircle.j - 1) * m_buttonStep;
            int y = -2 * m_buttonHeight + m_posInCircle.i * m_buttonHeight + m_buttonStep * m_posInCircle.i - 3 * (m_buttonStep / 2);
            int w = m_buttonWidth;
            int h = m_buttonHeight;
            QRect rect = QRect(x, y, w, h);
            painter->drawRoundedRect(rect, m_buttonRadius, m_buttonRadius);
        }
    }

    painter->restore();
}

//绘制悬浮时的颜色
void DigitalAuthDialog::drawHoverCircle(QPainter *painter)
{
    painter->save();
    if (!m_pressed)
    {
        if (m_posInCircle.isIn)
        {
            m_circleColor.setAlphaF(0.35);
            painter->setPen(m_circleColor);
            painter->setBrush(m_circleColor);
            int x = -3 * (m_buttonWidth / 2) + m_posInCircle.j * m_buttonWidth + (m_posInCircle.j - 1) * m_buttonStep;
            int y = -2 * m_buttonHeight + m_posInCircle.i * m_buttonHeight + m_buttonStep * m_posInCircle.i - 3 * (m_buttonStep / 2);
            int w = m_buttonWidth;
            int h = m_buttonHeight;
            QRect rect = QRect(x, y, w, h);
            painter->drawRoundedRect(rect, m_buttonRadius, m_buttonRadius);
        }

    }

    painter->restore();
}


void DigitalAuthDialog::drawJoinLine(QPainter *painter)
{
    painter->save();

    QPen pen(m_outCircleColor, 3);
    painter->setPen(pen);
    //绘制连接线段
    for (int i = 0; i < m_vecInputPoints.count() - 1; i++)
    {
        painter->drawLine(m_vecInputPoints[i], m_vecInputPoints[i + 1]);
    }

    int offset1 = 1;//圆环偏移量
    int offset2 = 3;//内圆偏移量
    //绘制连接圆
    for (int i = 0; i < m_vecInputPoints.count(); i++)
    {
        int x = m_vecInputPoints[i].x() - m_radius + offset1;
        int y = m_vecInputPoints[i].y() - m_radius + offset1;
        int w = 2 * (m_radius - offset1);
        int h = 2 * (m_radius - offset1);
        painter->setPen(m_circleRingColor);
        painter->setBrush(m_circleRingColor);
        painter->drawEllipse(x, y, w, h);

        x = m_vecInputPoints[i].x() - m_radius + offset2;
        y = m_vecInputPoints[i].y() - m_radius + offset2;
        w = 2 * (m_radius - offset2);
        h = 2 * (m_radius - offset2);
        painter->setPen(m_outCircleColor);
        painter->setBrush(m_outCircleColor);
        painter->drawEllipse(x, y, w, h);
    }
    //绘制跟随线
    if (m_pressed && m_vecInputPoints.count() < 9)
    {
        int width = this->width();
        int height = this->height();
        int side = qMin(width, height);
        int gestureScale = side / 200;
        int x = (m_followPoint.x() - width / 2) / gestureScale;
        int y = (m_followPoint.y() - height / 2) / gestureScale;
        if (m_vecInputPoints.count() > 0)
        {
            painter->setPen(pen);
            painter->drawLine(m_vecInputPoints.last(), QPoint(x, y));
        }
    }

    painter->restore();
}

void DigitalAuthDialog::mouseMoveEvent(QMouseEvent *event)
{
//    bool temp = false;
//    //nothing
//    int x = (event->x() - this->width()/2) / (qMin(width(), height())/200);
//    int y = (event->y() - this->height()/2) / (qMin(width(), height())/200);

//    for (int i = 0; i < 4; i++)
//    {
//        for (int j = 0; j < 3; j++)
//        {
//            int x1 = -3 * (m_buttonWidth / 2) + j * m_buttonWidth + (j - 1) * m_buttonStep;
//            int y1 = -2 * m_buttonHeight + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
//            if (isInButtonArea(x, y, x1, y1))
//            {
//                m_posInCircle.i = i;
//                m_posInCircle.j = j;
//                m_posInCircle.isIn = true;
//                temp = true;
//                repaint();
//            }
//        }
//    }

//    if (!temp)
//    {
//        m_posInCircle.isIn = false;
//        repaint();
//    }

}

void DigitalAuthDialog::mousePressEvent(QMouseEvent *event)
{
    //nothing
//    if(m_authMessage)
//        cleanMessage();
//    if(m_resetOrAuthFailure)
//        m_resetOrAuthFailure->clear();
//    m_pressed = true;
//    int x = (event->x() - this->width()/2) / (qMin(width(), height())/200);
//    int y = (event->y() - this->height()/2) / (qMin(width(), height())/200);

//    for (int i = 0; i < 4; i++)
//    {
//        for (int j = 0; j < 3; j++)
//        {
//            int x1 = -3 * (m_buttonWidth / 2) + j * m_buttonWidth + (j - 1) * m_buttonStep;
//            int y1 = -2 * m_buttonHeight + i * m_buttonHeight + m_buttonStep * i - 3 * (m_buttonStep / 2);
//            if (isInButtonArea(x, y, x1, y1))
//            {
//                if(m_isAuthing)
//                {
//                    qWarning() << "authenticating";
//                    return;
//                }
//                m_posInCircle.i = i;
//                m_posInCircle.j = j;
//                m_posInCircle.isIn = true;
//                if(posToNumber(i, j) == 10)
//                {
//                    //点击了清空按钮
//                    m_inputPassword.clear();
//                } else if (posToNumber(i, j) == 12)
//                {
//                    if (!m_inputPassword.isEmpty())
//                        m_inputPassword.remove(m_inputPassword.size() -1, 1);
//                } else {
//                    int num = posToNumber(i,j);
//                    m_inputPassword.append(QString::number(num == 11 ? 0 : num));
//                    if (m_inputPassword.size() >= 6)
//                    {
//                        checkPassword();
//                    }
////                    qDebug() << "---------------选中了"<< posToNumber(i,j);
//                }
//                repaint();
//            }
//        }
//    }
}

void DigitalAuthDialog::mouseReleaseEvent(QMouseEvent *event)
{
//    //nothing
//    m_pressed = false;
//    //m_posInCircle.isIn = false;
//    repaint();
}

////监听键盘输入
void DigitalAuthDialog::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "keyPressEvent " <<  event->key();
    if (m_isAuthing)
    {
        qWarning() << "authenticating";
        return;
    }
    if(event->key() == Qt::Key_Backspace)
    {
        if (!m_inputPassword.isEmpty())
            m_inputPassword.remove(m_inputPassword.size() -1, 1);
    } else if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 )
    {
        if(m_resetOrAuthFailure)
            m_resetOrAuthFailure->clear();
        m_inputPassword.append(event->text());
        if (m_inputPassword.size() >= 6)
        {
            checkPassword();
            m_inputPassword.clear();
        }
    } else {
        return;
    }
    repaint();
}

////监听键盘输入
//void DigitalAuthDialog::RecieveKey(int key)
//{
//    qDebug() << "keyPressEvent " <<  key;
//    if (m_isAuthing)
//    {
//        qWarning() << "authenticating";
//        return;
//    }
//    if(key == 10)
//    {
//        if (!m_inputPassword.isEmpty())
//            m_inputPassword.remove(m_inputPassword.size() -1, 1);
//    } else if (key >= 0 && key <= 9 )
//    {
//        if(m_resetOrAuthFailure)
//            m_resetOrAuthFailure->clear();
//        m_inputPassword.append(QString::number(key));
//        if (m_inputPassword.size() >= 6)
//        {
//            checkPassword();
//            m_inputPassword.clear();
//        }
//    } else {
//        return;
//    }
//    repaint();
//}

double DigitalAuthDialog::distance(int x1, int y1, int x2, int y2)
{
    //nothing
    double x = x1 - x2;
    double y = y1 - y2;
    return sqrt(x*x + y*y);
}

int DigitalAuthDialog::posToNumber(int i, int j)
{
    //nothing
    return ((3*i) + j +1);
}

void DigitalAuthDialog::setPassword(const QString password)
{
    //ToDo
    Q_EMIT setPinCode(password);
}

QString DigitalAuthDialog::getPassword()
{
    //nothing
}

bool DigitalAuthDialog::checkPassword()
{
    if(m_loginType == LoginType::AUTHENTICATE)
    {
        m_password = m_inputPassword;
        qDebug() << "Input finished" << m_password;
        m_auth->respond(m_password);
        m_isAuthing = true;
//        m_timer->start(5000);
//        onShowMessage(tr("now is authing, wait a moment"), Auth::MessageTypeInfo);
    } else if (m_loginType == LoginType::RESET)
    {
        m_resetPassword = m_inputPassword;
        m_title->setText(tr("ConfirmNewUEduPWD"));
        m_title->adjustSize();
        m_title->setGeometry((this->width() - m_title->width())/2, 5, m_title->width(), m_title->height());
        m_inputPassword.clear();
        m_loginType = LoginType::SECONDCONFIRMATION;
        Q_EMIT switchToReset(false);
    } else if (m_loginType == LoginType::SECONDCONFIRMATION)
    {
        if (m_resetPassword == m_inputPassword)
        {
            setPassword(m_resetPassword);
        } else {
            m_resetOrAuthFailure->setText(tr("The two password entries are inconsistent, please reset"));
            m_resetOrAuthFailure->setStyleSheet("font-size:14px;color:rgba(255, 255, 255, 255)");
            m_resetOrAuthFailure->adjustSize();
            m_resetOrAuthFailure->setGeometry((this->width()- m_resetOrAuthFailure->width())/2, 120,
                                              m_resetOrAuthFailure->width(),m_resetOrAuthFailure->height());
           showResetPasswordPage();
           Q_EMIT switchToReset(true);
        }
    }
    return false;
}

//画密码状态的圆圈
void DigitalAuthDialog::drawStatusCircle(QPainter *painter)
{
    painter->save();

    m_circleColor.setAlphaF(0.45);
    QPen pen(m_circleColor, 2);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    for(int i = 0; i < 6; i++)
    {
        int x = -11 * m_statusRadius + i * 4 * m_statusRadius;
        int y = -2 * m_buttonHeight - (11 * m_buttonStep) / 2;
        if (i < m_inputPassword.size()) {
            painter->save();
            painter->setBrush(QColor("#ffffff"));
            QRect rect(x, y, 2 * m_statusRadius, 2 * m_statusRadius);
            painter->drawEllipse(rect);
            painter->restore();
        } else {
            painter->drawEllipse(x, y, 2 * m_statusRadius, 2 * m_statusRadius);
        }
    }
    painter->restore();
}

void DigitalAuthDialog::onShowMessage(const QString &message, Auth::MessageType type)
{
    m_authMessage->setText(message);
    m_authMessage->adjustSize();
    m_authMessage->setGeometry((this->width()-m_authMessage->width())/2, this->height() -20, m_authMessage->width(),m_authMessage->height());
}

void DigitalAuthDialog::cleanMessage()
{
    m_authMessage->clear();
}

void DigitalAuthDialog::onShowPrompt(const QString &prompt, Auth::PromptType type)
{
    qDebug() << "prompt: " << prompt;
}

void DigitalAuthDialog::onAuthComplete()
{
    qDebug() << "Auth Complete";
//    m_timer->stop();
    DBusMsgCode errcode = EduPlatformInterface::getInstance()->CheckToken(getenv("USER"));
    if(m_auth->isAuthenticated())
    {
	/*
        DBusMsgCode errcode = EduPlatformInterface::getInstance()->CheckToken(getenv("USER"));
	
        if(DBusMsgCode::Error_AccessTokenInvalid == errcode){
	    m_resetOrAuthFailure->setText(tr("登录状态已过期，请重新扫码登录"));
            m_resetOrAuthFailure->setStyleSheet("font-size:14px;color:rgba(255, 255, 255, 255)");
            m_resetOrAuthFailure->adjustSize();
            m_resetOrAuthFailure->setGeometry((this->width()- m_resetOrAuthFailure->width())/2, 120,
                                              m_resetOrAuthFailure->width(),m_resetOrAuthFailure->height());
	    
	    startAuth(); 
	    m_inputPassword.clear();
            m_password.clear();
            m_isAuthing = false;
            repaint();
	    return;
	}
	*/
        Q_EMIT authenticateCompete(true);
        m_isAuthing = false;
    }
    else
    {
        qWarning() << "auth failed";
        if(m_loginType == LoginType::AUTHENTICATE)
        {
            m_resetOrAuthFailure->setText(tr("Password entered incorrectly, please try again"));
            m_resetOrAuthFailure->setStyleSheet("font-size:14px;color:rgba(255, 255, 255, 255)");
            m_resetOrAuthFailure->adjustSize();
            m_resetOrAuthFailure->setGeometry((this->width()- m_resetOrAuthFailure->width())/2, 120,
                                              m_resetOrAuthFailure->width(),m_resetOrAuthFailure->height());
        }
        //认证失败，重新认证
        startAuth();
    }
    m_inputPassword.clear();
    m_password.clear();
    m_isAuthing = false;
    repaint();
}

void DigitalAuthDialog::startAuth()
{
   m_inputPassword.clear();
   m_password.clear();
   m_auth->authenticate(getenv("USER"));
   m_isAuthing = false;
   repaint();
}

void DigitalAuthDialog::stopAuth()
{
    m_isAuthing = false;
}

void DigitalAuthDialog::startWaiting()
{

}

void DigitalAuthDialog::stopWaiting()
{

}

void DigitalAuthDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "DigitalAuthDialog::closeEvent";

    if(m_auth && m_auth->isAuthenticating())
    {
        m_auth->stopAuth();
    }
    return QWidget::closeEvent(event);
}

bool DigitalAuthDialog::isInButtonArea(int x1, int y1, int x2, int y2)
{
    if (x1 >= x2 && x1 <= (x2 + m_buttonWidth) && y1 >= y2 && y1 <= (y2 + m_buttonHeight))
        return true;
    return false;
}

void DigitalAuthDialog::reset()
{
    m_title->setText(tr("LoginByUEdu"));
    m_title->adjustSize();
    m_title->setGeometry((this->width() - m_title->width())/2, 5, m_title->width(), m_title->height());

    m_labelReset->show();
    m_inputPassword.clear();
    m_resetPassword.clear();
    m_loginType = LoginType::AUTHENTICATE;
    repaint();
}

/**
 * @brief DigitalAuthDialog::showResetPasswordPage
 * 重新展示重置密码页
 */
void DigitalAuthDialog::showResetPasswordPage()
{
    m_title->setText(tr("SetNewUEduPWD"));
    m_title->adjustSize();
    m_title->setGeometry((this->width() - m_title->width())/2, 5, m_title->width(), m_title->height());
    m_loginType = LoginType::RESET;
    m_resetPassword.clear();
    m_inputPassword.clear();
    repaint();
    Q_EMIT switchToReset(true);
}

int DigitalAuthDialog::getStatus()
{
    return m_loginType;
}

void DigitalAuthDialog::showErrorMessage(QString message)
{
    m_resetOrAuthFailure->setText(message);
    m_resetOrAuthFailure->setStyleSheet("font-size:14px;color:rgba(255, 255, 255, 255)");
    m_resetOrAuthFailure->adjustSize();
    m_resetOrAuthFailure->setGeometry((this->width()- m_resetOrAuthFailure->width())/2, 120,
                                      m_resetOrAuthFailure->width(),m_resetOrAuthFailure->height());

    QTimer::singleShot(3000,[=](){
        m_resetOrAuthFailure->clear();
    });
}

MyLabel::MyLabel(const QString & text,QWidget *parent) : QLabel(parent)
{
    setText(text);
}
 
MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
 
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    //Qt::LeftButton
    //Qt::RightButton
    if(event->button()== Qt::LeftButton)
    {
        this->setStyleSheet("QLabel{font-size:16px;color:rgba(255, 255, 255, 0.45);}");
    }
}

void MyLabel::mouseReleaseEvent(QMouseEvent *event)
{
    //Qt::LeftButton
    //Qt::RightButton
    if(event->button()== Qt::LeftButton)
    {
        this->setStyleSheet("QLabel{font-size:16px;color:rgba(255, 255, 255, 0.35);}"
                            "QLabel:hover{color:rgba(255, 255, 255, 1);}");
        Q_EMIT onClick();
    }
}
