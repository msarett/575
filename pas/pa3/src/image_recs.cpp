/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRect.h"
#include "GPoint.h"

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

static void draw_tri(GCanvas* canvas) {
    GPoint pts[] = {
        { 10, 10 },
        { 200, 50 },
        { 100, 200 },
    };
    canvas->fillConvexPolygon(pts, GARRAY_COUNT(pts), GColor::MakeARGB(1, 1, 0, 0));
}

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = 6.28318530718f / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
        angle += deltaAngle;
    }
}

static void dr_poly(GCanvas* canvas, float dx, float dy) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        for (int i = 0; i < count; ++i) {
            storage[i].fX += dx;
            storage[i].fY += dy;
        }
        GColor c = GColor::MakeARGB(0.8f,
                                    fabs(sin(count*7)),
                                    fabs(sin(count*11)),
                                    fabs(sin(count*17)));
        canvas->fillConvexPolygon(storage, count, c);
    }
}

static void draw_poly(GCanvas* canvas) {
    dr_poly(canvas, 0, 0);
}

static void draw_poly_center(GCanvas* canvas) {
    dr_poly(canvas, -128, -128);
}

const CS575DrawRec gDrawRecs[] = {
    { draw_blocks,      100, 100, "blocks"    },
    { draw_bitmaps,     512, 512, "bitmaps"   },
    { draw_tri,         512, 512, "tri"   },
    { draw_poly,        512, 512, "poly"   },
    { draw_poly_center, 256, 256, "poly_center"   },
    { NULL, 0, 0, NULL },
};

