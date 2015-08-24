/**
 *  Copyright 2015 Mike Reed
 *
 *  COMP 575 -- Fall 2015
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRect.h"
#include "GPoint.h"
#include <string>

static void setup_bitmap(GBitmap* bitmap, int w, int h) {
    bitmap->fWidth = w;
    bitmap->fHeight = h;
    bitmap->fRowBytes = w * sizeof(GPixel);
    bitmap->fPixels = (GPixel*)calloc(bitmap->rowBytes(), bitmap->height());
    
}

static void draw_blocks(GCanvas* canvas) {
    const struct {
        GRect   fRect;
        GColor  fColor;
    } rec[] = {
        { GRect::MakeXYWH( 0,  0, 50, 50), GColor::MakeARGB(1, 1, 0, 0) },
        { GRect::MakeXYWH(50,  0, 50, 50), GColor::MakeARGB(1, 0, 1, 0) },
        { GRect::MakeXYWH( 0, 50, 50, 50), GColor::MakeARGB(1, 0, 0, 1) },
        { GRect::MakeXYWH(50, 50, 50, 50), GColor::MakeARGB(1, 0, 0, 0) },
        { GRect::MakeXYWH(25, 25, 50, 50), GColor::MakeARGB(0.75, 1, 1, 1) },
    };

    for (int i = 0; i < GARRAY_COUNT(rec); ++i) {
        canvas->fillRect(rec[i].fRect, rec[i].fColor);
    }
}

static void draw_bitmaps(GCanvas* canvas) {
    GBitmap tex;
    tex.readFromFile("spock_png");

    canvas->fillRectBitmap(GRect::MakeWH(tex.width(), tex.height()), tex);
    canvas->fillRectBitmap(GRect::MakeLTRB(10, 140, 138, 500), tex);
    canvas->fillRectBitmap(GRect::MakeLTRB(140, 10, 500, 138), tex);
    canvas->fillRectBitmap(GRect::MakeXYWH(256, 256, 512, 512), tex);
}

static void draw_tri(GCanvas* canvas) {
    GPoint pts[] = {
        { 10, 10 },
        { 200, 50 },
        { 100, 200 },
    };
    canvas->fillConvexPolygon(pts, GARRAY_COUNT(pts), GColor::MakeARGB(1, 1, 0, 0));
}

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = 6.28318530718f / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
        angle += deltaAngle;
    }
}

static void draw_poly(GCanvas* canvas) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        GColor c = GColor::MakeARGB(0.8f,
                                    fabs(sin(count*7)),
                                    fabs(sin(count*11)),
                                    fabs(sin(count*17)));
        canvas->fillConvexPolygon(storage, count, c);
    }
}

const struct DrawRec {
    void        (*fDraw)(GCanvas*);
    int         fWidth;
    int         fHeight;
    const char* fName;
} gRec[] = {
    { draw_blocks,  100, 100, "blocks"    },
    { draw_bitmaps, 512, 512, "bitmaps"   },
    { draw_tri,     512, 512, "tri"   },
    { draw_poly,    512, 512, "poly"   },
};

/////////////////////////////////////////////////////////////////////////////////////////////////

static GMatrix matrix_identity(int w, int h) {
    return GMatrix();
}

static GMatrix matrix_scale(int w, int h) {
    GMatrix matrix;
    matrix.setTranslate(10, 10);
    matrix.preScale(0.75f, 0.75f);
    return matrix;
}

static GMatrix matrix_flip_x(int w, int h) {
    GMatrix matrix;
    matrix.setTranslate(w, 0);
    matrix.preScale(-1, 1);
    return matrix;
}

static GMatrix matrix_flip_y(int w, int h) {
    GMatrix matrix;
    matrix.setTranslate(0, h);
    matrix.preScale(1, -1);
    return matrix;
}

static GMatrix matrix_rotate(int w, int h) {
    GMatrix matrix;
    matrix.setTranslate(w/2, h/2);
    matrix.preRotate(M_PI / 6);
    matrix.preTranslate(-w/2, -h/2);
    return matrix;
}

const struct MatrixRec {
    GMatrix (*fMatrix)(int x, int y);
    const char* fSuffix;
} gMatrixRec[] = {
    { matrix_identity, ""        },
    { matrix_scale,    "_scale"  },
    { matrix_flip_x,   "_flip_x"  },
    { matrix_flip_y,   "_flip_y"  },
    { matrix_rotate,   "_rotate" },
};

/////////////////////////////////////////////////////////////////////////////////////////////////

static void handle_proc(const DrawRec& rec, const char path[], const MatrixRec& mrec) {
    GBitmap bitmap;
    setup_bitmap(&bitmap, rec.fWidth, rec.fHeight);

    GCanvas* canvas = GCanvas::Create(bitmap);
    if (NULL == canvas) {
        fprintf(stderr, "failed to create canvas for [%d %d] %s\n",
                rec.fWidth, rec.fHeight, rec.fName);
        return;
    }

    canvas->setCTM(mrec.fMatrix(rec.fWidth, rec.fHeight));
    rec.fDraw(canvas);

    if (!bitmap.writeToFile(path)) {
        fprintf(stderr, "failed to write %s\n", path);
    }

    delete canvas;
    free(bitmap.fPixels);
}

int main(int argc, char** argv) {
    const char* pathRoot = "";
    const char* match = NULL;

    for (int i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--write") && i+1 < argc) {
            pathRoot = argv[++i];
        } else if (0 == strcmp(argv[i], "--match") && i+1 < argc) {
            match = argv[++i];
        }
    }

    std::string root(pathRoot);
    if (root.size() > 0 && root[root.size() - 1] != '/') {
        root += "/";
    }

    for (int j = 0; j < GARRAY_COUNT(gMatrixRec); ++j) {
        for (int i = 0; i < GARRAY_COUNT(gRec); ++i) {
            std::string path(pathRoot);
            path += gRec[i].fName;
            path += gMatrixRec[j].fSuffix;
            path += ".png";

            if (match && !strstr(path.c_str(), match)) {
                continue;
            }
            handle_proc(gRec[i], path.c_str(), gMatrixRec[j]);
        }
    }
    return 0;
}
