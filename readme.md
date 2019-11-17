# 因时机械爪驱动

共两个版本

1. C++
2. Python



## C++ 使用方法

1. 将 cpp 文件夹复制到工程下 `3rdparty` 目录下，并改名为`talon`（注意，大小写敏感）

2. 在项目的 `CMakeLists.txt` 中添加如下代码

```cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/talon)
```

3. 上一步也可以将`cpp`中的`cmake`文件拷贝到你的`CMAKE_MODULE_PATH`中
4. 使用`find_package(Talon)`导入package
5. 在`target_link_libraries`中添加`Talon`，以此来使用代码。
6. 以上...



## Python 使用方法

> 直接拖入项目中即可