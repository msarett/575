/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GColor_DEFINED
#define GColor_DEFINED

#include "GMath.h"
#include "GPixel.h"

class GColor {
public:
    float   fA;
    float   fR;
    float   fG;
    float   fB;

    static GColor MakeARGB(float a, float r, float g, float b) {
        GColor c = { a, r, g, b };
        return c;
    }

    GColor pinToUnit() const {
        return MakeARGB(GPinToUnit(fA), GPinToUnit(fR), GPinToUnit(fG), GPinToUnit(fB));
    }

    GPixel premulToPixel() const {
        float a = GPinToUnit(fA);
        float r = GPinToUnit(fR);
        float g = GPinToUnit(fG);
        float b = GPinToUnit(fB);

        a *= 255;
        int ia = GRoundToInt(a);
        int ir = GRoundToInt(a * r);
        int ig = GRoundToInt(a * g);
        int ib = GRoundToInt(a * b);
        return GPixel_PackARGB(ia, ir, ig, ib);
    }
};

#endif
