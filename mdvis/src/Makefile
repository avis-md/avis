OSFLAG :=
CFLAGS2 :=
LDFLAGS2 :=
GLLIB :=
LIBS2 :=
PYLIB :=
WEAK_LIB :=
NUMPY :=
USR := /usr
UNAME_S := $(shell uname -s)

RRAYPRE := ../../radeonrays
RRAY := -DUSE_OPENCL -DRR_EMBED_KERNELS -I$(RRAYPRE)/Calc/inc -I$(RRAYPRE)/CLW/ -I$(RRAYPRE)/RadeonRays/include
RRAYL := -L$(RRAYPRE)/build/Calc -L$(RRAYPRE)/build/CLW -lCalc -lCLW -lRadeonRays
RRAYBIN :=

AVIPRE := ../../libgwavi
AVI := -I$(AVIPRE)/inc/
AVIL := $(AVIPRE)/obj/libgwavi.a

BINPRE := /usr/lib/x86_64-linux-gnu
BINPRE2 := /lib/x86_64-linux-gnu

ifeq ($(UNAME_S),Linux)
	OSFLAG += -D PLATFORM_LNX
	CFLAGS2 += -Wno-ignored-attributes
	LDFLAGS2 += -Wl,-rpath='$$ORIGIN' -static-libstdc++
	GLLIB += /usr/lib/x86_64-linux-gnu/libjpeg.a -lssh2 -lOpenCL `pkg-config --static --libs gl glew glfw3 freetype2`
	#PYLIB += $(shell python3-config --configdir)/libpython3.6m.a -lexpat
	PYLIB += -L../build -lpython3
	NUMPY += -I/usr/local/lib/python3.6/dist-packages/numpy/core/include
	RRAYBIN += libRadeonRays.so.2.0
endif
ifeq ($(UNAME_S),Darwin)
	OSFLAG += -D PLATFORM_OSX
	GLLIB += -framework OpenGL -framework OpenCL /usr/local/Cellar/glew/2.1.0/lib/libGLEW.a /usr/local/Cellar/jpeg/9c/lib/libjpeg.a /usr/local/Cellar/freetype/2.9.1/lib/libfreetype.a /usr/local/Cellar/libpng/1.6.34/lib/libpng.a -lbz2 -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS2 += /usr/local/Cellar/libssh2/1.8.0/lib/libssh2.a /usr/local/Cellar/openssl/1.0.2o_1/lib/libcrypto.a
	WEAK_LIB += -weak_library
	PYLIB += $(shell python3-config --configdir)/libpython3.6m.a
	NUMPY +=  -I/usr/local/lib/python3.6/site-packages/numpy/core/include
	RRAYBIN += libRadeonRays.dylib
endif
LIBS2 += -L../build $(RRAYL) $(AVIL)

C = gcc
CXX = g++
CXXFLAGS = -O0 -g -std=c++11 -fvisibility=hidden -I../src/ -I/usr/include/freetype2/ -I/usr/local/include/freetype2/ $(NUMPY) -I$(shell python3-config --prefix)/include/python3.6m $(RRAY) $(AVI) $(OSFLAG) $(CFLAGS2)
LDFLAGS = -L/usr/lib -L/usr/local/lib $(LDFLAGS2)
LIBS = $(WEAK_LIB) $(PYLIB) -lpthread -lm $(GLLIB) -lz $(LIBS2)

OBJS = obj/hdr.o obj/lodepng.o obj/ChokoLait.o obj/Engine.o obj/main.o obj/RenderEngine.o
OBJS += obj/asset/mesh.o obj/asset/tetrahedron.o obj/asset/rendertexture.o obj/asset/texture.o obj/asset/texture3d.o
OBJS += obj/core/bbox.o obj/core/debug.o obj/core/display.o obj/core/font.o obj/core/input.o obj/core/io.o obj/core/mvp.o obj/core/time.o obj/core/ui.o obj/core/ui3.o
OBJS += obj/md/Protein.o obj/md/Gromacs.o obj/md/parmenu.o obj/md/particles.o obj/md/parloader.o obj/md/pdb.o obj/md/lammps.o obj/md/CDV.o obj/md/dlpoly.o #obj/md/mdvbin.o obj/md/XYZ.o
OBJS += obj/web/anbrowse.o obj/web/anconv.o obj/web/annode.o obj/web/anops.o obj/web/anscript.o obj/web/anweb.o obj/web/creader.o obj/web/pyreader.o obj/web/freader.o obj/web/errorview.o
OBJS += obj/web/nodes/node_inputs.o obj/web/nodes/node_info.o obj/web/nodes/node_plot.o obj/web/nodes/node_volume.o obj/web/nodes/node_gromacs.o obj/web/nodes/node_addbond.o obj/web/nodes/node_camera.o obj/web/nodes/node_setparam.o obj/web/nodes/node_showrange.o obj/web/nodes/node_tracetrj.o
OBJS += obj/scene/object.o obj/scene/scene.o obj/scene/sceneobject.o obj/scene/transform.o obj/scene/comp/camera.o obj/scene/comp/component.o
OBJS += obj/utils/avi.o obj/utils/ssh.o obj/utils/color.o obj/utils/effects.o obj/utils/fft.o obj/utils/net.o obj/utils/plot.o obj/utils/random.o obj/utils/rect.o obj/utils/spline.o obj/utils/stream.o obj/utils/strext.o obj/utils/xml.o obj/utils/dialog.o obj/utils/dylib.o obj/utils/runcmd.o obj/utils/tinyfiledialogs.o obj/utils/shader.o obj/utils/glmext.o
OBJS += obj/ui/popups.o obj/ui/icons.o obj/ui/help.o obj/ui/ui_ext.o obj/ui/localizer.o
OBJS += obj/vis/pargraphics.o obj/vis/system.o obj/vis/shadows.o obj/vis/renderer.o obj/vis/cubemarcher.o obj/vis/selection.o
OBJS += obj/live/livesyncer.o live/lj256.o
OBJS += obj/ocl/raytracer.o
OBJS += obj/xdrfile/xdrfile.o obj/xdrfile/xdrfile_trr.o obj/xdrfile/xdrfile_xtc.o
OBJS += obj/tinyobj.o

