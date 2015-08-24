/*
 *  Copyright 2015 <your name here>
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"

#include "mike_malloc.h"
#include "mike_scan.h"
#include "mike_utils.h"

#include <vector>

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
        GPixel dstP = dst[i];
        if (0 == dstP) {
            dst[i] = srcP;
            continue;
        }

        unsigned sr = GPixel_GetR(srcP);
        unsigned sg = GPixel_GetG(srcP);
        unsigned sb = GPixel_GetB(srcP);
        unsigned isa = 255 - sa;

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

class ShaderBlitter : public MikeBlitter {
public:
    ShaderBlitter(const GBitmap& device, MikeShader* shader)
        : fDevice(device)
        , fShader(shader)
    {
        fStorage = new GPixel[device.width()];
    }

    virtual ~ShaderBlitter() {
        delete[] fStorage;
    }
    
    virtual void blitRow(int y, int x, int count) {
        GASSERT(count <= fDevice.width());
        fShader->shadeRow(y, x, count, fStorage);
        blend_row(get_addr(fDevice, x, y), fStorage, count);
    }
    
private:
    const GBitmap fDevice;
    MikeShader*   fShader;
    GPixel*       fStorage;
};

class BitmapShader : public MikeShader {
public:
    BitmapShader(const GMatrix& ctm, const GBitmap& tex, const GMatrix* localM) : fTex(tex) {
        if (localM) {
            GMatrix total;
            total.setConcat(ctm, *localM);
            total.invert(&fInverse);
        } else {
            ctm.invert(&fInverse);
        }
    }

    virtual void shadeRow(int dstY, int dstX, int count, GPixel row[]) {
        GPoint tmp = fInverse.mapXY(dstX + 0.5f, dstY + 0.5f);
        float x = tmp.fX;
        float y = tmp.fY;
        const float dx = fInverse[0];
        const float dy = fInverse[1];

        const int maxX = fTex.width() - 1;
        const int maxY = fTex.height() - 1;
        for (int i = 0; i < count; ++i) {
            int ix = clamp((int)x, 0, maxX);
            int iy = clamp((int)y, 0, maxY);
            row[i] = *get_addr(fTex, ix, iy);
            x += dx;
            y += dy;
        }
    }

private:
    const GBitmap   fTex;
    GMatrix         fInverse;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class GCanvasMike : public GCanvas {
public:
    GCanvasMike(const GBitmap& device) : fDevice(device) {}

    virtual void fillRect(const GRect& rect, const GColor& color) {
        GPixel c = color.premulToPixel();
        if (0 == GPixel_GetA(c)) {
            return;
        }
        
        if (fCTM.getType() & GMatrix::kAffine_TypeMask) {
            GPoint quad[4];
            to_quad(rect, quad);
            this->fillConvexPolygon(quad, 4, color);
            return;
        }

        GIRect bounds = GIRect::MakeWH(fDevice.width(), fDevice.height());
        GIRect ir = fCTM.mapRect(rect).round();
        if (ir.intersect(bounds)) {
            fill_rect(fDevice, ir, c);
        }
    }
    
    void blitConvexPolygon(const GPoint pts[], int count, MikeBlitter* blitter) {
        if (count < 3) {
            return;
        }
        AutoTArray<GPoint>  storage;
        if (fCTM.getType() != GMatrix::kIdentity_TypeMask) {
            fCTM.mapPoints(storage.setCount(count), pts, count);
            pts = storage;
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

        MikeScanConvex(pts, count, blitter, clipBounds);
    }

    virtual void fillRectBitmap(const GRect& dstR, const GBitmap& tex) {
        if (tex.width() == 0 || tex.height() == 0) {
            return;
        }
        GMatrix localM;
        set_rect_to_rect(&localM, GRect::MakeWH(tex.width(), tex.height()), dstR);
        BitmapShader shader(fCTM, tex, &localM);
        ShaderBlitter blitter(fDevice, &shader);

        GPoint quad[4];
        this->blitConvexPolygon(to_quad(dstR, quad), 4, &blitter);
    }

    virtual void fillConvexPolygon(const GPoint pts[], int count, const GColor& color) {
        const GPixel pixel = color.premulToPixel();
        if (GPixel_GetA(pixel) == 0) {
            return;
        }
        if (count < 3) {
            return;
        }
        RowBlitter blitter(fDevice, 0xFF == GPixel_GetA(pixel) ? src_row : srcover_row, pixel);
        this->blitConvexPolygon(pts, count, &blitter);
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
