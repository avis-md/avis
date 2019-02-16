PYTHON_VERSION = 3.7
PYTHON = python$(PYTHON_VERSION)
OSFLAG :=
CFLAGS2 :=
CXXFLAGS2 := `pkg-config --cflags freetype2` -Wno-trigraphs
LDFLAGS2 :=
GLLIB := `pkg-config --libs glfw3`
LIBS2 := `pkg-config --libs freetype2 libssh2 libjpeg`
PYLIB :=
WEAK_LIB :=
NUMPY := -I$(shell $(PYTHON) -m site --user-site)/numpy/core/include
USR := /usr
UNAME_S := $(shell uname -s)

RRAYPRE := ../../radeonrays
RRAY := -DUSE_OPENCL -DRR_EMBED_KERNELS -I$(RRAYPRE)/Calc/inc -I$(RRAYPRE)/CLW/ -I$(RRAYPRE)/RadeonRays/include
RRAYL := -L$(RRAYPRE)/build/Calc -L$(RRAYPRE)/build/CLW -lCalc -lCLW -lRadeonRays
RRAYBIN :=

AVIPRE := ../../libgwavi
AVI := -I$(AVIPRE)/inc/
AVIL := obj/libgwavi.a

ifeq ($(UNAME_S),Linux)
	OSFLAG += -D PLATFORM_LNX
	CFLAGS2 += -Wno-ignored-attributes
	CXXFLAGS2 += -fopenmp
	LDFLAGS2 += -Wl,-rpath='$$ORIGIN' -static-libstdc++ -lgomp
	GLLIB += -lOpenCL `pkg-config --libs gl glew`
	PYLIB += -L$(shell $(PYTHON)-config --configdir) $(shell $(PYTHON)-config --libs)
	RRAYBIN += libRadeonRays.so.2.0
endif
ifeq ($(UNAME_S),Darwin)
	OSFLAG += -D PLATFORM_OSX
	CXXFLAGS2 += -D_GLIBCXX_DEBUG -fno-pie
	GLLIB += -framework OpenGL -framework OpenCL -framework Cocoa -framework IOKit -framework CoreVideo
	WEAK_LIB += -weak_library
	PYLIB += $(shell $(PYTHON)-config --configdir)/lib$(PYTHON)m.a
	RRAYBIN += libRadeonRays.dylib
endif
LIBS2 += -L../build $(RRAYL) $(AVIL)

C = gcc
CXX = g++
CXXFLAGS = -O0 -g -Wall -std=c++11 -fvisibility=hidden -I../src/ $(CXXFLAGS2) $(NUMPY) -I$(shell $(PYTHON)-config --prefix)/include/$(PYTHON)m $(RRAY) $(AVI) $(OSFLAG) $(CFLAGS2)
LDFLAGS = -L/usr/lib -L/usr/local/lib $(LDFLAGS2)
LIBS = $(WEAK_LIB) $(PYLIB) -lpthread -lm $(GLLIB) $(LIBS2) -lz

HEADERS = hdr.h lodepng.h AssetObjects.h ChokoLait.h Engine.h SceneObjects.h unloader.h
HEADERS += asset/mesh.h asset/tetrahedron.h asset/texture.h asset/texture3d.h
HEADERS += core/bbox.h core/debug.h core/display.h core/font.h core/input.h core/io.h core/math.h core/mvp.h core/time.h core/ui.h core/ui3.h
HEADERS += md/Protein.h md/GenericSSV.h md/Gromacs.h md/CDV.h md/parmenu.h md/particles.h md/parloader.h md/XYZ.h md/pdb.h md/lammps.h md/dlpoly.h md/CDV.h
HEADERS += web/anbrowse.h web/anconv.h web/annode.h web/annode_internal.h web/anscript.h web/anscript_vars.h web/anops.h web/anweb.h web/errorview.h web/arrayview.h
HEADERS += web/dmscript.h
HEADERS += web/cc/cscript.h web/cc/cnode.h web/cc/creader.h
#HEADERS += web/pyreader.h web/freader.h
#HEADERS += web/nodes/node_tracetrj.h
HEADERS += web/nodes/get/node_inputs.h web/nodes/get/node_info.h web/nodes/get/node_getattribute.h
HEADERS += web/nodes/set/node_setradscl.h web/nodes/set/node_showrange.h web/nodes/set/node_setattribute.h #web/nodes/set/node_setbboxcenter.h web/nodes/set/node_setcamera.h
#HEADERS += web/nodes/mod/node_addbond.h web/nodes/mod/node_addsurface.h
HEADERS += web/nodes/conv/node_adjlist.h #web/nodes/conv/node_remap.h web/nodes/conv/node_vector.h
#HEADERS += web/nodes/ctrl/node_dowhile.h web/nodes/ctrl/node_dofor.h
HEADERS += web/nodes/misc/node_plot.h web/nodes/misc/node_accum.h
HEADERS += scene/object.h scene/scene.h scene/sceneobject.h scene/transform.h scene/comp/camera.h scene/comp/component.h scene/comp/light.h
HEADERS += ui/popups.h ui/icons.h ui/help.h ui/ui_ext.h ui/localizer.h ui/browse.h ui/browsetarget.h
HEADERS += vis/pargraphics.h vis/system.h vis/shadows.h vis/renderer.h vis/cubemarcher.h vis/selection.h vis/changelog.h vis/preferences.h
HEADERS += utils/avi.h utils/color.h utils/effects.h utils/fft.h utils/ifaddrs.h utils/net.h utils/plot.h utils/ptrext.h utils/random.h utils/rect.h utils/spline.h utils/stream.h utils/strext.h utils/xml.h utils/dialog.h utils/dylib.h utils/runcmd.h utils/ssh.h utils/tinyfiledialogs.h utils/shader.h utils/glmext.h utils/refcnt.h utils/uniquecaller.h
#HEADERS += live/livesyncer.h live/lj256.h
HEADERS += ocl/raytracer.h
HEADERS += res/resdata.h res/shddata.h ocl/kernel.h
HEADERSX = xdrfile/xdrfile.h xdrfile/xdrfile_trr.h xdrfile/xdrfile_xtc.h

