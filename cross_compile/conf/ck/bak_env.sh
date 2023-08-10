#/bin/bash

unset CHIP
unset DEMOD
unset ARCH
unset OS
unset CROSS_PATH
unset GXLIB_PATH
unset GXSRC_PATH
unset GX_KERNEL_PATH

if [ "$1" = "csky-ecos" ] ; then
	echo ARCH=csky OS=ecos Configuration  !
	export ARCH=csky
	export OS=ecos
	export CROSS_PATH=csky-ecos
fi

if [ "$1" = "csky-linux" ] ; then
	echo ARCH=csky OS=linux Configuration  !
	export ARCH=csky
	export OS=linux
	export CROSS_PATH=csky-linux
fi

if [ -z "$CROSS_PATH" ] ; then
	echo -e "\033[31m Config Errror!!! ,please check  ARCH_OS \033[0m"
	echo "               "              
	echo eg: source env.sh csky-ecos
	echo "    "source env.sh csky-linux
fi

# goxceed��汾·��
export GXLIB_PATH=`pwd`/../$CROSS_PATH
export GXSRC_PATH=`pwd`
# linux�������ں˴���·��
export GX_KERNEL_PATH=$GXSRC_PATH/../linux-2.6.27.55

unset DVB_CUSTOM
unset DVB_MARKET
unset DVB_CA_MARKET
unset DVB_AD_MARKET
unset DVB_CA_FLAG
unset DVB_USB_FLAG
unset DVB_PVR_SPEED_SUPPORT
unset DVB_PANEL_TYPE
unset PANEL_CLK_GPIO
unset PANEL_DATA_GPIO
unset PANEL_STANDBY_GPIO
unset PANEL_LOCK_GPIO
unset DVB_KEY_TYPE
unset DVB_TUNER_TYPE
unset DVB_DEMOD_TYPE
unset DVB_DEMOD_MODE
unset DVB_TS_SRC
unset DVB_AUTO_TEST_FLAG
unset DVB_THEME
unset DVB_MEDIA_FLAG
unset DVB_ZOOM_RESTART_PLAY
unset DVB_BAD_SIGNAL_SHOW_LOGO
unset ENABLE_SECURE
# CAö�٣���app_common_porting_stb_api.h��dvb_ca_type_t�ṹ���ж�Ӧ�������òο���ӦCA����Ŀ¼�µ�readme.txt
unset DVB_CA_TYPE
# ��Ӧ�г������ң�CA���ӿ����ƣ���libY1120-tonghui-gx3001-20121212D.a����ΪY1120-tonghui-gx3001-20121212D��
unset DVB_CA_1_LIB
unset DVB_CA_2_LIB
# ��ӦCA�����к궨��
unset DVB_CA_1_FLAG
unset DVB_CA_2_FLAG
# ��ӦCA���ƣ�Ŀ¼��,�磨cdcas3.0��
unset DVB_CA_1_NAME
unset DVB_CA_2_NAME

# ������ƣ�Ŀ¼�������òο���Ӧ������Ŀ¼�µ�readme.txt
unset DVB_AD_NAME
# �������
unset DVB_AD_TYPE
# ��Ӧ�г������ң�������ӿ�����
unset DVB_AD_LIB

# OTAЭ�飨���ͣ�
unset DVB_OTA_TYPE

unset DVB_JPG_LOGO
unset AUDIO_DOLBY
unset DVB_SUBTITLE_FLAG
unset DVB_PVR_FLAG
unset DVB_NETWORK_FLAG
unset DVB_ECOS_WIFI
unset LINUX_OTT_SUPPORT