HEADERS = hdr.h lodepng.h AssetObjects.h ChokoLait.h Engine.h SceneObjects.h
HEADERS += asset/mesh.h asset/tetrahedron.h asset/rendertexture.h asset/texture.h asset/texture3d.h
HEADERS += core/bbox.h core/debug.h core/display.h core/font.h core/input.h core/io.h core/math.h core/mvp.h core/time.h core/ui.h core/ui3.h
HEADERS += md/Protein.h md/Gromacs.h md/CDV.h md/parmenu.h md/particles.h md/mdvbin.h md/parloader.h md/XYZ.h md/pdb.h md/lammps.h md/dlpoly.h md/CDV.h
HEADERS += web/anbrowse.h web/anconv.h web/annode.h web/anscript.h web/anops.h web/anweb.h web/creader.h web/pyreader.h web/freader.h web/errorview.h
HEADERS += web/nodes/node_inputs.h web/nodes/node_info.h web/nodes/node_plot.h web/nodes/node_volume.h web/nodes/node_gromacs.h web/nodes/node_addbond.h web/nodes/node_camera.h web/nodes/node_setparam.h web/nodes/node_showrange.h web/nodes/node_tracetrj.h
HEADERS += scene/object.h scene/scene.h scene/sceneobject.h scene/transform.h scene/comp/camera.h scene/comp/component.h scene/comp/light.h
HEADERS += ui/popups.h ui/icons.h ui/help.h ui/ui_ext.h ui/localizer.h
HEADERS += vis/pargraphics.h vis/system.h vis/shadows.h vis/renderer.h vis/cubemarcher.h vis/selection.h
HEADERS += res/resdata.h res/shddata.h
HEADERS += utils/avi.h utils/color.h utils/effects.h utils/fft.h utils/ifaddrs.h utils/net.h utils/plot.h utils/ptrext.h utils/random.h utils/rawvector.h utils/rect.h utils/spline.h utils/stream.h utils/strext.h utils/xml.h utils/dialog.h utils/dylib.h utils/runcmd.h utils/ssh.h utils/tinyfiledialogs.h utils/shader.h utils/glmext.h
HEADERS += live/livesyncer.h live/lj256.h
HEADERS += ocl/raytracer.h ocl/kernel.h
HEADERSX = xdrfile/xdrfile.h xdrfile/xdrfile_trr.h xdrfile/xdrfile_xtc.h

PCH = precompile.h
PCH_OUT = precompile.h.gch

../build/avis: $(OBJS)
ifeq ($(UNAME_S),Darwin)
	cp $(RRAYPRE)/build/bin/libRadeonRays.dylib ../build/libRadeonRays.dylib
endif
ifeq ($(UNAME_S),Linux)
	cp $(BINPRE)/libpython3.6m.so.1.0 ../build/libpython3.6m.so.1.0
	cp $(BINPRE)/libssh2.so.1 ../build/libssh2.so.1
	cp $(BINPRE)/libOpenCL.so.1 ../build/libOpenCL.so.1
	cp $(BINPRE)/libGLEW.so.2.0 ../build/libGLEW.so.2.0
	cp $(BINPRE)/libglfw.so.3 ../build/libglfw.so.3
	cp $(BINPRE2)/libgcrypt.so.20 ../build/libgcrypt.so.20
	cp $(RRAYPRE)/build/bin/libRadeonRays.so.2.0 ../build/libRadeonRays.so.2.0
endif
	ar rvs $(AVIPRE)/obj/libgwavi.a $(AVIPRE)/obj/*.o
	@echo "Linking..."
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
ifeq ($(UNAME_S),Darwin)
	install_name_tool -add_rpath "@executable_path" ../build/avis
	install_name_tool -change $(shell python3-config --exec)/Python /tmpp/Python ../build/avis
	cp ../build/avis ../build/avis_clean
	cd ../build && rm .pyregfin && ./regpython.sh
endif

$(PCH_OUT): $(PCH) $(HEADERS)
	@echo "Creating precompiled header"
	@$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

obj/%.o: %.cpp $(PCH_OUT)
	@echo "Compiling" $<
	@$(CXX) $(CXXFLAGS) -include precompile.h -c $< -o $@

obj/xdrfile/%.o: $(HEADERSX)
obj/xdrfile/%.o: xdrfile/%.c
	@echo "Compiling" $<
	@$(C) -g -c $< -o $@

obj/utils/tinyfiledialogs.o: utils/tinyfiledialogs.c
	@echo "Compiling" $<
	@$(C) -g -c $< -o $@

obj/vis/system.o: vis/system.cpp $(PCH_OUT)
	echo "\""$(shell git rev-parse --short HEAD)"\"" > ../../githash.h
	@echo "Compiling" $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

obj/tinyobj.o : tmp/tiny_obj_loader.cpp
	@echo "Compiling" $<
	@$(CXX) -g -c $< -o $@

clean:
	find ./obj -name '*.o' -exec rm {} \;
	rm -f $(PCH_OUT)
	rm -f ../build/avis
clean-all:
	$(MAKE) clean
	cd web && $(MAKE) clean

srv:
	cd web && $(MAKE)
	
all:
	$(MAKE) && $(MAKE) srv
