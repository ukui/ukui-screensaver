#include "sounddeviceset.h"

int SoundDeviceSet::m_value = -1;

SoundDeviceSet* SoundDeviceSet::instance(QObject *parent)
{
    static SoundDeviceSet* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(!_instance)
        _instance = new SoundDeviceSet(parent);
    mutex.unlock();
    return _instance;
}

SoundDeviceSet::SoundDeviceSet(QObject *parent):
    QObject(parent)
{
    init();
}


void SoundDeviceSet::init(){
    if (mate_mixer_init () == FALSE){
                qDebug()<<"matemixer init false";
                return ;
        }

        context = mate_mixer_context_new ();
        if (mate_mixer_context_open (context) == FALSE) {
                qDebug()<<"matemixer context open context false.";
                return ;
        }

        MateMixerState state = mate_mixer_context_get_state (context);

        switch (state) {
                case MATE_MIXER_STATE_READY:
                        connected (context);
                        break;
                case MATE_MIXER_STATE_CONNECTING:
                        g_print ("Waiting for connection...\n");

                        /* The state will change asynchronously to either MATE_MIXER_STATE_READY
                         * or MATE_MIXER_STATE_FAILED, wait for the change in a main loop */
                        g_signal_connect (G_OBJECT (context),
                                        "notify::state",
                                        G_CALLBACK (on_context_state_notify),
                                        NULL);
                        break;
                default:
                        qDebug()<<"it's can not reached";
                        break;
        }
}

void SoundDeviceSet::emitSignal(bool val){
    emit muteChanged(val);
}

void SoundDeviceSet::setValue(int val){
    if(!context)
        return ;
    if(val > 100) val = 100;
    else if(val < 0) val = 0;
    if(val == m_value) return;
    m_value = val;
    emit valueChanged(m_value);

    MateMixerStream *m_pOutputStream = mate_mixer_context_get_default_output_stream(context);
    if(m_pOutputStream == NULL)
        return;

    MateMixerStreamControl *control = mate_mixer_stream_get_default_control(m_pOutputStream);

    bool status = false;
    int volume = val*65536/100;

    mate_mixer_stream_control_set_volume(control,guint(volume));
    if (val <= 0) {
        status = true;
        mate_mixer_stream_control_set_mute(control,status);
        mate_mixer_stream_control_set_volume(control,0);
    }
    else {
            mate_mixer_stream_control_set_mute(control,status);
    }
}

void SoundDeviceSet::onStreamControlMuteNotify(MateMixerStreamControl *m_pControl,GParamSpec *pspec,gpointer point){

      if(!m_pControl)
          return ;

      bool state = mate_mixer_stream_control_get_mute (m_pControl);
      SoundDeviceSet::instance()->emitSignal(state);
}

void SoundDeviceSet::onStreamControlVolumeNotify(MateMixerStreamControl *m_pControl,GParamSpec *pspec,gpointer point){

    MateMixerStreamControlFlags flags;
    guint volume = 0;

    if (m_pControl != nullptr)
        flags = mate_mixer_stream_control_get_flags(m_pControl);

    if (flags&MATE_MIXER_STREAM_CONTROL_VOLUME_READABLE) {
        volume = mate_mixer_stream_control_get_volume(m_pControl);
    }

    MateMixerDirection direction;
    MateMixerStream *m_pStream = mate_mixer_stream_control_get_stream(m_pControl);

    direction = mate_mixer_stream_get_direction(m_pStream);
    //设置输出滑动条的值
    int value = volume*100/65536.0 + 0.5;
    if (direction == MATE_MIXER_DIRECTION_OUTPUT) {
       SoundDeviceSet::instance()->setValue(value);
    }
}

void SoundDeviceSet::connected (MateMixerContext *context)
{

    MateMixerStream *m_pOutputStream = mate_mixer_context_get_default_output_stream(context);
    if(m_pOutputStream == NULL)
        return;

    MateMixerStreamControl *control = mate_mixer_stream_get_default_control(m_pOutputStream);

    g_signal_connect ( G_OBJECT (control),
                      "notify::volume",
                      G_CALLBACK (onStreamControlVolumeNotify),
                      NULL);

    g_signal_connect (control,
            "notify::mute",
            G_CALLBACK (onStreamControlMuteNotify),
            NULL);

    SoundDeviceSet::instance()->setDefaultVal();
}

void SoundDeviceSet::setDefaultVal()
{
    if(!context)
        return ;

    MateMixerStream *m_pOutputStream = mate_mixer_context_get_default_output_stream(context);
    if(m_pOutputStream == NULL)
        return;

    MateMixerStreamControl *control = mate_mixer_stream_get_default_control(m_pOutputStream);

//    int volume = mate_mixer_stream_control_get_volume(control);
//    int value = volume *100 /65536.0+0.5;

//    setValue(value);
}

void SoundDeviceSet::on_context_state_notify (MateMixerContext *context,GParamSpec *pspec,gpointer point )
{
    MateMixerState state;

    state = mate_mixer_context_get_state (context);

    switch (state) {
        case MATE_MIXER_STATE_READY:
            /* This state can be reached repeatedly if the context is connected
             * to a sound server, the connection is dropped and then reestablished */
            connected (context);
            break;
        case MATE_MIXER_STATE_FAILED:
            qDebug()<<"matemixser state failed";
            break;
        default:
            break;
    }
}

bool SoundDeviceSet::getIsMute(){
    if(!context)
        return false;

    MateMixerState state;
    state = mate_mixer_context_get_state (context);
    if(state != MATE_MIXER_STATE_READY)
        return false;

    MateMixerStream *m_pOutputStream = mate_mixer_context_get_default_output_stream(context);
    if(m_pOutputStream == NULL)
        return false;

    MateMixerStreamControl *control = mate_mixer_stream_get_default_control(m_pOutputStream);

    return  mate_mixer_stream_control_get_mute(control);
}

void SoundDeviceSet::setMute(bool val){
    if(!context)
        return;

    MateMixerState state;
    state = mate_mixer_context_get_state (context);
    if(state != MATE_MIXER_STATE_READY)
        return;

    MateMixerStream *m_pOutputStream = mate_mixer_context_get_default_output_stream(context);
    if(m_pOutputStream == NULL)
        return;

    MateMixerStreamControl *control = mate_mixer_stream_get_default_control(m_pOutputStream);

    mate_mixer_stream_control_set_mute(control,val);
}

bool SoundDeviceSet::getIsReady(){
    MateMixerState state;
    state = mate_mixer_context_get_state (context);
    return state == MATE_MIXER_STATE_READY;
}