unset DVB_LOGO_JPG
unset DVB_WORK_PATH
unset DVB_LOGO_PATH
unset DVB_I_FRAME_PATH
unset DVB_SYS_MODE
unset DVB_VIDEO_X
unset DVB_VIDEO_Y
unset DVB_VIDEO_W
unset DVB_VIDEO_H
unset DVB_SAT_MAX
unset DVB_TP_MAX
unset DVB_SERVICE_MAX
unset DVB_DDRAM_SIZE
unset DVB_FLASH_SIZE
unset DVB_PAT_TIMEOUT
unset DVB_SDT_TIMEOUT
unset DVB_NIT_TIMEOUT
unset DVB_PMT_TIMEOUT
unset DVB_LCN_DEFAULT
unset DVB_NETWORK_ID
unset DVB_NETWORK_NAME
unset DVB_HOME_CHANNEL_LCN
unset DVB_UPDATE_BASE_ON
unset DVB_HDMI_HDCP
unset DVB_CVBS_CGMS
unset DVB_LCN_BASE_BAT
unset DVB_CHECK_101
unset DVB_CHECK_102
unset DVB_NIT_CHANGE_SEARCH 
unset DVB_OPEN_AUTO_SEARCH_FLAG
unset DVB_NETWORK_LOCK
unset DVB_ENABLE_WATERMARKET
unset DVB_WATERMARK_X_POS
unset DVB_WATERMARK_Y_POS
unset DVB_WATERMARK_X_WIDTH	
unset DVB_WATERMARK_Y_HIGHT
unset DVB_YOUTUBE_AUTH
unset DVB_FULL_FREQ

unset DVB_FULL_FREQ_START
unset DVB_FULL_FREQ_END

unset DVB_HOME_FREQ
unset DVB_HOME_FREQURY
unset DVB_HOME_SYMBOL
unset DVB_HOME_QAM
unset TAXIS_MODE_SERVICE_MODE
unset TAXIS_MODE_LCN_MODE

unset NETWORK_LOCK_PASSWORD
unset DVB_UPDATE_PASSWORD
unset DVB_CHECK_FACTORY_VERSION
unset DVB_MSG_NETWORK_ERR
unset MANUAL_UPDATE_AVOID_DATA
unset ABV_POPMSG_POS
unset DVB_CA_VERSION
# ��оƬ����
export CHIP=gx3201h
# demodоƬ����
export DEMOD=gx1801

export BUILD_DIR_NAME=libxml
#export BUILD_DIR_ARRAY_NAME=(jansson libxml)
export BUILD_DIR_ARRAY_NAME=(libxml)

# �ͻ�Ӳ�������Գϣ���
# ͬһ�ͻ���ͬоƬ���������ܴ��ڶ��Ӳ����ƣ�����runde,rundeddr1,runde32m��
export DVB_CUSTOM=mjl
# �г�
export DVB_MARKET=mjl_abvca

export PVR_DIR='"/GxPvr"'
export IRR_SEARCH=1
export DVB_NETWORK_NAME=HCTN
export DVB_NETWORK_ID=0x64
export DVB_HOME_CHANNEL_LCN=0xffff
#export DVB_UPDATE_BASE_ON=sdt
export DVB_UPDATE_BASE_ON=pat

# CA�г�
# ͬһCA�����ܴ��ڲ�ͬ�г��汾
# ���ͬһ�г����ܴ��ڶ�����ҡ���ͬ������Ӳ��ƽ̨��CA��Ȳ��죬����ݲ��컯����
# ��ӦCAĿ¼gxapp_common/cas/$DVB_CA_1_NAME/$DVB_CA_MARKET/; gxapp_common/cas/$DVB_CA_2_NAME/$DVB_CA_MARKET/;
export DVB_CA_MARKET=gongban
# ����г�
export DVB_AD_MARKET=gongban
export DVB_AD_MARKET_1=gongban

# �Ƿ����CA����ֵΪno��Ϊ�����汾
export DVB_CA_FLAG=yes

