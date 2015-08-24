/*
 *  Copyright 2015 Mike Reed
 */

#ifndef mike_scan_DEFINED
#define mike_scan_DEFINED

#include "GBitmap.h"

class GPoint;
class GRect;

class MikeShader {
public:
    virtual ~MikeShader() {}

    virtual void shadeRow(int y, int x, int count, GPixel[]) = 0;
};

class MikeBlitter {
public:
    virtual ~MikeBlitter() {}

    virtual void blitRow(int y, int x, int count) = 0;
};

extern void MikeScanConvex(const GPoint[], int count, MikeBlitter*, const GIRect* clipBounds);

#endif
