OBJS = newzpr.o dem.o misc.o
CC = g++  
DEBUG = #-g
CFLAGS =  -c -w $(DEBUG) -O5	misc.h	misc.cpp
LFLAGS =  $(DEBUG) -lm -lGL -lGLU -lglut #-framework Cocoa -framework GLUT -framework OpenGL
zpr: $(OBJS)
	$(CC)   $(OBJS) -o  dem.exe  $(LFLAGS)
newzpr.o: newzpr.cpp  newzpr.h
	$(CC) $(CFLAGS) newzpr.cpp
dem.o:	dem.cpp	newzpr.h  newzpr.cpp
	$(CC)	    $(CFLAGS)	dem.cpp
