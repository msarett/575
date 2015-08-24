/**
 *  Copyright 2015 Mike Reed
 */

#include "GBitmap.h"
#include "GPixel.h"
#include <string>

//
//  To be provided by the student in a separate .cpp file
//
extern void cs575_draw_into_bitmap(const GBitmap&);

static void draw_circle(const GBitmap& bitmap) {
    const GPixel px = GPixel_PackARGB(0xFF, 0xFF, 0, 0);
    const float cx = (float)bitmap.width() / 2;
    const float cy = (float)bitmap.height() / 2;
    const float radius = cx * 5 / 6;
    const float radius2 = radius * radius;
    
    GPixel* dst = bitmap.pixels();
    for (int y = 0; y < bitmap.height(); ++y) {
        const float dy = y - cy;
        for (int x = 0; x < bitmap.width(); ++x) {
            const float dx = x - cx;
            const float dist2 = dx*dx + dy*dy;
            if (dist2 <= radius2) {
                dst[x] = px;
            } else {
                dst[x] = 0; // transparent
            }
        }
        dst = (GPixel*)((char*)dst + bitmap.rowBytes());
    }
}

static void draw_ramp(const GBitmap& bitmap) {
    const float g0 = 0;
    const float g1 = 255;
    const float dg = (g1 - g0) / bitmap.width();

    const float b0 = 0;
    const float b1 = 255;
    const float db = (b1 - b0) / bitmap.height();

    GPixel* dst = bitmap.fPixels;
    float g = g0 + dg/2;
    for (int y = 0; y < bitmap.height(); ++y) {
        float b = b0 + db/2;
        for (int x = 0; x < bitmap.width(); ++x) {
            dst[x] = GPixel_PackARGB(0xFF, 0, (int)g, (int)b);
            b += db;
        }
        g += dg;
        dst = (GPixel*)((char*)dst + bitmap.rowBytes());
    }
}

const struct {
    void (*fProc)(const GBitmap&);
    const char* fName;
} gRec[] = {
    { draw_circle,              "circle" },
    { draw_ramp,                "ramp"   },
    { cs575_draw_into_bitmap,   "custom" },
};

/////////////////////////////////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <unistd.h>

static bool is_dir(const char path[]) {
    struct stat status;
    return !stat(path, &status) && (status.st_mode & S_IFDIR);
}

static bool mk_dir(const char path[]) {
    if (is_dir(path)) {
        return true;
    }
    if (!access(path, F_OK)) {
        fprintf(stderr, "%s exists but is not a directory\n", path);
        return false;
    }
    if (mkdir(path, 0777)) {
        fprintf(stderr, "error creating dir %s\n", path);
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static void handle_proc(const GBitmap& bitmap, void (*proc)(const GBitmap&),
                        const std::string& root, const char name[]) {
    char suffix[100];
    sprintf(suffix, "_%d.png", bitmap.width());

    std::string path(root);
    path += name;
    path += suffix;

    // fill the bitmap with transparent black
    memset(bitmap.pixels(), 0, bitmap.rowBytes() * bitmap.height());
    proc(bitmap);

    if (!bitmap.writeToFile(path.c_str())) {
        fprintf(stderr, "failed to write %s\n", path.c_str());
    }
}

int main(int argc, char** argv) {
    int N = 256;
    std::string root;

    for (int i = 1; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--size") && i+1 < argc) {
            N = atoi(argv[++i]);
        }
        if (0 == strcmp(argv[i], "--write") && i+1 < argc) {
            root = argv[++i];
        }
        
    }
    if (N < 1 || N > 1024) {
        fprintf(stderr, "specify a reasonable size for the image (e.g. 256)\n");
        return -1;
    }

    if (root.size() > 0 && root[root.size() - 1] != '/') {
        root += "/";
        if (!mk_dir(root.c_str())) {
            return -1;
        }
    }
    
    GBitmap bitmap;
    bitmap.fWidth = N;
    bitmap.fHeight = N;
    bitmap.fRowBytes = N * sizeof(GPixel);
    bitmap.fPixels = (GPixel*)malloc(bitmap.rowBytes() * bitmap.height());

    for (int i = 0; i < GARRAY_COUNT(gRec); ++i) {
        handle_proc(bitmap, gRec[i].fProc, root, gRec[i].fName);
    }
    
    free(bitmap.fPixels);
    return 0;
}
