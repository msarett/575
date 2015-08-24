CC = g++ -g

CC_DEBUG = @$(CC)
CC_RELEASE = @$(CC) -O3 -DNDEBUG

G_SRC = *.cpp ../common/GBitmap.cpp apps/GTime.cpp

# need libpng to build
#
G_INC = -Iinclude -I../common -I/opt/local/include -L/opt/local/lib

all: image poster draw preview

image : $(G_SRC) apps/image.cpp
	$(CC_RELEASE) $(G_INC) $(G_SRC) apps/image.cpp -lpng -o image

# needs xwindows to build
#
X_INC = -I/opt/X11/include -L/opt/X11/lib
# -I/usr/X11R6/include -I/usr/X11R6/include/X11 -L/usr/X11R6/lib -L/usr/X11R6/lib/X11

DRAW_SRC = apps/draw.cpp apps/GWindow.cpp
draw: $(DRAW_SRC) $(G_SRC)
	$(CC_RELEASE) $(X_INC) $(G_INC) $(G_SRC) $(DRAW_SRC) -lpng -lX11 -o draw

POSTER_SRC = apps/poster.cpp apps/GWindow.cpp
poster: $(POSTER_SRC) $(G_SRC)
	$(CC_RELEASE) $(X_INC) $(G_INC) $(G_SRC) $(POSTER_SRC) -lpng -lX11 -o poster

PREVIEW_SRC = apps/preview.cpp apps/GWindow.cpp
preview: $(PREVIEW_SRC) $(G_SRC)
	$(CC_RELEASE) $(X_INC) $(G_INC) $(G_SRC) $(PREVIEW_SRC) -lpng -lX11 -o preview


clean:
	@rm -rf image poster draw preview *.png *.dSYM

