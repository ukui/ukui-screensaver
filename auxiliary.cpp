#include "auxiliary.h"

char *get_char_pointer(QString string)
{
	char *buffer;
	buffer = (char *)malloc(string.length() + 1);
	strcpy(buffer, string.toLocal8Bit().data());
	return buffer;
}
