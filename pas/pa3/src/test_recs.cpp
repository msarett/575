/**
 *  Copyright 2015 Mike Reed
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRect.h"
#include "tests.h"

static void setup_bitmap(GBitmap* bitmap, int w, int h) {
    bitmap->fWidth = w;
    bitmap->fHeight = h;
    bitmap->fRowBytes = w * sizeof(GPixel);
    bitmap->fPixels = (GPixel*)calloc(bitmap->rowBytes(), bitmap->height());
}

static void clear(const GBitmap& bitmap) {
    memset(bitmap.pixels(), 0, bitmap.rowBytes() * bitmap.height());
}

static GPixel get_px(const GBitmap& bitmap, int x, int y) {
    GASSERT(x >= 0 && x < bitmap.width());
    GASSERT(y >= 0 && y < bitmap.height());
    return ((const GPixel*)((const char*)bitmap.pixels() + y * bitmap.rowBytes()))[x];
}

static void test_bad_input(GTestStats* stats) {
    GBitmap bitmap;

    bitmap.fWidth = -1;
    bitmap.fHeight = 1;
    stats->expectNULL(GCanvas::Create(bitmap));
    
    bitmap.fWidth = 5;
    bitmap.fHeight = -5;
    stats->expectNULL(GCanvas::Create(bitmap));

    bitmap.fWidth = bitmap.fHeight = 10;
    bitmap.fRowBytes = (bitmap.fWidth - 1) * sizeof(GPixel);
    stats->expectNULL(GCanvas::Create(bitmap));
    
    bitmap.fWidth = bitmap.fHeight = 10;
    bitmap.fRowBytes = (bitmap.fWidth + 7) * sizeof(GPixel);
    bitmap.fPixels = (GPixel*)malloc(bitmap.rowBytes() * bitmap.height());
    GCanvas* canvas = GCanvas::Create(bitmap);
    stats->expectPtr(canvas);
    delete canvas;
}

static bool check9(const GBitmap& bitmap, const GPixel expected[9]) {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (get_px(bitmap, x, y) != expected[y * 3 + x]) {
                return false;
            }
        }
    }
    return true;
}

static void test_rect_colors(GTestStats* stats) {
    const GPixel pred = GPixel_PackARGB(0xFF, 0xFF, 0, 0);
    const GColor cred = GColor::MakeARGB(1, 1, 0, 0);

    GBitmap bitmap;
    setup_bitmap(&bitmap, 3, 3);
    GCanvas* canvas = GCanvas::Create(bitmap);

    GPixel nine[9] = { 0, 0, 0, 0, pred, 0, 0, 0, 0 };
    canvas->fillRect(GRect::MakeLTRB(1, 1, 2, 2), cred);
    stats->expectTrue(check9(bitmap, nine));

    nine[4] = 0;
    clear(bitmap);
    // don't expect these to draw anything
    const GRect rects[] = {
        GRect::MakeLTRB(-10, 0, 0.25f, 10),
        GRect::MakeLTRB(0, -10, 10, 0.25f),
        GRect::MakeLTRB(2.51f, 0, 10, 10),
        GRect::MakeLTRB(0, 2.51, 10, 10),

        GRect::MakeLTRB(1, 1, 1, 1),
        GRect::MakeLTRB(1.51f, 0, 2.49f, 3),
    };
    for (int i = 0; i < GARRAY_COUNT(rects); ++i) {
        canvas->fillRect(rects[i], cred);
        stats->expectTrue(check9(bitmap, nine));
    }

    // vertical stripe down center
    nine[1] = nine[4] = nine[7] = pred;
    canvas->fillRect(GRect::MakeLTRB(0.6f, -3, 2.3f, 2.6f), cred);
    stats->expectTrue(check9(bitmap, nine));

    clear(bitmap);
    memset(nine, 0, sizeof(nine));
    // don't expect anything to draw
    const GColor colors[] = {
        GColor::MakeARGB(0, 1, 0, 0),
        GColor::MakeARGB(-1, 1, 0, 0),
        GColor::MakeARGB(0.00001f, 1, 0, 0),
    };
    for (int i = 0; i < GARRAY_COUNT(colors); ++i) {
        canvas->fillRect(GRect::MakeWH(3, 3), colors[i]);
        stats->expectTrue(check9(bitmap, nine));
    }
}

const GTestRec gTestRecs[] = {
    { test_bad_input, "bad_input" },
    { test_rect_colors, "rect_colors" },
    { NULL, NULL },
};

bool gCrashOnGTestStatsFailure;
