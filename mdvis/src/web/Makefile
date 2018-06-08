OSFLAG :=
LDFLAGS2 :=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	OSFLAG += -D PLATFORM_LNX
	LDFLAGS2 += -Wl,-rpath='$$ORIGIN' -static-libstdc++
endif
ifeq ($(UNAME_S),Darwin)
	OSFLAG += -D PLATFORM_OSX
endif

CXX = g++
CXXFLAGS = -O0 -g -std=c++11 -I./inc/ -I/usr/include/python2.7/ -I/usr/local/include/python2.7/ -D CHOKO_LAIT -D IS_ANSERVER $(OSFLAG) -ferror-limit=1
LDFLAGS = -L/usr/lib -L/usr/local/lib $(LDFLAGS2)
LIBS = -L/usr/lib/python2.7 -L/usr/local/lib/python2.7 -lpthread -lm -lpython2.7

OBJS = obj/main_ansrv.o obj/inc/Engine.o obj/anbrowse.o obj/anconv.o obj/annode.o obj/anscript.o obj/anweb.o obj/creader.o obj/pyreader.o obj/nodes/node_inputs.o obj/nodes/node_plot.o obj/nodes/node_volume.o
#SRC = $(wildcard *.cpp) $(wildcard */*.cpp)
#OBJS = $(patsubst ./%%/%.cpp, obj/%%/%.o, $(SRC))

HEADERS = inc/Engine.h anbrowse.h anconv.h annode.h anscript.h anweb.h creader.h pyreader.h nodes/node_inputs.h nodes/node_plot.h nodes/node_volume.h

../../build/mdvis_ansrv: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

obj/%.o: $(HEADERS)
obj/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	find ./obj -name '*.o' -exec rm {} \;
	rm -f ../../build/mdvis_ansrv
