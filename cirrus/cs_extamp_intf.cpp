#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <tinyalsa/asoundlib.h>
#include "cs_extamp_intf.h"


static int cs_extamp_speakerOn(struct SmartPaRuntime *runtime)
{

    struct mixer_ctl *mixer_ctl;
    struct mixer *mixer = mixer_open(0);
    int ret = 0;

    ALOGD("%s: begin, mixer: %p\n", __func__, mixer);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 17);
    if (ret < 0)
        ALOGE("%s: SPK1 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 17);
    if (ret < 0)
        ALOGE("%s: SPK2 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 17);
    if (ret < 0)
        ALOGE("%s: SPK3 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 17);
    if (ret < 0)
        ALOGE("%s: SPK4 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK1 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK2 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK3 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK4 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK1 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK2 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK3 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: SPK4 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "I2S1_Out_Mux");
    if (!mixer_ctl) {
        ALOGE("%s: I2S1_Out_Mux not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: I2S1_Out_Mux set value fail\n", __func__);

    mixer_close(mixer);
    return 1;
}

static int cs_extamp_speakerOff()
{

    struct mixer_ctl *mixer_ctl;
    struct mixer *mixer = mixer_open(0);
    int ret = 0;

    ALOGD("%s: begin, mixer: %p\n", __func__, mixer);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 9);
    if (ret < 0)
        ALOGE("%s: SPK1 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 9);
    if (ret < 0)
        ALOGE("%s: SPK2 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 9);
    if (ret < 0)
        ALOGE("%s: SPK3 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 AMP PCM Gain");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 AMP PCM Gain not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 9);
    if (ret < 0)
        ALOGE("%s: SPK4 AMP PCM Gain set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK1 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK2 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK3 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 AMP Enable Switch");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 AMP Enable Switch not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK4 AMP Enable Switch set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK1 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK1 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK1 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK2 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK2 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK2 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK3 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK3 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK3 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "SPK4 PCM Source");
    if (!mixer_ctl) {
        ALOGE("%s: SPK4 PCM Source not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: SPK4 PCM Source set value fail\n", __func__);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "I2S1_Out_Mux");
    if (!mixer_ctl) {
        ALOGE("%s: I2S1_Out_Mux not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 0);
    if (ret < 0)
        ALOGE("%s: I2S1_Out_Mux set value fail\n", __func__);

    mixer_close(mixer);
    return 1;
}

int mtk_smartpa_init(struct SmartPa *smart_pa)
{
    smart_pa->ops.speakerOn = cs_extamp_speakerOn;
    smart_pa->ops.speakerOff = cs_extamp_speakerOff;
    return 0;
}
