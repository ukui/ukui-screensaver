#ifndef DIGITALKEYBOARD_H
#define DIGITALKEYBOARD_H

#include <QWidget>
#include <QPushButton>
#include <QKeyEvent>

class DigitalKeyBoard : public QWidget
{
    Q_OBJECT

public:
    explicit DigitalKeyBoard(QWidget *parent = nullptr);

public Q_SLOTS:
    void onKeyReleaseEvent(QKeyEvent *event);

Q_SIGNALS:
    void numbersButtonPress(int btn_id);

private:
    void initUI();
    void initConnect();
    void setQSS();

private:
    QPushButton *m_pNumerPressBT[12]; // 0~9 是数字按键 10是删除键 11是清空键
};

#endif // DIGITALKEYBOARD_H