OBJS = obj/hdr.o obj/lodepng.o obj/ChokoLait.o obj/Engine.o obj/main.o obj/RenderEngine.o obj/unloader.o
OBJS += obj/asset/mesh.o obj/asset/tetrahedron.o obj/asset/texture.o obj/asset/texture3d.o
OBJS += obj/core/bbox.o obj/core/debug.o obj/core/display.o obj/core/font.o obj/core/input.o obj/core/io.o obj/core/math.o obj/core/mvp.o obj/core/time.o obj/core/ui.o obj/core/ui3.o
OBJS += obj/md/Protein.o obj/md/GenericSSV.o obj/md/Gromacs.o obj/md/parmenu.o obj/md/particles.o obj/md/particles_animdata.o obj/md/particles_attrdata.o obj/md/parloader.o obj/md/pdb.o obj/md/lammps.o obj/md/CDV.o obj/md/dlpoly.o #obj/md/XYZ.o
OBJS += obj/web/anbrowse.o obj/web/anconv.o obj/web/annode.o obj/web/annode_internal.o obj/web/anops.o obj/web/anscript.o obj/web/anweb.o obj/web/errorview.o obj/web/arrayview.o
OBJS += obj/web/dmscript.o
OBJS += obj/web/cc/creader.o obj/web/cc/cnode.o obj/web/cc/cscript.o
#OBJS += obj/web/pyreader.o obj/web/pynode.o obj/web/pyscript.o
#OBJS += obj/web/freader.o obj/web/fnode.o obj/web/fscript.o
#OBJS += obj/web/nodes/node_tracetrj.o
OBJS += obj/web/nodes/get/node_inputs.o obj/web/nodes/get/node_info.o obj/web/nodes/get/node_getattribute.o
OBJS += obj/web/nodes/set/node_setradscl.o obj/web/nodes/set/node_showrange.o obj/web/nodes/set/node_setattribute.o #obj/web/nodes/set/node_setbboxcenter.o obj/web/nodes/set/node_setcamera.o
#OBJS += obj/web/nodes/mod/node_addbond.o obj/web/nodes/mod/node_addsurface.o
OBJS += obj/web/nodes/conv/node_adjlist.o #obj/web/nodes/conv/node_remap.o obj/web/nodes/conv/node_vector.o
#OBJS += obj/web/nodes/ctrl/node_dowhile.o obj/web/nodes/ctrl/node_dofor.o
OBJS += obj/web/nodes/misc/node_plot.o obj/web/nodes/misc/node_accum.o
OBJS += obj/scene/object.o obj/scene/scene.o obj/scene/sceneobject.o obj/scene/transform.o obj/scene/comp/camera.o obj/scene/comp/component.o
OBJS += obj/utils/avi.o obj/utils/ssh.o obj/utils/color.o obj/utils/effects.o obj/utils/fft.o obj/utils/net.o obj/utils/plot.o obj/utils/random.o obj/utils/rect.o obj/utils/spline.o obj/utils/stream.o obj/utils/strext.o obj/utils/xml.o obj/utils/dialog.o obj/utils/dylib.o obj/utils/runcmd.o obj/utils/tinyfiledialogs.o obj/utils/shader.o obj/utils/glmext.o obj/utils/uniquecaller.o
OBJS += obj/ui/popups.o obj/ui/icons.o obj/ui/help.o obj/ui/ui_ext.o obj/ui/localizer.o obj/ui/browse.o obj/ui/localbrowsetarget.o obj/ui/remotebrowsetarget.o
OBJS += obj/vis/pargraphics.o obj/vis/system.o obj/vis/shadows.o obj/vis/renderer.o obj/vis/cubemarcher.o obj/vis/selection.o obj/vis/changelog.o obj/vis/preferences.o
OBJS += obj/live/livesyncer.o live/lj256.o
OBJS += obj/ocl/raytracer.o
OBJSX = obj/xdrfile/xdrfile.o obj/xdrfile/xdrfile_trr.o obj/xdrfile/xdrfile_xtc.o
OBJS += obj/tinyobj.o

PCH = precompile.h
PCH_OUT = precompile.h.gch

../build/avis: $(OBJS) $(OBJSX)
	@echo "Copying libraries..."
ifeq ($(UNAME_S),Darwin)
	@cp $(RRAYPRE)/build/bin/libRadeonRays.dylib ../build/libRadeonRays.dylib
endif
ifeq ($(UNAME_S),Linux)
	@cp $(RRAYPRE)/build/bin/libRadeonRays.so.2.0 ../build/libRadeonRays.so.2.0
endif
	@ar rvs obj/libgwavi.a $(AVIPRE)/obj/*.o > /dev/null
	@echo "Linking..."
	@$(CXX) -o $@ $(OBJS) $(OBJSX) $(LDFLAGS) $(LIBS)
ifeq ($(UNAME_S),Darwin)
	install_name_tool -add_rpath "@executable_path" ../build/avis
	install_name_tool -change $(shell $(PYTHON)-config --exec)/Python /tmpp/Python ../build/avis
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
	@$(CXX) $(CXXFLAGS) -include precompile.h -c $< -o $@

obj/tinyobj.o : tmp/tiny_obj_loader.cpp
	@echo "Compiling" $<
	@$(CXX) $(CXXFLAGS) -g -c $< -o $@

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
