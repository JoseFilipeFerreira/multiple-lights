# multiple-lights

Exploring different approaches to optimize the rendering of scenes with multiple lights
with the objective of improving rendering time while minimizing noise. (VI2 assignment)

The explored methods were:
* pooling all lights from the scene
* use a uniform distribution to fetch one random light per point
* choose a random light based on how close it is to the point being calculated

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

## Building the project Linux/MacOs

Make sure the enviroment variables defined in "setenv" point to the proper install paths. On windows, make sure they are defined on the proper place.

```
mkdir build
cd build
cmake ..
```

Builing the project
```
make
```

## Runing the exemple
```
./CGViewer2021 -m models/cornell/CornellBox-Water.obj
```

## Authors
* [Jose Filipe Ferreira](https://github.com/JoseFilipeFerreira)
* [Jorge Mota](https://github.com/K1llByte)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file
for details
