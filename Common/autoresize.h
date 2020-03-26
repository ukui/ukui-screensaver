#ifndef AUTORESIZE_H
#define AUTORESIZE_H

#include <QWidget>

struct AutoResizeOriginalData
{
        QRect data_rect;
        QFont data_font;
};
class AutoResize
{
public:
	AutoResize(QWidget* obj,int baseWidth,int baseHeight);
        ~AutoResize(void);
private:
	QWidget *obj;
	int baseWidth;
	int baseHeight;

};

#endif
