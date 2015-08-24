CC = g++ -g

CC_DEBUG = @$(CC)
CC_RELEASE = @$(CC) -O3 -DNDEBUG

G_SRC = src/*.cpp *.cpp

# need libpng to build
#
G_INC = -Iinclude -I/opt/local/include -L/opt/local/lib

all: image

image : $(G_SRC)
	$(CC_DEBUG) $(G_INC) $(G_SRC) -lpng -o image

# needs xwindows to build
#
X_INC = -I/opt/X11/include -L/opt/X11/lib -I/usr/X11R6/include -I/usr/X11R6/include/X11 -L/usr/X11R6/lib -L/usr/X11R6/lib/X11

clean:
	@rm -rf image *.png *.dSYM

