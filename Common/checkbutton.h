#ifndef CHECKBUTTON_H
#define CHECKBUTTON_H
#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QPainter>
#include <QEvent>

class checkButton : public QFrame
{
    Q_OBJECT
public:
    checkButton(QWidget *parent = 0);
    void setChecked(bool checked);

    bool isChecked();

protected:
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

private:
    bool checked;

    QColor borderColorOff;
    QColor bgColorOff;
    QColor bgColorOn;
    QColor sliderColorOff;
    QColor sliderColorOn;
    int space; //滑块离背景间隔
    int rectRadius; //圆角角度
    int step; //移动步长
    int startX;
    int endX;
    QTimer * timer;

    void drawBg(QPainter *painter);
    void drawSlider(QPainter *painter);


private slots:
    void updatevalue();


Q_SIGNALS:
    void checkedChanged(bool checked);


};

#endif // CHECKBUTTON_H
