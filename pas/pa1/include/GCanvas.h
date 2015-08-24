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
     *  Fill the entire canvas with the specified color.
     *
     *  This completely overwrites any previous colors, it does not blend.
     */
    virtual void clear(const GColor&) = 0;

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
};

#endif
