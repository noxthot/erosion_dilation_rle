############ PARTIAL MAKEFILE FOR PNGWRITER ######################################
#
#       Website: Main:             http://pngwriter.sourceforge.net/
#       Author:                    Paul Blackburn
#       Email:                     individual61@users.sourceforge.net
#       Version:                   0.5.3   (24 / I / 2005)
#       License:                   GNU General Public License
#                                   2002, 2003, 2004, 2005 Paul Blackburn
#
##################################################################################


include ../make.include.linux

OBJECTS=pngwriter.o
all: libpngwriter.a

libpngwriter.a: $(OBJECTS)
	ar rv $@ $^
	ranlib $@

pngwriter.o: pngwriter.cc pngwriter.h
	$(CXX) $(CXXFLAGS) $(INC) -c -o pngwriter.o pngwriter.cc

clean	:    
	rm -f $(OBJECTS) libpngwriter.a pngtest.cc~ pngwriter.cc~
	rm -f pngwriter.h~ Makefile~
	rm -f .DS_Store


	
