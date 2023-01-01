#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef ANDROID_BUILD
#include <cutils/log.h>
#endif
#include <tinyalsa/asoundlib.h>
#include <tinyxml2.h>
#include <list>
#include <dlfcn.h>
#include "rt_extamp_intf.h"

#include "libnvram.h"
#include "libfile_op.h"

using namespace std;
using namespace tinyxml2;
#define DEVICE_CONFIG_FILE "/system/vendor/etc/smartpa_param/rt_device.xml"

#ifdef MTK_SCENARIO
enum {
	RT_CASE_NORMAL,
	RT_CASE_RINGTONE,
	RT_CASE_PHONECALL,
	RT_CASE_VIOP,
	RT_CASE_MAX,
};

enum {
	RT_DEV_SPK,
	RT_DEV_RECV,
	RT_DEV_MAX,
};

static const char * const scenario[RT_DEV_MAX][RT_CASE_MAX] = {
	{ "speaker_on_normal", "speaker_on_ringtone", "speaker_on_phonecall", "speaker_on_voip"},
	{ "receiver_on_phonecall", "receiver_on_phonecall", "receiver_on_phonecall", "receiver_on_voip"},
};

static int bootMode;
static int isHwSmartPAUsed;

#define FACTORY_BOOT 4
#define ATE_FACTORY_BOOT 6
#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"

int read_bootmode(char const *path) {

	int fd;

	if (path == NULL)
		return -1;

	fd = open(path, O_RDONLY);
	if (fd >= 0) {
		char buffer[20];
		int amt = read(fd, buffer, sizeof(int));
		close(fd);
		return amt == -1 ? -errno : atoi(buffer);
	}
	ALOGE("write_int failed to open %s\n", path);
	return -errno;
}
#endif

#ifdef ANDROID_BUILD
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rt_extamp"

