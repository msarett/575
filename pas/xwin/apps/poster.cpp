/**
 *  Copyright 2015 Mike Reed
 *
 *  COMP 575 -- Fall 2015
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"

#include <vector>

#define MIN_N       4
#define WINDOW_MAX  768

static GPixel* get_addr(const GBitmap& bitmap, int x, int y) {
    return bitmap.pixels() + (bitmap.rowBytes() >> 2) * y + x;
}

class TestWindow : public GWindow {
    const GBitmap fBitmap;
    int fN, fMaxN;
    float fScale;

public:
    TestWindow(int w, int h, const GBitmap& bitmap) : GWindow(w, h), fBitmap(bitmap) {
        fN = MIN_N;
        fScale = WINDOW_MAX * 1.0 / std::max(bitmap.width(), bitmap.height());
        fMaxN = std::min(bitmap.width(), bitmap.height());
    }

    GColor sample(float ux, float uy) const {
        const float scale = 1 / 255.0;
        int x = ux * fBitmap.width();
        int y = uy * fBitmap.height();
        GPixel p = *get_addr(fBitmap, x, y);
        return GColor::MakeARGB(GPixel_GetA(p) * scale,
                                GPixel_GetR(p) * scale,
                                GPixel_GetG(p) * scale,
                                GPixel_GetB(p) * scale);
    }

protected:
    virtual bool onKeyPress(const XEvent&, KeySym sym) {
        int n = fN;
        switch (sym) {
            case XK_Up:
                n = (int)(n / 0.8);
                break;
            case XK_Down:
                n = (int)(n * 0.8);
                break;
            default:
                return false;
        }
        fN = std::max(std::min(fMaxN, n), MIN_N);
        this->requestDraw();
        return true;
    }
    
    virtual void onDraw(GCanvas* canvas) {
        const float invN = 1.0 / fN;
        const float rw = fBitmap.width() * fScale * invN;
        const float rh = fBitmap.height() * fScale * invN;

        for (int y = 0; y < fN; ++y) {
            for (int x = 0; x < fN; ++x) {
                float fx = (x + 0.5) * invN;
                float fy = (y + 0.5) * invN;
                GRect r = GRect::MakeXYWH(x * rw, y * rh, rw, rh);
                canvas->fillRect(r, this->sample(fx, fy));
            }
        }
    }

private:
    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    if (argc == 2) {
        GBitmap bitmap;
        if (bitmap.readFromFile(argv[1])) {
            TestWindow* wind = new TestWindow(WINDOW_MAX, WINDOW_MAX, bitmap);
            return wind->run();
        }
    }
    return -1;
}

