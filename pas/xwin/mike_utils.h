/*
 *  Copyright 2015 Mike Reed
 */

#ifndef mike_utils_DEFINED
#define mike_utils_DEFINED

#include "GRect.h"

static inline GRect compute_bounds(const GPoint pts[], int count) {
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

extern void set_rect_to_rect(GMatrix*, const GRect& src, const GRect& dst);

extern GPoint* to_quad(const GRect&, GPoint pts[4]);

#endif
