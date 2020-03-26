#include "autoresize.h"

AutoResize::AutoResize(QWidget* obj,float baseWidth,float baseHeight):
	obj(obj),
	baseWidth(baseWidth),
	baseHeight(baseHeight)
{

}

AutoResize::~AutoResize(void)
{
}

