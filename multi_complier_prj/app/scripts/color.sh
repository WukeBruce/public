#!/bin/bash
#sh path ./gxappcommon

#==========================
#| 色彩 | 前景色 | 背景色|
#| 黑   | 30	 | 40	 |
#| 红   | 31	 | 41	 |
#| 绿   | 32     | 42    |
#| 黄   | 33     | 43	 |
#| 蓝   | 34     | 44	 |
#| 洋红 | 35	 | 45	 |
#| 青   | 36     | 46	 |
#| 白   | 37 	 | 47	 |
#==========================
################################StartScript#######################################
black='\E[30m'
red='\E[31m'
green='\E[32m'
yellow='\E[33m'
blue='\E[34m'
magenta='\E[35m'
cyan='\E[36m'
white='\E[37m'

#${stringZ:0}
#let 
#$1:string  $2:color $3: 0 1
cecho ()
{
	local default_msg="message NULL."
	message=${1:-$default_msg} # 默认的信息.
	color=${2:-$black}
	if [ "1" = "$3" ];then
		echo -e "$color""\033[1m $message \033[0m"
	else	
		echo -e "$color""$message \033[0m"
	fi					
	# 重设文本属性.
	return
}

# 2) 增加可选的彩色背景.



cecho "	source color shell ..." $blue

