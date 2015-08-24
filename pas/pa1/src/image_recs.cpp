/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRect.h"
#include <string>

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

const CS575DrawRec gDrawRecs[] = {
    { draw_blocks,  100, 100, "blocks"    },
    { NULL,  0, 0, NULL    },
};
