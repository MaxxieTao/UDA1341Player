1、目录结构
bin目录下为应用程序可执行文件
driver目录下为驱动文件和make后生成的.ko文件
obj目录下为目标文件
include目录下为头文件
src目录下为应用程序源文件
./code
|-- bin
|   
|-- driver
|   |-- uda1341.c
|   |-- button.c
|   |-- oled.c
|   |-- Makefile 
|    
|-- include
|   |-- base.h
|   |-- file.h
|   |-- font.h
|   |-- graphy.h
|   |-- handle.h
|   |-- main.h
|   |-- player.h
|   |-- rtc.h
|   |-- ui.h
|   
|-- obj
|
|-- src
|   |-- file.c
|   |-- font.c
|   |-- graphy.c
|   |-- handle.c
|   |-- main.c
|   |-- player.c
|   |-- rtc.c
|   |-- ui.c
|-- Makefile

6 directories, 22 files

2、内核版本为Linux 2.6.22.6
交叉编译工具版本为gcc-3.4.5-glibc-2.3.6

3、由于交叉编译工具版本较低，需要更新libiconv库，才可以正常调用font.c中的iconv_open函数
（1）解压libiconv-1.14.tar.gz
（2）在libiconv目录下：
	./configure --prefix=$PWD/out --host=arm-linux
	make
	make install
	上述命令执行完成后会在libiconv目录下生成新的out目录，该目录下存在4个目录分别是：
	bin include lib share
（3）在lib目录下为生成的库文件，其中一个为preloadable_libiconv.so，把它下载到开发板的lib目录下，然后再设置开发板的系统环境变量：
	$ export LD_PRELOAD=/lib/preloadable_libiconv.so
4、guard.sh
guard.sh为守护脚本，与main文件放在同一目录下，可防止系统出错退出后不再运行进程，需要额外运行（非必须）。