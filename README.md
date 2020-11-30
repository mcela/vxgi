![](/screenshots/sponza.jpg "VXGI 2020")

# VXGI

This is a deferred renderer with voxel cone traced indirect diffuse and specular reflections, ambient occlusion and soft shadows. The voxel grid is a mipmapped 3D texture which contains a voxelized representation of the directly lit polygonal scene. The cone travels through the mipmap levels and gathers samples along the way.

## Build
The application is using OpenGL 4.5, GLEW for loading OpenGL, GLFW for window management, tinyobjloader to load obj-files, lodepng to load png-images, ImGui for the UI and GLM as a math library. All the dependencies are included in the `lib`-folder, but you should have GLEW and GLFW installed from your package manager. The app has been developed and tested in Linux/Ubuntu, but works in Windows too.

Just take a look at the simple Makefile, you should be able to edit the settings for your setup. Build and run the app with the following commands:
```sh
$ make imgui
$ make scene=cornell run
```
Selectable scenes include `suzanne/cornell/sponza`.

Press `F1` to enable/disable the UI. Use `WASD` to fly around.

## References
See [Interactive Indirect Illumination Using Voxel Cone Tracing by Crassin et al](https://research.nvidia.com/sites/default/files/publications/GIVoxels-pg2011-authors.pdf).

Models and textures downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data).

## Licence
MIT