/*
 *  Copyright 2015 <your name here>
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"

#include "mike_matrix.h"
#include "mike_malloc.h"
#include "mike_scan.h"

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

static void fill_rect(const GBitmap& bitmap, const GIRect& rect, const GPixel& color) {
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

static void blend_row(GPixel dst[], const GPixel src[], int count) {
    for (int i = 0; i < count; ++i) {
        GPixel srcP = src[i];
        if (0 == srcP) {
            continue;
        }

        unsigned sa = GPixel_GetA(srcP);
        if (0xFF == sa) {
            dst[i] = srcP;
            continue;
        }

        unsigned sr = GPixel_GetR(srcP);
        unsigned sg = GPixel_GetG(srcP);
        unsigned sb = GPixel_GetB(srcP);
        unsigned isa = 255 - sa;

        GPixel dstP = dst[i];
        unsigned da = GPixel_GetA(dstP);
        unsigned dr = GPixel_GetR(dstP);
        unsigned dg = GPixel_GetG(dstP);
        unsigned db = GPixel_GetB(dstP);
        
        unsigned a = sa + GDiv255(isa * da);
        unsigned r = sr + GDiv255(isa * dr);
        unsigned g = sg + GDiv255(isa * dg);
        unsigned b = sb + GDiv255(isa * db);
        dst[i] = GPixel_PackARGB(a, r, g, b);
    }
}

static int clamp(int value, int min_value, int max_value) {
    return std::max(std::min(value, max_value), min_value);
}

static void shade_span(const GBitmap& tex, const GMatrix& inv, int dstX, int dstY,
                       GPixel span[], int count) {
    float tmp[2];
    inv.mapXY(dstX + 0.5f, dstY + 0.5f, tmp);
    float x = tmp[0];
    float y = tmp[1];
    const float dx = inv.scaleX();
    const float dy = inv.skewY();

    const int maxX = tex.width() - 1;
    const int maxY = tex.height() - 1;
    for (int i = 0; i < count; ++i) {
        int ix = clamp((int)x, 0, maxX);
        int iy = clamp((int)y, 0, maxY);
        span[i] = *get_addr(tex, ix, iy);
        x += dx;
        y += dy;
    }
}

static void tex_rect(const GBitmap& device, const GIRect& ir, const GMatrix& inv,
                     const GBitmap& tex) {
    GPixel storage[1000];

    const int count = ir.width();
    const int x = ir.left();
    for (int y = ir.top(); y < ir.bottom(); ++y) {
        GPixel* dst = get_addr(device, x, y);
        shade_span(tex, inv, x, y, storage, count);
        blend_row(dst, storage, count);
    }
}

static GRect compute_bounds(const GPoint pts[], int count) {
    GASSERT(count > 0);

    float L, T, R, B;
    L = R = pts[0].fX;
    T = B = pts[0].fY;
    for (int i = 1; i < count; ++i) {
        L = std::min(L, pts[i].fX);
        T = std::min(T, pts[i].fY);
        R = std::max(R, pts[i].fX);
        B = std::max(B, pts[i].fY);
    }
    return GRect::MakeLTRB(L, T, R, B);
}

static bool does_intersect(const GRect& a, const GRect& b) {
    if (a.isEmpty() || b.isEmpty()) {
        return false;
    }
    return a.fLeft < b.fRight && b.fLeft < a.fRight && a.fTop < b.fBottom && b.fTop < a.fBottom;
}

template <typename T> bool contains(const T& outer, const T& inner) {
    return  outer.fLeft <= inner.fLeft && outer.fRight >= inner.fRight &&
            outer.fTop <= inner.fTop && outer.fBottom >= inner.fBottom;
}

class RowBlitter : public MikeBlitter {
public:
    RowBlitter(const GBitmap& device, void (*proc)(GPixel[], int, GPixel), GPixel pixel)
        : fDevice(device)
        , fProc(proc)
        , fPixel(pixel)
    {}

    virtual void blitRow(int y, int x, int count) {
        fProc(get_addr(fDevice, x, y), count, fPixel);
    }

private:
    const GBitmap fDevice;
    void (*fProc)(GPixel[], int, GPixel);
    GPixel fPixel;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GCanvasMike : public GCanvas {
public:
    GCanvasMike(const GBitmap& device) : fDevice(device) {}

    virtual void clear(const GColor& color) {
        GPixel pixel = color.premulToPixel();
        for (int y = 0; y < fDevice.height(); ++y) {
            src_row(get_addr(fDevice, 0, y), pixel, fDevice.width());
        }
    }

    virtual void fillRect(const GRect& rect, const GColor& color) {
        GPixel c = color.premulToPixel();
        if (0 == GPixel_GetA(c)) {
            return;
        }
        
        GIRect bounds = GIRect::MakeWH(fDevice.width(), fDevice.height());
        GIRect ir = rect.round();
        if (!ir.intersect(bounds)) {
            return;
        }
        
        fill_rect(fDevice, ir, c);
    }

    virtual void fillRectBitmap(const GRect& dstR, const GBitmap& tex) {
        if (tex.width() == 0 || tex.height() == 0) {
            return;
        }

        GIRect bounds = GIRect::MakeWH(fDevice.width(), fDevice.height());
        GIRect idstR = dstR.round();
        if (!idstR.intersect(bounds)) {
            return;
        }

        GRect srcR = GRect::MakeWH(tex.width(), tex.height());
        GMatrix inv;
        inv.setRectToRect(dstR, srcR);

        tex_rect(fDevice, idstR, inv, tex);
    }

    virtual void fillConvexPolygon(const GPoint pts[], int count, const GColor& color) {
        const GPixel pixel = color.premulToPixel();
        if (GPixel_GetA(pixel) == 0) {
            return;
        }
        if (count < 3) {
            return;
        }

        const GRect srcBounds = compute_bounds(pts, count);
        const GRect deviceBounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        if (!does_intersect(srcBounds, deviceBounds)) {
            return;
        }

        const GIRect* clipBounds = NULL;
        GIRect clipStorage;
        if (!contains(deviceBounds, srcBounds)) {
            clipStorage = deviceBounds.round();
            clipBounds = &clipStorage;
        }

        RowBlitter blitter(fDevice, 0xFF == GPixel_GetA(pixel) ? src_row : srcover_row, pixel);
        MikeScanConvex(pts, count, &blitter, clipBounds);
    }

private:
    GBitmap fDevice;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

GCanvas* GCanvas::Create(const GBitmap& device) {
    if (device.width() > 0 && device.height() > 0 && device.pixels() != NULL) {
        return new GCanvasMike(device);
    }
    return NULL;
}
