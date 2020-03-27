#include "autoresize.h"

AutoResize::AutoResize(QWidget* obj,int baseWidth,int baseHeight):
	obj(obj),
	baseWidth(baseWidth),
	baseHeight(baseHeight)
{

}

AutoResize::~AutoResize(void)
{
}

