#ifndef SOUNDDEVICESET_H
#define SOUNDDEVICESET_H

#include <QObject>
#include <QEvent>
#include <libmatemixer/matemixer.h>
#include <glib.h>
#include <QMutex>
#include <QDebug>

class SoundDeviceSet : public QObject
{
    Q_OBJECT
public:
    static SoundDeviceSet* instance(QObject *parent = nullptr);
    void setMute(bool val);
    bool getIsMute();
    void setDefaultVal();
    int getValue(){ return m_value; }

Q_SIGNALS:
    void muteChanged(bool mute);
    // val from 0 to 100 change
    void valueChanged(int val);

public Q_SLOTS:
    // val from 0 to 100 change
    void setValue(int val);

private:
    SoundDeviceSet(QObject *parent = nullptr);

private:
    MateMixerContext *context;

    static void onStreamControlMuteNotify(MateMixerStreamControl *m_pControl,GParamSpec *pspec,gpointer point);
    static void onStreamControlVolumeNotify(MateMixerStreamControl *m_pControl,GParamSpec *pspec,gpointer point);
    static void on_context_state_notify (MateMixerContext *context,GParamSpec *pspec,gpointer point );
    static void connected (MateMixerContext *context);
    void init();
    void emitSignal(bool val);
    bool getIsReady();

    static int m_value; // 当前的音量值

};

#endif // SOUNDDEVICESET_H
