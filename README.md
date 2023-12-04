![](/screenshots/sponza.jpg "VXGI 2020")

# VXGI

This is a deferred renderer with voxel cone traced indirect diffuse and specular reflections, ambient occlusion and soft shadows. The voxel grid is a mipmapped 3D texture which contains a voxelized representation of the directly lit polygonal scene. The cone travels through the mipmap levels and gathers samples along the way.

You're probably most interested in `./src/shaders/` and `./screenshots/`.

## Build
The application uses OpenGL 4.5, GLEW for loading OpenGL, GLFW for window management, tinyobjloader to load obj-files, lodepng to load png-images, ImGui for the UI and GLM as a math library. All the dependencies are included in the `lib`-folder.

Build with the following commands:
```sh
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

Run the binary from `data/vxgi.exe`. Press `F1` to enable/disable the UI. Use `WASD` to fly around.

You can also run the binary from the command line with a scene as an argument:
```sh
$ cd data
$ vxgi.exe sponza
```
Selectable scenes include `cornell`, `suzanne`, and `sponza`.

## Versions
```
v1.2 - 12/2023
* added cmake
* added glfw sources
* code cleanups

v1.1 - 11/2020
* public release

v1.0 - 06/2020
```

## References
See [Interactive Indirect Illumination Using Voxel Cone Tracing by Crassin et al](https://research.nvidia.com/sites/default/files/publications/GIVoxels-pg2011-authors.pdf).

Models and textures downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data).

## Licence
MIT