if [ $DVB_CA_FLAG = "yes" ]; then
	#CAö�٣���app_common_porting_stb_api.h��dvb_ca_type_t�ṹ���ж�Ӧ��
	export DVB_CA_TYPE=DVB_CA_TYPE_ABV38
	export DVB_CA_1_NAME=abvca38mjl
	#export DVB_CA_1_LIB=abvcamjl20171025
	export DVB_CA_1_LIB=AbvCaslib_S_GX3201H_r2028_0x290103_20171128_142137
	#export DVB_CA_1_LIB=abvca
	export DVB_CA_VERSION=ABV38_MJL_CA_VER
	export DVB_CA_1_FLAG=DVB_CA_TYPE_ABV_CAS38_FLAG
fi

#����˫CA�����������ֻ��һ��CA���ڶ���CA��������
#if [ $DVB_CA_FLAG = "yes" ]; then
#	export DVB_CA_2_NAME=byca
#	export DVB_CA_2_LIB=byca
#	export DVB_CA_2_FLAG=DVB_CA_TYPE_BY_FLAG
#fi

#���DVB_AD_NAME

export DVB_AD_NAME=abvads
export DVB_AD_FLAG=DVB_AD_TYPE_ABV_FLAG
#export DVB_AD_LIB=ead3.1.2
export DVB_AD_LIB=ead3.2.3
# ѡ��OTAЭ��
export DVB_OTA_TYPE=DVB_NATIONALCHIP_OTA

# ����loader���Ƿ��ѿ���jpg logo��ʾ֧��(����gxloader�е�.config����һ��)
export DVB_JPG_LOGO=yes

# ����оƬ�Ƿ�֧��ac3����
export AUDIO_DOLBY=no

export ENABLE_SECURE=no
# �����Ƿ���PVR���ܣ�¼�ơ�ʱ�Ƶȣ�
export DVB_PVR_FLAG=yes

# �����Ƿ���ˮӡ����
export DVB_ENABLE_WATERMARKET=yes
# �����Ƿ���ˮӡ����
export DVB_CHECK_FACTORY_VERSION=yes

export DVB_WATERMARK_X_POS=1100
export DVB_WATERMARK_Y_POS=560
export DVB_WATERMARK_X_WIDTH=80
export DVB_WATERMARK_Y_HIGHT=64

#���ü��youtube�����Ƿ���Ȩ
export DVB_YOUTUBE_AUTH=no
#���������������
#export NETWORK_LOCK_PASSWORD=0110
export NETWORK_LOCK_PASSWORD=542865
export DVB_UPDATE_PASSWORD=5125

#�������������ʾ����Ϣ
export DVB_MSG_NETWORK_ERR='"STB has lost connectivity to HCTN. Please contact service provider."'

#����ȫƵ�㷶Χ
export DVB_FULL_FREQ=yes
#����ȫƵ�㷶Χ
#export DVB_FULL_FREQ_START=226
#export DVB_FULL_FREQ_END=826

export DVB_FULL_FREQ_START=354
export DVB_FULL_FREQ_END=538
#������Ƶ����Ϣ
export DVB_HOME_FREQ=yes
#export DVB_HOME_FREQURY=818
export DVB_HOME_FREQURY=354
export DVB_HOME_SYMBOL=6875
export DVB_HOME_QAM=2

#����������USB��ʱ���Ƿ�ܿ�data����
export MANUAL_UPDATE_AVOID_DATA=no

#�����г�������һ������λ��
export ABV_POPMSG_POS=yes

#���ð���service ����
export TAXIS_MODE_SERVICE_MODE=no
#���ð���lcn ����
export TAXIS_MODE_LCN_MODE=yes

#�����Ƿ񿪻�����������
export DVB_NETWORK_LOCK=yes

# �����Ƿ���Ӳ��¼�ƹ��ܣ�¼��CAָ��osd�ȣ�
export DVB_HW_PVR_FLAG=no

# �����Ƿ���subtitle��Ļ��ʾ����
export DVB_SUBTITLE_FLAG=no

# �����Ƿ������繦�ܣ�youtube�ȣ� (��linux����֧�ִ˹���)
export DVB_NETWORK_FLAG=no

