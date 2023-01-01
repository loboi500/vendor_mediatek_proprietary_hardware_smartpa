#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <tinyalsa/asoundlib.h>
#include "cs_cs35l45_intf.h"

#define SPEAKON_NUM (sizeof(SpeakerOn)/sizeof(kctrl_val))
#define SPEAKOFF_NUM (sizeof(SpeakerOff)/sizeof(kctrl_val))

#define KCTRL_STR_VAL(kctrl, str, val) \
{       .name = kctrl, \
        .val_str = str, \
        .value = val }

struct kctrl_val {
    const char* name;
    const char* val_str;
    int value;
};

static struct mixer *g_mixer = NULL;

static const struct kctrl_val SpeakerOn[]= {
    KCTRL_STR_VAL("SPK1L DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK2L DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK3L DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK4L DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK1H DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK2H DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK3H DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK4H DSP1 Enable Switch", NULL, 1),
    KCTRL_STR_VAL("SPK1L DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK2L DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK3L DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK4L DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK1H DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK2H DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK3H DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK4H DACPCM Source", "ASP_RX1", 0),
    KCTRL_STR_VAL("SPK1L AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK2L AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK3L AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK4L AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK1H AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK2H AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK3H AMP Mute", NULL, 0),
    KCTRL_STR_VAL("SPK4H AMP Mute", NULL, 0),
};

static const struct kctrl_val SpeakerOff[]= {
    KCTRL_STR_VAL("SPK1L DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK2L DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK3L DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK4L DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK1H DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK2H DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK3H DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK4H DSP1 Enable Switch", NULL, 0),
    KCTRL_STR_VAL("SPK1L DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK2L DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK3L DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK4L DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK1H DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK2H DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK3H DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK4H DACPCM Source", "DSP_TX1", 0),
    KCTRL_STR_VAL("SPK1L AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK2L AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK3L AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK4L AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK1H AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK2H AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK3H AMP Mute", NULL, 1),
    KCTRL_STR_VAL("SPK4H AMP Mute", NULL, 1),
};

static int cs_cs35l45_init(__unused struct SmartPa *smart_pa)
{
    ALOGD("%s: begin, mixer: %p\n", __func__, smart_pa->attribute.mMixer);
    g_mixer = smart_pa->attribute.mMixer;

    return 0;
}

static int cs_cs35l45_speakerOn(struct SmartPaRuntime *runtime)
{

    struct mixer_ctl *mixer_ctl;
    struct mixer *mixer = g_mixer;
    int ret = 0, i = 0;

    ALOGD("%s: begin, mixer: %p\n", __func__, mixer);

    for ( i = 0; i < SPEAKON_NUM; i++) {
        mixer_ctl = mixer_get_ctl_by_name(mixer, SpeakerOn[i].name);
        if (!mixer_ctl) {
            ALOGE("%s: %s not invalid ctl name\n", __func__, SpeakerOn[i].name);
        }
        if (SpeakerOn[i].val_str == NULL) {
            ret = mixer_ctl_set_value(mixer_ctl, 0, SpeakerOn[i].value);
        } else {
            ret = mixer_ctl_set_enum_by_string(mixer_ctl, SpeakerOn[i].val_str);
        }
        if (ret < 0)
            ALOGE("%s: %s set value fail\n", __func__, SpeakerOn[i].name);
    }

    return 1;
}

static int cs_cs35l45_speakerOff()
{

    struct mixer_ctl *mixer_ctl;
    struct mixer *mixer = g_mixer;
    int ret = 0, i = 0;

    ALOGD("%s: begin, mixer: %p\n", __func__, mixer);

    for ( i = 0; i < SPEAKOFF_NUM; i++) {
        mixer_ctl = mixer_get_ctl_by_name(mixer, SpeakerOff[i].name);
        if (!mixer_ctl) {
            ALOGE("%s: %s not invalid ctl name\n", __func__, SpeakerOff[i].name);
        }
        if (SpeakerOff[i].val_str == NULL) {
            ret = mixer_ctl_set_value(mixer_ctl, 0, SpeakerOff[i].value);
        } else {
            ret = mixer_ctl_set_enum_by_string(mixer_ctl, SpeakerOff[i].val_str);
        }
        if (ret < 0)
            ALOGE("%s: %s set value fail\n", __func__, SpeakerOff[i].name);
    }

    return 1;
}

int mtk_smartpa_init(struct SmartPa *smart_pa)
{
    smart_pa->ops.init = cs_cs35l45_init;
    smart_pa->ops.speakerOn = cs_cs35l45_speakerOn;
    smart_pa->ops.speakerOff = cs_cs35l45_speakerOff;
    return 0;
}
