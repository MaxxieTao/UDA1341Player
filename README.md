# UDA1341Player
The audio player which based on the UDA1341 codec chip adopts Linux kernel version of 2.6.22.6 and complies with cross-compiler which is arm-linux-gcc version of 3.4.5. By analysing audio system architecture that based on the IIS bus, the UDA1341 codec chip and the L3 bus, designed the sound card driver. In order to control the audio and display audio information, the independent buttons and OLED display screen have added to the audio player. The application of this audio player attains to the expected effect of playing and recording.

基于UDA1341声卡的音频录播放器采用Linux 2.6.22.6版本内核，使用arm-linux-gcc 3.4.5版本交叉工具编译。通过分析IIS总线、UDA1341声卡和L3总线，采用字符型驱动设计了UDA1341声卡驱动、8位独立按键驱动、OLED显示屏驱动。应用程序实现了采集和播放音频的效果。
