#include "gsettings.h"

QGSettings::QGSettings(QString schema)
{
	gsettings = g_settings_new(schema.toLocal8Bit().data());
}

/*
 * Setter
 */

QString QGSettings::getString(QString key)
{
	char *key_str, *value;
	key_str = key.toLocal8Bit().data();
	value = g_settings_get_string(gsettings, key_str);
	return QString::fromLocal8Bit(value);
}

int QGSettings::getInt(QString key)
{
	char *key_str;
	int value;
	key_str = key.toLocal8Bit().data();
	value = g_settings_get_int(gsettings, key_str);
	return value;
}

bool QGSettings::getBool(QString key)
{
	char *key_str;
	bool value;
	key_str = key.toLocal8Bit().data();
	value = g_settings_get_boolean(gsettings, key_str);
	return value;
}

QList<QString> QGSettings::getStringList(QString key)
{
	char *key_str;
	char **value;
	key_str = key.toLocal8Bit().data();
	value = g_settings_get_strv(gsettings, key_str);
	QList<QString> list;
	for (; *value; value++)
		list.append(QString::fromLocal8Bit(*value));
	return list;
}


/*
 * Setter
 */

bool QGSettings::setString(QString key, QString value)
{
	char *key_str, *value_str;
	key_str = key.toLocal8Bit().data();
	value_str = value.toLocal8Bit().data();
	return g_settings_set_string(gsettings, key_str, value_str);
}

bool QGSettings::setInt(QString key, int value)
{
	char *key_str;
	key_str = key.toLocal8Bit().data();
	return g_settings_set_int(gsettings, key_str, value);
}

bool QGSettings::setBool(QString key, bool value)
{
	char *key_str;
	key_str = key.toLocal8Bit().data();
	return g_settings_set_boolean(gsettings, key_str, value);
}

bool QGSettings::setStringList(QString key, QList<QString> value)
{
	char *key_str;
	const char ** string_array;
	int i;
	key_str = key.toLocal8Bit().data();
	string_array = (const char **)malloc(value.count() * sizeof(char *));
	for (i = 0; i < value.count(); i++) {
		string_array[i] = value[i].toLocal8Bit().data();
	}
	string_array[i] = 0;
	return g_settings_set_strv(gsettings, key_str, string_array);
}
