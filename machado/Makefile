# Autor: Anderson Fraiha Machado

ifdef P_FREETYPE
FT_ARG = -DNO_FREETYPE
else
FT_ARG=`freetype-config --cflags` `freetype-config --libs`
endif

CXX=g++

CXXFLAGS= -O2
#CXXFLAGS= -g -O0
CXXFLAGS+= -std=c++0x
#CXXFLAGS+= -Wall
CXXFLAGS+= -Wno-deprecated

LIBS = -Lpngwriter -L../build/bin -Ipngwriter -I../iplplus/iplplus/include -I../build/iplplus/include -lz -lpngwriter -lpng12 -liplplus
LIBS += -llog4cplus -lopencv_core -lopencv_imgproc
LIBS += $(FT_ARG)

all: pngw mmc

mmc: main.cpp
	@ echo ========================= visao ==============================
	$(CXX) main.cpp -o mmach $(CXXFLAGS) $(LIBS)
  
pngw: 
	@ echo ========================= pngwriter=============================
	@ cd pngwriter;	make 

clean:
	@ rm -fv *~ *.o mmach

clean_all: clean
	@ rm -fv pngwriter/pngwriter.o pngwriter/libpngwriter.a 
