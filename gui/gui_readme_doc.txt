1.this is simulator project
2.配置文件lv_drv_conf.h
3.lvgl使用版本v8.0.0
4.lvgl官方字体转换器地址： https://lvgl.io/tools/fontconverter
4-1最简单的方法是使用【在线字体转换器】(https://lvgl.io/tools/fontconverter)。只需设置参数，单击Convert 按钮，将字体复制到您的项目并使用它。 请务必仔细阅读该网站上提供的步骤，否则转换时会出错。
4-2使用【离线字体转换器】(https://github.com/lvgl/lv_font_conv)。 （需要安装 Node.js）
4-3如果您想创建类似内置字体（Roboto 字体和符号）但大小和/或范围不同的内容，您可以使用 lvgl/scripts/built_in_font 文件夹中的 built_in_font_gen.py 脚本。 （这需要安装 Python 和 lv_font_conv）
4-4要在文件中声明字体，请使用LV_FONT_DECLARE(my_font_name)。
要使字体全局可用（如内置字体），请将它们添加到 lv_conf.h 中的 LV_FONT_CUSTOM_DECLARE。
