OSFLAG :=
LDFLAGS2 :=
GLLIB :=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	OSFLAG += -D PLATFORM_LNX
	LDFLAGS2 += -Wl,-rpath='$$ORIGIN' -static-libstdc++
	GLLIB += -lGL -lglfw3 -lGLEW -ljpeg -lfreetype
endif
ifeq ($(UNAME_S),Darwin)
	OSFLAG += -D PLATFORM_OSX
	GLLIB += -framework OpenGL /usr/local/Cellar/glew/2.1.0/lib/libGLEW.a /usr/local/Cellar/jpeg/9c/lib/libjpeg.a /usr/local/Cellar/freetype/2.9.1/lib/libfreetype.a /usr/local/Cellar/libpng/1.6.34/lib/libpng.a -lbz2 -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
endif

CXX = g++
CXXFLAGS = -O0 -g -std=c++11 -I../src/ -I/usr/include/freetype2/ -I/usr/local/include/freetype2/ -I/usr/include/python2.7/ -I/usr/local/include/python2.7/ -I/usr/local/include/xdrfile/ -D CHOKO_LAIT $(OSFLAG) -ferror-limit=1
LDFLAGS = -L/usr/lib -L/usr/local/lib $(LDFLAGS2)
LIBS = -L/usr/lib/python2.7 -L/usr/local/lib/python2.7 -lpthread -lm $(GLLIB) -lxdrfile -lz -lpython2.7 -lssh2

OBJS = obj/md/Protein.o obj/hdr.o obj/lodepng.o obj/AudioEngine.o obj/ChokoLait.o obj/Editor.o obj/EditorShortcuts.o obj/Engine.o obj/main.o obj/RenderEngine.o obj/SceneObjects.o obj/Types.o obj/asset/animation.o obj/asset/animclip.o obj/asset/assetmanager.o obj/asset/assetobject.o obj/asset/audioclip.o obj/asset/cameffect.o obj/asset/compute.o obj/asset/cubemap.o obj/asset/material.o obj/asset/mesh.o obj/asset/resources.o obj/asset/shader.o obj/asset/img/background.o obj/asset/img/rendertexture.o obj/asset/img/texture.o obj/asset/img/texture3d.o obj/asset/img/videotexture.o obj/core/audio.o obj/core/bbox.o obj/core/debug.o obj/core/display.o obj/core/font.o obj/core/input.o obj/core/io.o obj/core/mvp.o obj/core/time.o obj/core/ui.o obj/md/Gromacs.o obj/md/ParMenu.o obj/md/Particles.o obj/web/anbrowse.o obj/web/anconv.o obj/web/annode.o obj/web/anscript.o obj/web/anweb.o obj/web/creader.o obj/web/pyreader.o obj/web/nodes/node_inputs.o obj/web/nodes/node_plot.o obj/web/nodes/node_volume.o obj/scene/object.o obj/scene/scene.o obj/scene/sceneobject.o obj/scene/transform.o obj/scene/comp/animator.o obj/scene/comp/armature.o obj/scene/comp/audiolistener.o obj/scene/comp/audiosource.o obj/scene/comp/camera.o obj/scene/comp/component.o obj/scene/comp/light.o obj/scene/comp/meshfilter.o obj/scene/comp/meshrenderer.o obj/scene/comp/scenescript.o obj/scene/comp/skinnedmeshrenderer.o obj/ui/popups.o obj/ui/icons.o obj/utils/color.o obj/utils/effects.o obj/utils/fft.o obj/utils/net.o obj/utils/plot.o obj/utils/precedurals.o obj/utils/random.o obj/utils/rect.o obj/utils/serial.o obj/utils/solidify.o obj/utils/spline.o obj/utils/stream.o obj/utils/strext.o obj/utils/xml.o obj/utils/dialog.o obj/utils/dylib.o obj/utils/runcmd.o obj/vis/pargraphics.o obj/vis/system.o
OBJS += obj/utils/ssh.o
#SRC = $(wildcard *.cpp) $(wildcard */*.cpp)
#OBJS = $(patsubst ./%%/%.cpp, obj/%%/%.o, $(SRC))

HEADERS = web/anbrowse.h web/anconv.h hweb/annode.h web/anscript.h web/anweb.h web/creader.h web/pyreader.h web/nodes/node_inputs.h web/nodes/node_plot.h web/nodes/node_volume.h md/Protein.h hdr.h lodepng.h AssetObjects.h AudioEngine.h ChokoLait.h Defines.h Editor.h Engine.h SceneObjects.h SceneScriptResolver.h asset/animation.h asset/animclip.h asset/assetmanager.h asset/assetobject.h asset/audioclip.h asset/cameffect.h asset/compute.h asset/cubemap.h asset/material.h asset/mesh.h asset/resources.h asset/shader.h asset/img/background.h asset/img/rendertexture.h asset/img/texture.h asset/img/texture3d.h asset/img/videotexture.h core/audio.h core/bbox.h core/debug.h core/display.h core/font.h core/input.h core/io.h core/math.h core/mvp.h core/time.h core/ui.h md/Gromacs.h md/ParMenu.h md/Particles.h scene/object.h scene/scene.h scene/sceneobject.h scene/texturerenderer.h scene/transform.h scene/comp/animator.h scene/comp/armature.h scene/comp/audiolistener.h scene/comp/audiosource.h scene/comp/camera.h scene/comp/component.h scene/comp/light.h scene/comp/meshfilter.h scene/comp/meshrenderer.h scene/comp/scenescript.h scene/comp/skinnedmeshrenderer.h scene/comp/voxelrenderer.h ui/popups.h ui/icons.h utils/color.h utils/effects.h utils/fft.h utils/ifaddrs.h utils/net.h utils/plot.h utils/precedurals.h utils/ptrext.h utils/random.h utils/rawvector.h utils/rect.h utils/serial.h utils/solidify.h utils/spline.h utils/stream.h utils/strext.h utils/xml.h utils/dialog.o utils/dylib.h utils/runcmd.h utils/ssh.h vis/pargraphics.h vis/system.h

../build/mdvis: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

obj/%.o: $(HEADERS)
obj/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find ./obj -name '*.o' -exec rm {} \;
	rm -f ../build/mdvis
clean-all:
	$(MAKE) clean
	cd web && $(MAKE) clean

srv:
	cd web && $(MAKE)
	
all:
	$(MAKE) && $(MAKE) srv