#define RT_LOGD(format, args...) \
do {\
	printf(format, ##args);\
	ALOGD(format, ##args);\
} while (0);

#define RT_LOGE(format, args...) \
do {\
	printf(format, ##args);\
	ALOGE(format, ##args);\
} while (0);
#else
#define RT_LOGD(format, args...) \
do {\
	printf(format, ##args);\
} while (0);

#define RT_LOGE(format, args...) \
do {\
	printf(format, ##args);\
} while (0);
#endif

struct ctl_item {
	const char *name;
	const char *strval;
};

class conf_item {
public:
	conf_item() {}
	const char *path;
	const char *file;
	const char *check;
	list<struct ctl_item> ctl_list;
	~conf_item() {
		ctl_list.clear();
	}
};



class scenario_item {
public:
	scenario_item() {}
	const char *name;
	list<struct ctl_item> ctl_list;
	bool conf_item_exist;
	conf_item scene_conf;
	~scenario_item() {
		ctl_list.clear();
	}
};

class amp_control_inst {
private:
	int apply_amp_ctl(list<struct ctl_item> &);
	int apply_amp_param(const conf_item *);
	int amp_ctl_check(const conf_item *);
	int show_ctl_list(list<struct ctl_item> &);
public:
	struct mixer *mixer;
	list<conf_item> conf_list;
	list<scenario_item> scenario_list;

	amp_control_inst(struct mixer *audio_mixer) {
		mixer = audio_mixer;
	}
	~amp_control_inst() {
		conf_list.clear();
		scenario_list.clear();
	}
	int apply_amp_conf();
	int apply_amp_scenario(const char *name);
	int show_all_item();
};

static amp_control_inst *amp_ctl_inst = NULL;

int amp_control_inst::apply_amp_ctl(list<struct ctl_item> &ctl_list)
{
	list<struct ctl_item>::iterator it;
	struct mixer_ctl *mixer_ctl;
	int i, ret = 0;
	const char *tmp;

	if (ctl_list.empty())
		return 0;
	for (it = ctl_list.begin(); it != ctl_list.end(); ++it) {
		mixer_ctl = mixer_get_ctl_by_name(mixer, (*it).name);
		if (!mixer_ctl) {
			RT_LOGE("%s: %s not invalid ctl name\n", __func__, (*it).name);
			continue;
		}
		switch (mixer_ctl_get_type(mixer_ctl)) {
		case MIXER_CTL_TYPE_BOOL:
		case MIXER_CTL_TYPE_INT:
		case MIXER_CTL_TYPE_BYTE:
			for (i = 0; i < (int)mixer_ctl_get_num_values(mixer_ctl); i++) {
				ret = mixer_ctl_set_value(mixer_ctl, i, atoi((*it).strval));
				if (ret < 0)
					RT_LOGE("%s: %s set value fail\n", __func__, (*it).name);
			}
			break;
		case MIXER_CTL_TYPE_ENUM:
			tmp = mixer_ctl_get_enum_string(mixer_ctl, atoi((*it).strval));
			if (!tmp) {
				RT_LOGE("%s %s: not valid value\n", __func__, (*it).name);
				continue;
			}
			printf("%s\n", tmp);
			ret = mixer_ctl_set_enum_by_string(mixer_ctl, tmp);
			if (ret < 0)
				RT_LOGE("%s: %s set enum fail\n", __func__, (*it).name);
			break;
		default:
			RT_LOGE("%s: %s not supported type\n", __func__, (*it).name);
			break;
		}
	}
	return 0;
}

int amp_control_inst::apply_amp_param(const conf_item *p_item)
{
	char data[4096] = {0};
	int size;
	int fd;

	RT_LOGD("%s: begin\n", __func__);
	if (!p_item)
		return -EINVAL;
	RT_LOGD("%s: item_path %s\n", __func__, p_item->path);
	fd = open(p_item->file, O_RDONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot open file\n", __func__, p_item->file);
		goto out_amp_param;
	}
	size = read(fd, data, 4096);
	close(fd);
	if (size < 0)
		goto out_amp_param;
	fd = open(p_item->path, O_WRONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot open path\n", __func__, p_item->path);
		goto out_amp_param;
	}
	if (write(fd, data, size) < 0)
		RT_LOGE("%s: %s cannot write path\n", __func__, p_item->path);
	close(fd);
	return 0;
out_amp_param:
	return -EBADF;
}

int amp_control_inst::amp_ctl_check(const conf_item *p_item)
{
	char data[4096] = {0};
	int size;
	int val, fd;

	RT_LOGD("%s: begin\n", __func__);
	if (!p_item)
		return -EINVAL;
	fd = open(p_item->check, O_RDONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot check file\n", __func__, p_item->check);
		goto out_amp_check;
	}
	size = read(fd, data, 4096);
	close(fd);
	if (size < 0)
		goto out_amp_check;

	if (sscanf(data, "%d\n", &val) != 1)
		goto out_amp_check;

	return val;
out_amp_check:
	return -EACCES;
}

int amp_control_inst::apply_amp_conf()
{
	list<conf_item>::iterator it;
	int ret = 0;

	if (conf_list.empty())
		return 0;
	for (it = conf_list.begin(); it != conf_list.end(); ++it) {
		ret = apply_amp_param(&(*it));
		if (ret < 0)
			continue;
		ret = amp_ctl_check(&(*it));
		if (ret == 0) {
			ret = apply_amp_ctl((*it).ctl_list);
			if (ret < 0)
				RT_LOGE("%s: %s apply amp ctl fail\n", __func__, (*it).path);
		} else if (ret < 0)
			RT_LOGE("%s: %s check amp control fail\n", __func__, (*it).path);
	}
	return 0;
}

int amp_control_inst::apply_amp_scenario(const char *name)
{
	list<scenario_item>::iterator it;
	int ret = 0;

	if (!name) {
		RT_LOGE("%s name is null\n", __func__);
		return -EINVAL;
	} else
		RT_LOGD("%s name = %s\n", __func__, name);
	if (scenario_list.empty())
		return 0;

	if (!strcmp(name, "NULL"))
		return 0;

	for (it = scenario_list.begin(); it != scenario_list.end(); ++it) {
		if (!strcmp(name, (*it).name)) {
			if ((*it).conf_item_exist) {
				ret = apply_amp_param(&(*it).scene_conf);
				if (ret < 0)
					RT_LOGE("%s: %s apply amp param fail\n", __func__, (*it).name);
			} else
				RT_LOGD("%s: no amp scene conf\n", __func__);
			ret = apply_amp_ctl((*it).ctl_list);
			if (ret < 0)
				RT_LOGE("%s: %s apply amp control fail\n", __func__, (*it).name);
			break;
		}
	}
	return ret;
}

int amp_control_inst::show_ctl_list(list<struct ctl_item> &ctl_list)
{
	for (list<struct ctl_item>::iterator it = ctl_list.begin(); it != ctl_list.end(); ++it)
		RT_LOGD("%s -> %s\n", (*it).name, (*it).strval);
	return 0;
}

int amp_control_inst::show_all_item()
{
	/* conf_item show */
	RT_LOGD("conf_list start\n");
	for (list<conf_item>::iterator it = conf_list.begin(); it != conf_list.end(); ++it) {
		RT_LOGD("check %s\n", (*it).check);
		RT_LOGD("path %s\n", (*it).path);
		RT_LOGD("file %s\n", (*it).file);
		show_ctl_list((*it).ctl_list);
	}
	RT_LOGD("conf_list end\n");
	/* scenario_item show */
	RT_LOGD("scenario_list start\n");
	for (list<scenario_item>::iterator it = scenario_list.begin(); it != scenario_list.end(); ++it) {
		RT_LOGD("name %s\n", (*it).name);
		show_ctl_list((*it).ctl_list);
	}
	RT_LOGD("scenario_list end\n");
	return 0;
}

static int get_xml_ctl_item(XMLElement *np, struct ctl_item *p_item)
{
	p_item->name = np->Attribute("name");
	p_item->strval = np->Attribute("val");
	if (!p_item->name || !p_item->strval)
		return -EINVAL;
	return 0;
}

static int parse_xml_ctl_element(XMLElement *root,
	list<struct ctl_item> &ctl_list)
{
	XMLElement *pChild;
	int ret = 0;

	if (!root)
		return -EINVAL;

	pChild = root->FirstChildElement("ctl");

	while (pChild != NULL) {
		struct ctl_item ctl_item;
		ret = get_xml_ctl_item(pChild, &ctl_item);
		if (ret == 0) {
			ctl_list.push_back(ctl_item);
		}
		pChild = pChild->NextSiblingElement("ctl");
	}

	return 0;
}

static int get_xml_conf_item(XMLElement *np, conf_item *p_item)
{
	p_item->check = np->Attribute("check_path");
	p_item->path = np->Attribute("param_path");
	p_item->file = np->Attribute("param_file");
	if (!p_item->check || !p_item->path || !p_item->file) {
		RT_LOGE("%s empty attribute\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static int parse_xml_conf_element(XMLElement *root,
	list<conf_item> &conf_list)
{
	XMLElement *pChild;
	int ret = 0;

	if (!root)
		return -EINVAL;

	pChild = root->FirstChildElement("conf");
	while (pChild != NULL) {
		conf_item p_item;
		ret = get_xml_conf_item(pChild, &p_item);
		if (ret == 0) {
			if (parse_xml_ctl_element(pChild, p_item.ctl_list) < 0)
				RT_LOGE("%s: parse control fail\n", __func__);
			conf_list.push_back(p_item);
		}
		pChild = pChild->NextSiblingElement("conf");
	}
	return 0;
}

static int get_xml_scenario_item(XMLElement *np, scenario_item *p_item)
{
	p_item->name = np->Attribute("name");
	if (!p_item->name)
		return -EINVAL;
	return 0;
}

static int parse_xml_scenario_element(XMLElement *root,
	list<scenario_item> &scenario_list)
{
	XMLElement *pChild;
	int ret = 0;

	if (!root)
		return -EINVAL;

	pChild = root->FirstChildElement("scenario");
	while (pChild != NULL) {
		scenario_item p_item;
		ret = get_xml_scenario_item(pChild, &p_item);
		if (ret == 0) {
			XMLElement *conf_Child = pChild->FirstChildElement("conf");
			if (conf_Child) {
				ret = get_xml_conf_item(conf_Child, &p_item.scene_conf);
				if (ret == 0)
					p_item.conf_item_exist = true;
				else
					p_item.conf_item_exist = false;
			} else
				p_item.conf_item_exist = false;

			if (parse_xml_ctl_element(pChild, p_item.ctl_list) < 0)
				RT_LOGD("%s: parse control fail\n", __func__);
			scenario_list.push_back(p_item);
		}
		pChild = pChild->NextSiblingElement("scenario");
	}

	return 0;
}

static int parse_xml_document(const char *file, amp_control_inst *ctl_inst)
{
	XMLDocument *doc;
	int ret = 0;

	if (!file || !ctl_inst) {
		RT_LOGE("%s file or ctl_inst is NULL\n", __func__);
		return -EINVAL;
	}
	doc = new XMLDocument();

	XMLError error = doc->LoadFile(file);
	if (error != XMLError::XML_SUCCESS) {
		RT_LOGE("%s load file %s failed\n", __func__, file);
		return -EINVAL;
	}

	XMLElement *root = doc->RootElement();

	if (root) {
		ret = parse_xml_conf_element(root, ctl_inst->conf_list);
		if (ret < 0) {
			RT_LOGE("%s, parse xml conf element failed\n",
				__func__);
			goto out_parse;
		}
		ret = parse_xml_scenario_element(root, ctl_inst->scenario_list);
		if (ret < 0) {
			RT_LOGE("%s, parse xml scenario element failed\n",
				__func__);
			goto out_parse;
		}
	} else {
		RT_LOGE("%s root is Invalid\n", __func__);
		ret = -EINVAL;
		goto out_parse;
	}

	RT_LOGD("%s successfully\n", __func__);
	return 0;
out_parse:
	delete doc;
	return ret;
}

static int rt_extamp_init(__unused struct SmartPa *smart_pa)
{
	int ret = 0;
	struct mixer *mMixer;

	RT_LOGD("%s: begin, mixer %p\n", __func__, smart_pa->attribute.mMixer);
	if (amp_ctl_inst != NULL) {
		RT_LOGD("%s amp_ctl_inst already inited\n", __func__);
		return 0;
	}

	amp_ctl_inst = new amp_control_inst(smart_pa->attribute.mMixer);
	if (!amp_ctl_inst) {
		RT_LOGE("%s: alloc amp control instance fail\n", __func__);
		return -ENOMEM;
	}
	ret = parse_xml_document(DEVICE_CONFIG_FILE, amp_ctl_inst);
	if (ret < 0) {
		RT_LOGE("%s: parse xml fail\n", __func__);
		goto out_xml_parse;
	}
	ret = amp_ctl_inst->apply_amp_conf();
	if (ret < 0)
		RT_LOGE("%s: apply amp conf fail\n", __func__);
	ret = amp_ctl_inst->apply_amp_scenario("init");
	if (ret < 0)
		RT_LOGE("%s: apply amp init scenario fail\n", __func__);

	mMixer = amp_ctl_inst->mixer;

	// only HW SmartPA(e.g. rt5509) has kcontrol "Calib_Start"
	if (mixer_get_ctl_by_name(mMixer, "Calib_Start") != NULL) {
		isHwSmartPAUsed = true;
	}
#ifdef MTK_SCENARIO
	if (!isHwSmartPAUsed) {
		bootMode = read_bootmode(BOOTMODE_PATH);
		if (bootMode == FACTORY_BOOT || bootMode == ATE_FACTORY_BOOT) {
			RT_LOGD("%s factory mode, -8db\n", __func__);
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0, 215))  {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Left Volume_Ctrl"), 0, 215))
					RT_LOGE("%s: set Left Volume_Ctrl fail\n", __func__);
			}
		}
	}
#endif /* MTK_SCENARIO */

	return 0;
out_xml_parse:
	delete amp_ctl_inst;
	amp_ctl_inst = NULL;
	return ret;
}

static int rt_extamp_deinit()
{
	int ret;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
		ret = amp_ctl_inst->apply_amp_scenario("deinit");
		if (ret < 0) {
			RT_LOGE("%s: apply amp on fail\n", __func__);
		}
		delete amp_ctl_inst;
		amp_ctl_inst = NULL;
	}
	return 0;
}

static int rt_extamp_speakerOn(struct SmartPaRuntime *runtime)
{
	int ret = 0;
	int is_dsp_on = 0;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
#ifdef MTK_SCENARIO
		if (runtime->device >= RT_DEV_MAX || runtime->device < 0 ||
			runtime->mode >= RT_CASE_MAX || runtime->mode < 0) {
			RT_LOGE("%s, Invalid device(%d), mode(%d)\n",
				__func__, runtime->device, runtime->mode);
			return -EINVAL;
		}
		RT_LOGD("%s: device = %d, mode = %d, set %s\n",
			__func__, runtime->device, runtime->mode, scenario[runtime->device][runtime->mode]);
		ret = amp_ctl_inst->apply_amp_scenario(scenario[runtime->device][runtime->mode]);
		if (ret < 0)
			RT_LOGE("%s: apply amp on fail\n", __func__);
#endif
		if (isHwSmartPAUsed || bootMode == FACTORY_BOOT || bootMode == ATE_FACTORY_BOOT)
			return ret;

		ret = mixer_ctl_get_value(mixer_get_ctl_by_name(amp_ctl_inst->mixer,
					"swdsp_smartpa_process_enable"), 0);
		is_dsp_on = 0;
		if (ret > 0 && (ret & 1)) {
			is_dsp_on = 1;
		} else {
			ret = mixer_ctl_get_value(mixer_get_ctl_by_name(amp_ctl_inst->mixer,
						  "scp_spk_process_enable"), 0);
			if (ret > 0 && (ret & 1))
				is_dsp_on = 1;
		}
		if (is_dsp_on) {
			RT_LOGD("%s _swdsp is enable\n", __func__);
		} else {
			RT_LOGD("%s _swdsp is disable\n", __func__);
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(amp_ctl_inst->mixer,
							"Volume_Ctrl"), 0, 215)) {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				if (mixer_ctl_set_value(mixer_get_ctl_by_name(amp_ctl_inst->mixer,
								"Left Volume_Ctrl"), 0, 215)) {
					RT_LOGE("%s: set Left Volume_Ctrl fail\n", __func__);
					return -1;
				}
			}
		}
	}
	return ret;
}

static int rt_extamp_speakerOff()
{
	int ret = 0;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
		ret = amp_ctl_inst->apply_amp_scenario("amp_off");
		if (ret < 0)
			RT_LOGE("%s: apply amp off fail\n", __func__);
	}
	return ret;
}

static int rt_extamp_speakerCalibrate(int state)
{
	int result = -1;
	int num = 0, size = 0;
	char calib_str[] = "calib_value: ";
	FILE *fd;
	static int volume = 231;
	char newline[128], calib_value[64];
	struct mixer *mMixer = amp_ctl_inst->mixer;

	memset(newline, '\0', 128);
	memset(calib_value, '\0', 64);
	switch (state) {
	case SPK_CALIB_STAGE_UNKNOWN:
		break;
	case SPK_CALIB_STAGE_INIT:
		if (!isHwSmartPAUsed) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "CS_Comp_Disable"), 0, 1)) {
				RT_LOGE("%s: set CS_Comp_Disable fail\n", __func__);
				if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "IS_TC_BYPASS"), 0, 1)) {
					RT_LOGE("%s: set IS_TC_BYPASS fail\n", __func__);
					return -1;
				}
			}
			volume = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0);
			if (volume < 0) {
				RT_LOGE("%s: get Volume_Ctrl value fail\n", __func__);
				return -1;
			}
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0, 231)) {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				return -1;
			}
		} else {
			if (mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Calib_Start"), 0) == 1) {
				ALOGD("%s(), already calibrated", __FUNCTION__);
				return -1;
			} else {
				return 0;
			}
		}
		break;
	case SPK_CALIB_STAGE_CALCULATE_AND_SAVE:
		if (isHwSmartPAUsed) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Calib_Start"), 0, 5526799)) {
				ALOGE("RT5509 trigger Calibration failed");
			} else {
				ALOGD("RT5509 trigger Calibration successfully");
			}
		}
		else {
			unsigned int val;
			F_ID smartpa_calib_fd;

			if (mixer_get_ctl_by_name(mMixer, "IS_TC_BYPASS") != NULL)
				fd = popen("rt5512_calibration -m 5526789 -2 /data/vendor/audiohal/audio_dump/*_ivdump.pcm", "r");
			else
				fd = popen("mt6660_calibration -m 5526789 -2 /data/vendor/audiohal/audio_dump/*_ivdump.pcm", "r");

			if (fd == NULL) {
				ALOGE("%s popen failed\n", __func__);
				break;
			}

			while((fgets(newline, 128, fd)) != NULL) {
				ALOGD("%s", newline);
				if (strncmp(newline, calib_str, strlen(calib_str)) == 0) {
					snprintf(calib_value, sizeof(calib_value), "%s", newline + strlen(calib_str));
					val = atoi(calib_value);
					ALOGD("calib_value:%d", val);
					unsigned int *file_lid;
					void *hcustlib = dlopen("libcustom_nvram.so", RTLD_LAZY);
					if (!hcustlib) {
						ALOGE("%s(), dlopen fail! (%s)\n", __FUNCTION__, dlerror());
						pclose(fd);
						return -1;
					}
					ALOGD("%s hcustlib: %p\n", __FUNCTION__, hcustlib);
					dlerror();    /* Clear any existing error */
					file_lid = (unsigned int*)dlsym(hcustlib, "iAP_CFG_CUSTOM_FILE_SMARTPA_CALIB_LID");
					if (file_lid == NULL) {
						ALOGE("%s get file_lid fail\n", __func__);
						pclose(fd);
						dlclose(hcustlib);
						return -1;
					}
					smartpa_calib_fd = NVM_GetFileDesc(*file_lid, &size, &num, ISWRITE);
					result = write(smartpa_calib_fd.iFileDesc, &val, size*num);
					if (result != size*num) {
						ALOGE("%s NVRAM write fail\n", __func__);
						pclose(fd);
						dlclose(hcustlib);
						return -1;
					}
					NVM_CloseFileDesc(smartpa_calib_fd);
					result = FileOp_BackupToBinRegion_All() ? 1 : 0;
					if (result) {
						ALOGE("ADP SmartPA calibration pass");
						result = 0;
					} else {
						ALOGD("ADP SmartPA calibration error");
						result = -1;
					}
					dlclose(hcustlib);
				}
			}
			pclose(fd);
		}
		break;
	case SPK_CALIB_STAGE_DEINIT:
		if (!isHwSmartPAUsed) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "CS_Comp_Disable"), 0, 0)) {
				ALOGW("%s(), CS_Comp_Disable fail", __func__);
				return -1;
			}
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0, volume)) {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				return -1;
			}
		}
		break;
	}
    return result;
}

int mtk_smartpa_init(struct SmartPa *smart_pa)
{
	smart_pa->ops.init = rt_extamp_init;
	smart_pa->ops.deinit = rt_extamp_deinit;
	smart_pa->ops.speakerOn = rt_extamp_speakerOn;
	smart_pa->ops.speakerOff = rt_extamp_speakerOff;
	smart_pa->ops.speakerCalibrate = rt_extamp_speakerCalibrate;
	return 0;
}
