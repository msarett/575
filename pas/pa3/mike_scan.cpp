/*
 *  Copyright 2015 Mike Reed
 */

#include "GPoint.h"
#include "GRect.h"
#include "mike_scan.h"
#include "mike_malloc.h"

class ClippingBlitter : public MikeBlitter {
public:
    ClippingBlitter() : fProxy(NULL) {}

    void setProxy(const GIRect& bounds, MikeBlitter* proxy) {
        fBounds = bounds;
        fProxy = proxy;
    }

    virtual void blitRow(int y, int x, int count) {
        GASSERT(fProxy);

        if (y >= fBounds.top() && y < fBounds.bottom()) {
            int x0 = std::max(x, fBounds.left());
            int x1 = std::min(x + count, fBounds.right());
            if (x0 < x1) {
                fProxy->blitRow(y, x0, x1 - x0);
            }
        }
    }

private:
    MikeBlitter*    fProxy;
    GIRect          fBounds;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef int32_t GFixed;

static GFixed float_to_fixed(float x) {
    return (GFixed)(x * 65536);
}

struct Edge {
    GFixed  fX;
    GFixed  fDX;
    int     fY;
    int     fLastY;

    bool init(GPoint p0, GPoint p1) {
        int y0 = GRoundToInt(p0.fY);
        int y1 = GRoundToInt(p1.fY);
        if (y0 == y1) {
            return false;
        }
        if (y0 > y1) {
            std::swap(p0, p1);
            std::swap(y0, y1);
        }

        const float slope = (p1.fX - p0.fX) / (p1.fY - p0.fY);
        const float dy = y0 - p0.fY + 0.5f;

        fDX = float_to_fixed(slope);
        fX = float_to_fixed(p0.fX + slope * dy + 0.5f);
        fY = y0;
        fLastY = y1 - 1;
        return true;
    }

    friend bool operator<(const Edge& prev, const Edge& next) {
        if (prev.fY < next.fY) {
            return true;
        }
        if (prev.fY > next.fY) {
            return false;
        }
        return prev.fX < next.fX;
    }
};

static void walk_convex(Edge edge[], int count, MikeBlitter* blitter) {
    int y = edge[0].fY;
    const int lastY = edge[count - 1].fLastY;
    GASSERT(y <= lastY);
    
    Edge* leftE = edge++;
    Edge* riteE = edge++;
    GASSERT(leftE->fY == riteE->fY);

    for (;;) {
        GASSERT(y >= leftE->fY && y <= leftE->fLastY);
        GASSERT(y >= riteE->fY && y <= riteE->fLastY);
        GASSERT(leftE->fX <= riteE->fX);

        int x0 = leftE->fX >> 16;
        int x1 = riteE->fX >> 16;
        if (x1 > x0) {
            blitter->blitRow(y, x0, x1 - x0);
        }
        if (++y > lastY) {
            break;
        }

        if (y <= leftE->fLastY) {
            leftE->fX += leftE->fDX;
        } else {
            leftE = edge++;
        }
        if (y <= riteE->fLastY) {
            riteE->fX += riteE->fDX;
        } else {
            riteE = edge++;
        }
    }
}

static Edge* clip_line(const GRect& bounds, GPoint p0, GPoint p1, Edge* edge) {
    if (p0.fY == p1.fY) {
        return edge;
    }

    if (p0.fY > p1.fY) {
        std::swap(p0, p1);
    }
    // now we're monotonic in Y: p0 <= p1
    if (p1.fY <= bounds.top() || p0.fY >= bounds.bottom()) {
        return edge;
    }
    
    double dxdy = (double)(p1.fX - p0.fX) / (p1.fY - p0.fY);
    if (p0.fY < bounds.top()) {
        p0.fX += dxdy * (bounds.top() - p0.fY);
        p0.fY = bounds.top();
    }
    if (p1.fY > bounds.bottom()) {
        p1.fX += dxdy * (bounds.bottom() - p1.fY);
        p1.fY = bounds.bottom();
    }

    // Now p0...p1 is strictly inside bounds vertically, so we just need to clip horizontally

    if (p0.fX > p1.fX) {
        std::swap(p0, p1);
    }
    // now we're left-to-right: p0 .. p1

    if (p1.fX <= bounds.left()) {   // entirely to the left
        p0.fX = p1.fX = bounds.left();
        return edge + edge->init(p0, p1);
    }
    if (p0.fX >= bounds.right()) {  // entirely to the right
        p0.fX = p1.fX = bounds.right();
        return edge + edge->init(p0, p1);
    }

    if (p0.fX < bounds.left()) {
        float y = p0.fY + (bounds.left() - p0.fX) / dxdy;
        edge += edge->init(GPoint::Make(bounds.left(), p0.fY), GPoint::Make(bounds.left(), y));
        p0.set(bounds.left(), y);
    }
    if (p1.fX > bounds.right()) {
        float y = p0.fY + (bounds.right() - p0.fX) / dxdy;
        edge += edge->init(p0, GPoint::Make(bounds.right(), y));
        edge += edge->init(GPoint::Make(bounds.right(), y), GPoint::Make(bounds.right(), p1.fY));
    } else {
        edge += edge->init(p0, p1);
    }
    return edge;
}

void MikeScanConvex(const GPoint pts[], int count, MikeBlitter* blitter, const GIRect* clipBounds) {
    AutoTArray<Edge> storage(count*2);
    Edge* edge = storage;

    if (clipBounds) {
        const GRect cb = GRect::Make(*clipBounds);
        for (int i = 1; i < count; ++i) {
            edge = clip_line(cb, pts[i - 1], pts[i], edge);
        }
        edge = clip_line(cb, pts[count - 1], pts[0], edge);
    } else {
        for (int i = 1; i < count; ++i) {
            if (edge->init(pts[i - 1], pts[i])) {
                edge += 1;
            }
        }
        if (edge->init(pts[count - 1], pts[0])) {
            edge += 1;
        }
    }
    
    int edgeCount = edge - storage;
    if (edgeCount < 2) {
        if (1 == edgeCount) {
            printf("OOPS : edgeCount = %d\n", edgeCount);
        }
        return;
    }

#if 0
    ClippingBlitter clipper;
    if (clipBounds) {
        clipper.setProxy(*clipBounds, blitter);
        blitter = &clipper;
    }
#endif
    
    Edge* start = storage;
    std::sort(start, edge);
    walk_convex(start, edgeCount, blitter);
}
