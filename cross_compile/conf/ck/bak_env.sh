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

# goxceed库版本路径
export GXLIB_PATH=`pwd`/../$CROSS_PATH
export GXSRC_PATH=`pwd`
# linux方案，内核代码路径
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
# CA枚举（与app_common_porting_stb_api.h中dvb_ca_type_t结构体中对应），配置参考对应CA代码目录下的readme.txt
unset DVB_CA_TYPE
# 对应市场（厂家）CA链接库名称（如libY1120-tonghui-gx3001-20121212D.a，设为Y1120-tonghui-gx3001-20121212D）
unset DVB_CA_1_LIB
unset DVB_CA_2_LIB
# 对应CA代码中宏定义
unset DVB_CA_1_FLAG
unset DVB_CA_2_FLAG
# 对应CA名称（目录）,如（cdcas3.0）
unset DVB_CA_1_NAME
unset DVB_CA_2_NAME

# 广告名称（目录），配置参考对应广告代码目录下的readme.txt
unset DVB_AD_NAME
# 广告类型
unset DVB_AD_TYPE
# 对应市场（厂家）广告链接库名称
unset DVB_AD_LIB

# OTA协议（类型）
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
# 主芯片类型
export CHIP=gx3201h
# demod芯片类型
export DEMOD=gx1801

export BUILD_DIR_NAME=libxml
#export BUILD_DIR_ARRAY_NAME=(jansson libxml)
export BUILD_DIR_ARRAY_NAME=(libxml)

# 客户硬件（晶辉诚）。
# 同一客户相同芯片方案，可能存在多版硬件设计，例如runde,rundeddr1,runde32m等
export DVB_CUSTOM=mjl
# 市场
export DVB_MARKET=mjl_abvca

export PVR_DIR='"/GxPvr"'
export IRR_SEARCH=1
export DVB_NETWORK_NAME=HCTN
export DVB_NETWORK_ID=0x64
export DVB_HOME_CHANNEL_LCN=0xffff
#export DVB_UPDATE_BASE_ON=sdt
export DVB_UPDATE_BASE_ON=pat

# CA市场
# 同一CA，可能存在不同市场版本
# 针对同一市场可能存在多个厂家。不同厂家因硬件平台、CA库等差异，需兼容差异化编译
# 对应CA目录gxapp_common/cas/$DVB_CA_1_NAME/$DVB_CA_MARKET/; gxapp_common/cas/$DVB_CA_2_NAME/$DVB_CA_MARKET/;
export DVB_CA_MARKET=gongban
# 广告市场
export DVB_AD_MARKET=gongban
export DVB_AD_MARKET_1=gongban

# 是否编译CA，如值为no则为清流版本
export DVB_CA_FLAG=yes

if [ $DVB_CA_FLAG = "yes" ]; then
	#CA枚举（与app_common_porting_stb_api.h中dvb_ca_type_t结构体中对应）
	export DVB_CA_TYPE=DVB_CA_TYPE_ABV38
	export DVB_CA_1_NAME=abvca38mjl
	#export DVB_CA_1_LIB=abvcamjl20171025
	export DVB_CA_1_LIB=AbvCaslib_S_GX3201H_r2028_0x290103_20171128_142137
	#export DVB_CA_1_LIB=abvca
	export DVB_CA_VERSION=ABV38_MJL_CA_VER
	export DVB_CA_1_FLAG=DVB_CA_TYPE_ABV_CAS38_FLAG
fi

#兼容双CA。多数情况下只有一个CA，第二个CA不用设置
#if [ $DVB_CA_FLAG = "yes" ]; then
#	export DVB_CA_2_NAME=byca
#	export DVB_CA_2_LIB=byca
#	export DVB_CA_2_FLAG=DVB_CA_TYPE_BY_FLAG
#fi

#广告DVB_AD_NAME

export DVB_AD_NAME=abvads
export DVB_AD_FLAG=DVB_AD_TYPE_ABV_FLAG
#export DVB_AD_LIB=ead3.1.2
export DVB_AD_LIB=ead3.2.3
# 选择OTA协议
export DVB_OTA_TYPE=DVB_NATIONALCHIP_OTA

# 设置loader中是否已开启jpg logo显示支持(需与gxloader中的.config开关一致)
export DVB_JPG_LOGO=yes

# 设置芯片是否支持ac3解码
export AUDIO_DOLBY=no

export ENABLE_SECURE=no
# 设置是否开启PVR功能（录制、时移等）
export DVB_PVR_FLAG=yes

# 设置是否开启水印功能
export DVB_ENABLE_WATERMARKET=yes
# 设置是否开启水印功能
export DVB_CHECK_FACTORY_VERSION=yes

export DVB_WATERMARK_X_POS=1100
export DVB_WATERMARK_Y_POS=560
export DVB_WATERMARK_X_WIDTH=80
export DVB_WATERMARK_Y_HIGHT=64

#设置检测youtube功能是否授权
export DVB_YOUTUBE_AUTH=no
#设置网络解锁密码
#export NETWORK_LOCK_PASSWORD=0110
export NETWORK_LOCK_PASSWORD=542865
export DVB_UPDATE_PASSWORD=5125

#设置网络错误提示框信息
export DVB_MSG_NETWORK_ERR='"STB has lost connectivity to HCTN. Please contact service provider."'

#设置全频点范围
export DVB_FULL_FREQ=yes
#设置全频点范围
#export DVB_FULL_FREQ_START=226
#export DVB_FULL_FREQ_END=826

export DVB_FULL_FREQ_START=354
export DVB_FULL_FREQ_END=538
#设置主频点信息
export DVB_HOME_FREQ=yes
#export DVB_HOME_FREQURY=818
export DVB_HOME_FREQURY=354
export DVB_HOME_SYMBOL=6875
export DVB_HOME_QAM=2

