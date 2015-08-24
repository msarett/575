/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GTypes.h"

class GBitmap;
class GColor;
class GPoint;
class GRect;

class GCanvas {
public:
    static GCanvas* Create(const GBitmap&);

    virtual ~GCanvas() {}

    /**
     *  Fill the rectangle with the color.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     *
     *  Any area in the rectangle that is outside of the bounds of the canvas is ignored.
     *
     *  If the color's alpha is < 1, blend it using SRCOVER blend mode.
     */
    virtual void fillRect(const GRect&, const GColor&) = 0;
    
    /**
     *  Scale and translate the bitmap such that it fills the specific rectangle.
     *
     *  Any area in the rectangle that is outside of the bounds of the canvas is ignored.
     *
     *  If a given pixel in the bitmap is not opaque (e.g. GPixel_GetA() < 255) then blend it
     *  using SRCOVER blend mode.
     */
    virtual void fillRectBitmap(const GRect&, const GBitmap&) = 0;

    
    /**
     *  Fill the convex polygon with the color, following the same "containment" rule as
     *  rectangles.
     *
     *  Any area in the polygon that is outside of the bounds of the canvas is ignored.
     *
     *  If the color's alpha is < 1, blend it using SRCOVER blend mode.
     */
    virtual void fillConvexPolygon(const GPoint[], int count, const GColor&) = 0;
};

#endif