# �����Ƿ������繦��(ecos wifi���,youtube,�Ժ�׼�������ɰ����繦��) 
export DVB_ECOS_WIFI=yes

# �����Ƿ�֧��OTT����
export LINUX_OTT_SUPPORT=no

#�Ƿ�֧��USB����
export DVB_USB_FLAG=yes

#�Ƿ�֧��HDMI��HDCP����
export DVB_HDMI_HDCP=no
#�Ƿ�֧��CVBS��CGMSģʽ
export DVB_CVBS_CGMS=yes

#�Ƿ�֧��nit������Զ�����
export DVB_NIT_CHANGE_SEARCH=yes

#�Ƿ�֧��PVR���������
export DVB_PVR_SPEED_SUPPORT=1

# ѡ��ǰ������� panel_xxx(�г�)_xxx(����)_xxx(�������)����PANEL_TYPE_fd650_RUNDE
# ��֧�����ο���include/bsp/panel_enum.h
export DVB_PANEL_TYPE=PANEL_TYPE_fd650_ABV
export PANEL_CLK_GPIO=18
export PANEL_DATA_GPIO=15
export PANEL_STANDBY_GPIO=19
export PANEL_LOCK_GPIO=33

# ѡ��ң�����궨������ keymap_xxx(�г�)_(����)����KEY_GONGBAN_NATIONALCHIP_NEW
# ��֧�����ο�:key_xmlĿ¼�µ�.xml group="KEY_GONGBAN_NATIONALCHIP_NEW"�ȣ�ע��ͬһ��kex(x).xml���ܳ���ң�������������������ң�������벻��ͬkey(x).xml�ж��壩
export DVB_KEY_TYPE=KEY_ABV

# ���оƬ���� ���ο�demod_enum.h����
export DVB_DEMOD_TYPE=DVB_DEMOD_GX1801

# ���ģʽ���ο�demod_enum.h���壨Ŀǰ��֧��DVB_DEMOD_DVBS��
export DVB_DEMOD_MODE=DVB_DEMOD_DVBC

# ����ѡ�õڼ�·ts��� : ��ѡ0,1,2��
export DVB_TS_SRC=0

# ����tuner���ͣ��ο�demod/include/tuner_neum.h����
export DVB_TUNER_TYPE=TUNER_MXL608

# ѡ�񿪻�����jpgͼƬ��logo-jpg/Ŀ¼��Ϊ������jpg
export DVB_LOGO_JPG=logo_hctn.jpg
# ����theme��Դ�����ߡ����桢���塢����ȣ����ο�development/theme�¶�ӦĿ¼
export DVB_THEME=INDIA_HD
if [ "$1" = "csky-linux" ] ; then
# XML��ͼƬ��·��
export DVB_WORK_PATH='"/dvb/"'
# �㲥����ͼƬ·��
export DVB_LOGO_PATH='"/dvb/theme/logo.bin"'
export DVB_I_FRAME_PATH='"/dvb/theme/bootlogo.bin"'
fi

if [ "$1" = "csky-ecos" ] ; then
# �㲥����ͼƬ·��
export DVB_LOGO_PATH='"/theme/logo.bin"'
export DVB_I_FRAME_PATH='"/theme/bootlogo.bin"'
export DVB_NETWORK_FLAG=no
fi


