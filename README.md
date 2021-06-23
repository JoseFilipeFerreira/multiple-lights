# Computer Graphics Spring 2021

## Projects Dependencies

+ [CMake](https://cmake.org/)
+ [Intel TBB](https://software.intel.com/content/www/us/en/develop/tools/threading-building-blocks.html)
+ [Intel ISPC compiler](https://ispc.github.io/)
+ [Intel Embree](https://www.embree.org/)
+ [Intel OSPray](https://www.ospray.org/)
+ [Dear ImGui](https://github.com/ocornut/imgui)
+ [JSON Cpp](https://github.com/open-source-parsers/jsoncpp)

## Linux and MacOs dependencies build

There is a local-thrparty folder with a buildeps script. The builddeps script will download, configure, compile and install into local-thrparty folder.

It will also generate a enviroment variable script that will setup the build environment.
```
source local-thrparty/install/bin/setenv
```

## Scene description

Use WaveFront OBJ format to describe the scene, however to extend the material capabilities a json format was created.

The JSON material names must match the material names on the OBJ and mtl file. Each material describes a materital type from ospray and the respective parameters. ( [OSPray Materials](https://www.ospray.org/documentation.html#materials) )




## Building the project Linux/MacOs

Make sure the enviroment variables defined in "setenv" point to the proper install paths. On windows, make sure they are defined on the proper place.

```
mkdir build
cd build
cmake ..
```

Builing the project

```
cmake --build .
```

or
```
make
```

## Building the project on Windows

You can use the CMake to generate Visual Studio Projects.

Make sure the enviroment variables defined in "setenv" point to the proper install paths. On windows, make sure they are defined on the proper place.

Here is a tutorial to use CMake Projects in Visual Studio. (https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)


## Runing the exemple
```
./CGViewer2021 -m models/cornell/CornellBox-Water.obj
```

## Other Notes

#### Editor

This project has the proper Visual Studio Code settings, assuming that you have setup the enviroment variables in setenv (if you don's use the builddeps script, the end of the script here the setenv is created).

#### CMake building framework

If you are not familiar with CMake, please take sometime. It will help when adding source files to the application builing step. 



# TODO

[ ] - Add camera position and directiom

[ ] - Add texture loading

[ ] - Add camera controls to gui

[ ] - Add image size parameter command line

[ ] - Add command line render parameters

[ ] - Add render to image file
