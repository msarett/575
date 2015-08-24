/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRect.h"

static void draw_blocks(GCanvas* canvas) {
    const struct {
        GRect   fRect;
        GColor  fColor;
    } rec[] = {
        { GRect::MakeXYWH( 0,  0, 50, 50), GColor::MakeARGB(1, 1, 0, 0) },
        { GRect::MakeXYWH(50,  0, 50, 50), GColor::MakeARGB(1, 0, 1, 0) },
        { GRect::MakeXYWH( 0, 50, 50, 50), GColor::MakeARGB(1, 0, 0, 1) },
        { GRect::MakeXYWH(50, 50, 50, 50), GColor::MakeARGB(1, 0, 0, 0) },
        { GRect::MakeXYWH(25, 25, 50, 50), GColor::MakeARGB(0.75, 1, 1, 1) },
    };

    for (int i = 0; i < GARRAY_COUNT(rec); ++i) {
        canvas->fillRect(rec[i].fRect, rec[i].fColor);
    }
}

static void draw_bitmaps(GCanvas* canvas) {
    GBitmap tex;
    tex.readFromFile("spock_png");

    canvas->fillRectBitmap(GRect::MakeWH(tex.width(), tex.height()), tex);
    canvas->fillRectBitmap(GRect::MakeLTRB(10, 140, 138, 500), tex);
    canvas->fillRectBitmap(GRect::MakeLTRB(140, 10, 500, 138), tex);
    canvas->fillRectBitmap(GRect::MakeXYWH(256, 256, 512, 512), tex);
}

const CS575DrawRec gDrawRecs[] = {
    { draw_blocks,  100, 100, "blocks"    },
    { draw_bitmaps, 512, 512, "bitmaps"   },
    { NULL, 0, 0, NULL },
};