# �궨�������
# ����Ƶͬ����ʽ 0-PCR_RECOVER 1-VPTS_RECOVER 2-APTS_RECOVER 3-AVPTS_RECOVER 4-NO_RECOVER
export DVB_SYS_MODE=0
# ȫ����ƵXλ��
export DVB_VIDEO_X=0
# ȫ����ƵYλ��
export DVB_VIDEO_Y=0
# ȫ����Ƶ���С
export DVB_VIDEO_W=1280
# ȫ����Ƶ�ߴ�С
export DVB_VIDEO_H=720
# ���������������ߡ����淽������Ϊ1��
export DVB_SAT_MAX=1
# TPƵ��������
export DVB_TP_MAX=200
# ��Ŀ������
export DVB_SERVICE_MAX=1000
# DDRAM ��С
export DVB_DDRAM_SIZE=128
# FLASH ��С
export DVB_FLASH_SIZE=8
# ����PAT��ʱʱ�䣨ms��
export DVB_PAT_TIMEOUT=3000
# ����SDT��ʱʱ�䣨ms��
export DVB_SDT_TIMEOUT=8000
# ����NIT��ʱʱ�䣨ms��
export DVB_NIT_TIMEOUT=10000
# ����PMT��ʱʱ�䣨ms��
export DVB_PMT_TIMEOUT=8000
# LCN����ģʽ�£��Ǳ��������߼�Ƶ���ŵĽ�Ŀ��Ĭ����ʼ�߼�Ƶ����
export DVB_LCN_DEFAULT=500
# �Ƿ�֧��dvbc-dtmb˫ģ
export DVB_DUAL_MODE=no
# Ӳ���Զ����Կ��ء�yes- �����Զ����Թ��� no -�ر��Զ����Թ���
export DVB_AUTO_TEST_FLAG=no
# �Ƿ�֧�ֶ�ý�幦��
export DVB_MEDIA_FLAG=yes
# ��Ƶ�����Ƿ����²���
export DVB_ZOOM_RESTART_PLAY=no
export DVB_BAD_SIGNAL_SHOW_LOGO=no
export DVB_OPEN_AUTO_SEARCH_FLAG=no
# echo export path
if [ -z "$CROSS_PATH" ] ; then
	echo
else
	echo CHIP=$CHIP
	echo DEMOD=$DEMOD
	echo PATH:
	echo GX_KERNEL_PATH=$GX_KERNEL_PATH
	echo GXLIB_PATH=$GXLIB_PATH
	echo
	echo
	echo DVB_CUSTOM=$DVB_CUSTOM
	echo DVB_MARKET=$DVB_MARKET
	echo DVB_NIT_CHANGE_SEARCH=$DVB_NIT_CHANGE_SEARCH
	echo DVB_OPEN_AUTO_SEARCH_FLAG=$DVB_OPEN_AUTO_SEARCH_FLAG
	echo DVB_CA_FLAG=$DVB_CA_FLAG
	echo DVB_ENABLE_WATERMARKET=$DVB_ENABLE_WATERMARKET
	echo DVB_NETWORK_LOCK=$DVB_NETWORK_LOCK
	echo DVB_WATERMARK_X_POS=$DVB_WATERMARK_X_POS
	echo DVB_WATERMARK_Y_POS=$DVB_WATERMARK_Y_POS
	echo DVB_WATERMARK_X_WIDTH=$DVB_WATERMARK_X_WIDTH
	echo DVB_WATERMARK_Y_HIGHT=$DVB_WATERMARK_Y_HIGHT
	echo DVB_YOUTUBE_AUTH=$DVB_YOUTUBE_AUTH
	echo DVB_FULL_FREQ=$DVB_FULL_FREQ
	echo DVB_FULL_FREQ_START=$DVB_FULL_FREQ_START
	echo DVB_FULL_FREQ_END=$DVB_FULL_FREQ_END
	echo DVB_HOME_FREQ=$DVB_HOME_FREQ
	echo DVB_HOME_FREQURY=$DVB_HOME_FREQURY
	echo DVB_HOME_SYMBOL=$DVB_HOME_SYMBOL
	echo DVB_HOME_QAM=$DVB_HOME_QAM
	echo TAXIS_MODE_SERVICE_MODE=$TAXIS_MODE_SERVICE_MODE
	echo TAXIS_MODE_LCN_MODE=$TAXIS_MODE_LCN_MODE
	echo DVB_CHECK_FACTORY_VERSION=$DVB_CHECK_FACTORY_VERSION
	echo NETWORK_LOCK_PASSWORD=$NETWORK_LOCK_PASSWORD
	echo DVB_UPDATE_PASSWORD=$DVB_UPDATE_PASSWORD
	echo DVB_MSG_NETWORK_ERR=$DVB_MSG_NETWORK_ERR
	echo MANUAL_UPDATE_AVOID_DATA=$MANUAL_UPDATE_AVOID_DATA
	echo ABV_POPMSG_POS=$ABV_POPMSG_POS
	echo
	echo
	if [ $DVB_CA_FLAG = "yes" ]; then
		echo DVB_CA_MARKET=$DVB_CA_MARKET
		echo DVB_CA_TYPE=$DVB_CA_TYPE
		echo DVB_CA_1_NAME=$DVB_CA_1_NAME
		echo DVB_CA_VERSION=$DVB_CA_VERSION
		echo DVB_CA_1_LIB=$DVB_CA_1_LIB