#设置在升级USB的时候是否避开data区域
export MANUAL_UPDATE_AVOID_DATA=no

#设置市场采用哪一组坐标位置
export ABV_POPMSG_POS=yes

#设置按照service 排序
export TAXIS_MODE_SERVICE_MODE=no
#设置按照lcn 排序
export TAXIS_MODE_LCN_MODE=yes

#设置是否开机网络锁功能
export DVB_NETWORK_LOCK=yes

# 设置是否开启硬解录制功能（录制CA指纹osd等）
export DVB_HW_PVR_FLAG=no

# 设置是否开启subtitle字幕显示功能
export DVB_SUBTITLE_FLAG=no

# 设置是否开启网络功能（youtube等） (仅linux方案支持此功能)
export DVB_NETWORK_FLAG=no

# 设置是否开启网络功能(ecos wifi相关,youtube,以后准备废弃旧版网络功能) 
export DVB_ECOS_WIFI=yes

# 设置是否支持OTT功能
export LINUX_OTT_SUPPORT=no

#是否支持USB升级
export DVB_USB_FLAG=yes

#是否支持HDMI的HDCP功能
export DVB_HDMI_HDCP=no
#是否支持CVBS的CGMS模式
export DVB_CVBS_CGMS=yes

#是否支持nit变更后自动搜索
export DVB_NIT_CHANGE_SEARCH=yes

#是否支持PVR快进、快退
export DVB_PVR_SPEED_SUPPORT=1

# 选用前面板类型 panel_xxx(市场)_xxx(厂家)_xxx(面板类型)，如PANEL_TYPE_fd650_RUNDE
# 已支持面板参考：include/bsp/panel_enum.h
export DVB_PANEL_TYPE=PANEL_TYPE_fd650_ABV
export PANEL_CLK_GPIO=18
export PANEL_DATA_GPIO=15
export PANEL_STANDBY_GPIO=19
export PANEL_LOCK_GPIO=33

# 选用遥控器宏定义类型 keymap_xxx(市场)_(厂家)，如KEY_GONGBAN_NATIONALCHIP_NEW
# 已支持面板参考:key_xml目录下的.xml group="KEY_GONGBAN_NATIONALCHIP_NEW"等，注意同一个kex(x).xml不能出现遥控器串键的情况（串键遥控器必须不不同key(x).xml中定义）
export DVB_KEY_TYPE=KEY_ABV

# 解调芯片类型 ，参考demod_enum.h定义
export DVB_DEMOD_TYPE=DVB_DEMOD_GX1801

# 解调模式，参考demod_enum.h定义（目前不支持DVB_DEMOD_DVBS）
export DVB_DEMOD_MODE=DVB_DEMOD_DVBC

# 配置选用第几路ts输出 : 可选0,1,2，
export DVB_TS_SRC=0

# 配置tuner类型，参考demod/include/tuner_neum.h定义
export DVB_TUNER_TYPE=TUNER_MXL608

# 选择开机画面jpg图片，logo-jpg/目录下为可配置jpg
export DVB_LOGO_JPG=logo_hctn.jpg
# 配置theme资源（有线、地面、标清、高清等），参考development/theme下对应目录
export DVB_THEME=INDIA_HD
if [ "$1" = "csky-linux" ] ; then
# XML、图片等路径
export DVB_WORK_PATH='"/dvb/"'
# 广播背景图片路径
export DVB_LOGO_PATH='"/dvb/theme/logo.bin"'
export DVB_I_FRAME_PATH='"/dvb/theme/bootlogo.bin"'
fi

if [ "$1" = "csky-ecos" ] ; then
# 广播背景图片路径
export DVB_LOGO_PATH='"/theme/logo.bin"'
export DVB_I_FRAME_PATH='"/theme/bootlogo.bin"'
export DVB_NETWORK_FLAG=no
fi


# 宏定义参数项
# 音视频同步方式 0-PCR_RECOVER 1-VPTS_RECOVER 2-APTS_RECOVER 3-AVPTS_RECOVER 4-NO_RECOVER
export DVB_SYS_MODE=0
# 全屏视频X位置
export DVB_VIDEO_X=0
# 全屏视频Y位置
export DVB_VIDEO_Y=0
# 全屏视频宽大小
export DVB_VIDEO_W=1280
# 全屏视频高大小
export DVB_VIDEO_H=720
# 卫星最大个数（有线、地面方案设置为1）
export DVB_SAT_MAX=1
# TP频点最大个数
export DVB_TP_MAX=200
# 节目最大个数
export DVB_SERVICE_MAX=1000
# DDRAM 大小
export DVB_DDRAM_SIZE=128
# FLASH 大小
export DVB_FLASH_SIZE=8
# 搜索PAT超时时间（ms）
export DVB_PAT_TIMEOUT=3000
# 搜索SDT超时时间（ms）
export DVB_SDT_TIMEOUT=8000
# 搜索NIT超时时间（ms）
export DVB_NIT_TIMEOUT=10000
# 搜索PMT超时时间（ms）
export DVB_PMT_TIMEOUT=8000
# LCN开启模式下，非标码流无逻辑频道号的节目，默认起始逻辑频道号
export DVB_LCN_DEFAULT=500
# 是否支持dvbc-dtmb双模
export DVB_DUAL_MODE=no
# 硬件自动测试开关。yes- 开启自动测试功能 no -关闭自动测试功能
export DVB_AUTO_TEST_FLAG=no
# 是否支持多媒体功能
export DVB_MEDIA_FLAG=yes
# 视频缩放是否重新播放
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






 




