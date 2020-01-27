# LedStrip
Adress Led Strip (ws2812) with STM32 microcontroller`s family

There are submodule in this project. Before build you should do the following:

1. git clone https://github.com/bohdan-chupys/LedStrip.git
2. git submodule init
3. git submodule update

# Kbuild system
In release-2.1 we have significant change - add kbuild system. This allow to perform small tuning of ws2812 diver just before compilation.
Thats why, you need to configure this project before compiling it.

Steps to build project:
1. make menuconfig - configure project; generate .config and .config.h
2. make - build project, according to generated .config file
3. Enjoy

If you do not issue make menuconfig; build system still force you to configure it. So if build system doesn`t find any .config and .config.h file, it will ask about evenry config options before compilation.