#		echo DVB_CA_1_FLAG=$DVB_CA_1_FLAG
#		echo DVB_CA_2_NAME=$DVB_CA_2_NAME
#		echo DVB_CA_2_LIB=$DVB_CA_2_LIB
#		echo DVB_CA_2_FLAG=$DVB_CA_2_FLAG
	fi
	if [ -z "$DVB_AD_NAME" ] ; then
		echo
	else
		echo DVB_AD_MARKET=$DVB_AD_MARKET
		echo DVB_AD_NAME=$DVB_AD_NAME
#		echo DVB_AD_FLAG=$DVB_AD_FLAG
#		echo DVB_AD_LIB=$DVB_AD_LIB
	fi
	echo DVB_JPG_LOGO=$DVB_JPG_LOGO
	echo DVB_USB_FLAG=$DVB_USB_FLAG
	echo DVB_PVR_FLAG=$DVB_PVR_FLAG
	echo DVB_SUBTITLE_FLAG=$DVB_SUBTITLE_FLAG
	echo AUDIO_DOLBY=$AUDIO_DOLBY
	echo DVB_NETWORK_FLAG=$DVB_NETWORK_FLAG
	echo DVB_ECOS_WIFI=$DVB_ECOS_WIFI
	echo LINUX_OTT_SUPPORT=$LINUX_OTT_SUPPORT
	echo DVB_PVR_SPEED_SUPPORT=$DVB_PVR_SPEED_SUPPORT
	echo DVB_PANEL_TYPE=$DVB_PANEL_TYPE
	echo DVB_KEY_TYPE=$DVB_KEY_TYPE
	echo DVB_LOGO_JPG=$DVB_LOGO_JPG
	echo DVB_TUNER_TYPE=$DVB_TUNER_TYPE
	echo DVB_DEMOD_TYPE=$DVB_DEMOD_TYPE
	echo DVB_DEMOD_MODE=$DVB_DEMOD_MODE
	echo DVB_TS_SRC=$DVB_TS_SRC
	echo DVB_SYS_MODE=$DVB_SYS_MODE
	echo DVB_DDRAM_SIZE=$DVB_DDRAM_SIZE
	echo DVB_FLASH_SIZE=$DVB_FLASH_SIZE
	echo DVB_DUAL_MODE=$DVB_DUAL_MODE
    echo DVB_AUTO_TEST_FLAG=$DVB_AUTO_TEST_FLAG
	echo DVB_THEME=$DVB_THEME
	echo DVB_MEDIA_FLAG=$DVB_MEDIA_FLAG
	echo DVB_ZOOM_RESTART_PLAY=$DVB_ZOOM_RESTART_PLAY
	echo DVB_BAD_SIGNAL_SHOW_LOGO=$DVB_BAD_SIGNAL_SHOW_LOGO
	echo BUILD_DIR_NAME=$BUILD_DIR_NAME
	echo BUILD_DIR_ARRAY_NAME=${BUILD_DIR_ARRAY_NAME[*]}
fi






 




