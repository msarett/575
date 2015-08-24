/*
 *  Copyright 2015 <your name here>
 */

#include "GBitmap.h"
#include "GColor.h"
#include "GCanvas.h"
#include "GPixel.h"
#include "GRect.h"

extern void cs575_draw_rect(const GBitmap&, const GRect&, const GColor&);

///////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned GDiv255(unsigned prod) {
    return (prod + prod + 255) / (2*255);
}

static GPixel* get_addr(const GBitmap& bitmap, int x, int y) {
    GASSERT(x >= 0 && x < bitmap.width());
    GASSERT(y >= 0 && y < bitmap.height());
    return bitmap.fPixels + x + (y * bitmap.rowBytes() >> 2);
}

static void src_row(GPixel row[], int count, GPixel color) {
    for (int i = 0; i < count; ++i) {
        row[i] = color;
    }
}

static void srcover_row(GPixel row[], int count, GPixel src) {
    unsigned sa = GPixel_GetA(src);
    unsigned sr = GPixel_GetR(src);
    unsigned sg = GPixel_GetG(src);
    unsigned sb = GPixel_GetB(src);
    unsigned isa = 255 - sa;

    for (int i = 0; i < count; ++i) {
        GPixel dst = row[i];
        unsigned da = GPixel_GetA(dst);
        unsigned dr = GPixel_GetR(dst);
        unsigned dg = GPixel_GetG(dst);
        unsigned db = GPixel_GetB(dst);

        unsigned a = sa + GDiv255(isa * da);
        unsigned r = sr + GDiv255(isa * dr);
        unsigned g = sg + GDiv255(isa * dg);
        unsigned b = sb + GDiv255(isa * db);
        row[i] = GPixel_PackARGB(a, r, g, b);
    }
}

typedef void (*RowProc)(GPixel[], int, GPixel);

static void srcover_rect(const GBitmap& bitmap, const GIRect& rect, const GPixel& color) {
    unsigned a = GPixel_GetA(color);
    if (0 == a) {
        return;
    }

    RowProc proc = (255 == a) ? src_row : srcover_row;
    const int width = rect.width();

    for (int y = rect.top(); y < rect.bottom(); ++y) {
        proc(get_addr(bitmap, rect.x(), y), width, color);
    }
}

static void src_rect(const GBitmap& bitmap, const GIRect& rect, const GPixel& color) {
    const int width = rect.width();
    for (int y = rect.top(); y < rect.bottom(); ++y) {
        src_row(get_addr(bitmap, rect.x(), y), width, color);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class MikeCanvas : public GCanvas {
public:
    MikeCanvas(const GBitmap& device)
        : fDevice(device)
        , fIBounds(GIRect::MakeWH(fDevice.width(), fDevice.height()))
        , fBounds(  GRect::MakeWH(fDevice.width(), fDevice.height()))
    {}

    virtual void clear(const GColor& color) {
        src_rect(fDevice, fIBounds, color.premulToPixel());
    }
    
    virtual void fillRect(const GRect& rect, const GColor& color) {
        GPixel c = color.premulToPixel();
        if (0 == GPixel_GetA(c)) {
            return;
        }

        GIRect ir = fIBounds;
        if (ir.intersect(rect.round())) {
            srcover_rect(fDevice, ir, c);
        }
    }

private:
    const GBitmap   fDevice;
    const GRect     fBounds;
    const GIRect    fIBounds;
};

GCanvas* GCanvas::Create(const GBitmap& device) {
    if (device.width() < 0 || device.height() < 0 || NULL == device.pixels() ||
        device.rowBytes() < device.width() * sizeof(GPixel)) {
        return NULL;
    }
    return new MikeCanvas(device);
}
