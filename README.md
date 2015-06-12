喜欢用C++的同学可以用cpp目录下的.cpp文件，py目录下的文件是可供py调用的动态链接库（因为我是在windows环境下编译的，因此非windows环境的同学需要重新编译）。

我的环境配置信息如下：
vs2013
[openCV 2.49](http://opencv.org/downloads.html)
[FFmpeg](http://ffmpeg.zeranoe.com/builds/) <- 该链接仅适用于windows环境。

如果想自己编译成可被py调用的库，可以借助boost，我用的是[boost 1_56_0](http://www.boost.org/)