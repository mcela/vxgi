# VXGI 2020
#   Nicolas Mäkelä
#
#
# $ make imgui
# $ make run (builds and runs) / runonly / build
#
# You can also specify which scene to run like so:
# > make scene=sponza run
#
# Make sure you have GLEW and GLFW installed. Also load sponza textures and dump them into data/textures.
#

ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

#
# Windows 10
#
# I use cmder + mingw64. Add mingw/bin folder to $PATH. Change mingw32-make.exe to make.exe in the bin folder.
# Make sure to install x86_64 version of mingw. Download glfw3 and put the correct 64 bit libs into mingw/libs folder.
# 
ifeq ($(detected_OS),Windows)
IMGUI := imgui-1.75
CC := g++
CFLAGS := -std=c++17 -Wall -fmessage-length=0 -Wno-unused-function -Wno-unused-variable 
CINC := -I./src -I./lib -I./lib/$(IMGUI) -I/usr/local/include
CMAIN := src/main.cpp

# cmd line arguments for the application
scene="none"

build: $(CMAIN)
	$(CC) $(CFLAGS) -m64 -g -ggdb -O0 $(CINC) $(CMAIN) -o bin/windowsDebug.exe bin/libglfw3.a -lglu32 -lopengl32 -lgdi32 bin/libimgui_linux.a 

run: build
	cd data && ../bin/windowsDebug $(scene)

runonly:
	cd data && ../bin/windowsDebug $(scene)

imgui:
	cd bin && \
	$(CC) -m64 -std=c++11 -O3 -g0 -Wall -c -fPIC -I../lib/$(IMGUI)/imgui.h ../lib/$(IMGUI)/imgui.cpp ../lib/$(IMGUI)/imgui_draw.cpp ../lib/$(IMGUI)/imgui_widgets.cpp ../lib/$(IMGUI)/imgui_demo.cpp && \
	ar rvs libimgui_linux.a imgui.o imgui_draw.o imgui_widgets.o && \
	rm imgui.o imgui_draw.o imgui_widgets.o imgui_demo.o

endif


#
# Linux
#
ifeq ($(detected_OS),Linux)
IMGUI := imgui-1.75
CC := clang++
CFLAGS := -std=c++17 -Wall -fmessage-length=0 -ferror-limit=5 -fno-exceptions -Wno-unused-function -Wno-unused-variable -Wno-unused-private-field -Wno-c++11-narrowing -Wno-unknown-warning-option
CINC := -I./src -I./lib -I./lib/$(IMGUI) -I/usr/local/include
CMAIN := src/main.cpp

# cmd line arguments for the application
scene="none"

build: $(CMAIN)
	$(CC) $(CFLAGS) -g -ggdb -O0 $(CINC) $(CMAIN) bin/libimgui_linux.a -ldl -lGL -pthread -lX11 -lGLEW -lglfw -o bin/linuxDebug

run: build
	cd data && ../bin/linuxDebug $(scene)

runonly:
	cd data && ../bin/linuxDebug $(scene)

imgui:
	cd bin && \
	$(CC) -std=c++11 -O3 -g0 -Wall -c -fPIC -I../lib/$(IMGUI)/imgui.h ../lib/$(IMGUI)/imgui.cpp ../lib/$(IMGUI)/imgui_draw.cpp ../lib/$(IMGUI)/imgui_widgets.cpp ../lib/$(IMGUI)/imgui_demo.cpp && \
	ar rvs libimgui_linux.a imgui.o imgui_draw.o imgui_widgets.o && \
	rm imgui.o imgui_draw.o imgui_widgets.o imgui_demo.o

endif