# 简介

喜欢用C++的同学可以用cpp目录下的.cpp文件，py目录下的文件是可供py调用的动态链接库（因为我是在windows环境下编译的，因此非windows环境的同学需要重新编译）。

# 配置信息
我的环境配置信息如下：

vs2013

[openCV 2.49](http://opencv.org/downloads.html)

[FFmpeg](http://ffmpeg.zeranoe.com/builds/) <- 该链接仅适用于windows环境。

如果想自己编译成可被py调用的库，可以借助boost，我用的是[boost 1_56_0](http://www.boost.org/)

# 参考文献

[直方图匹配](http://blog.csdn.net/xiaowei_cqu/article/details/7606607)
[搭建Visual Studio 2013 + FFMpeg环境](http://www.cnblogs.com/myffx/p/3453488.html)
[VS2013下OpenCV2.48配置](http://blog.csdn.net/gameloong/article/details/18242823)
[ffmpeg如何提取视频的关键帧](http://ask.5lulu.com/question/e3n6g5nn9g89c8.html)
[使用boost.python在python调用C++接口](http://blog.csdn.net/lzl001/article/details/20120451)
