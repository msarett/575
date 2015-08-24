/**
 *  Copyright 2015 Mike Reed
 *
 *  575 Introduction to Graphics
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GMATRIX_DEFINED

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
#else
class GMatrix {};
const struct MatrixRec {
    GMatrix (*fMatrix)(int x, int y);
    const char* fSuffix;
} gMatrixRec[] = {
    { NULL, "" },
};
#endif

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

static void setup_bitmap(GBitmap* bitmap, int w, int h) {
    bitmap->fWidth = w;
    bitmap->fHeight = h;
    bitmap->fRowBytes = w * sizeof(GPixel);
    bitmap->fPixels = (GPixel*)calloc(bitmap->rowBytes(), bitmap->height());
    
}

static void handle_proc(const CS575DrawRec& rec, const char path[], const MatrixRec& mrec) {
    GBitmap bitmap;
    setup_bitmap(&bitmap, rec.fWidth, rec.fHeight);

    GCanvas* canvas = GCanvas::Create(bitmap);
    if (NULL == canvas) {
        fprintf(stderr, "failed to create canvas for [%d %d] %s\n",
                rec.fWidth, rec.fHeight, rec.fName);
        return;
    }

#ifdef GMATRIX_DEFINED
    canvas->setCTM(mrec.fMatrix(rec.fWidth, rec.fHeight));
#endif
    rec.fDraw(canvas);

    if (!bitmap.writeToFile(path)) {
        fprintf(stderr, "failed to write %s\n", path);
    }

    delete canvas;
    free(bitmap.fPixels);
}

static bool is_arg(const char arg[], const char name[]) {
    std::string str("--");
    str += name;
    if (!strcmp(arg, str.c_str())) {
        return true;
    }

    char shortVers[3];
    shortVers[0] = '-';
    shortVers[1] = name[0];
    shortVers[2] = 0;
    return !strcmp(arg, shortVers);
}

int main(int argc, char** argv) {
    bool verbose = false;
    std::string root;
    const char* match = NULL;

    for (int i = 1; i < argc; ++i) {
        if (is_arg(argv[i], "verbose")) {
            verbose = true;
        } else if (is_arg(argv[i], "write") && i+1 < argc) {
            root = argv[++i];
        } else if (is_arg(argv[i], "match") && i+1 < argc) {
            match = argv[++i];
        }
    }

    if (root.size() > 0 && root[root.size() - 1] != '/') {
        root += "/";
        if (!mk_dir(root.c_str())) {
            return -1;
        }
    }

    for (int j = 0; j < GARRAY_COUNT(gMatrixRec); ++j) {
        for (int i = 0; gDrawRecs[i].fDraw; ++i) {
            std::string path(root);
            path += gDrawRecs[i].fName;
            path += gMatrixRec[j].fSuffix;
            path += ".png";

            if (match && !strstr(path.c_str(), match)) {
                continue;
            }
            if (verbose) {
                printf("    image: %s\n", path.c_str());
            }
            handle_proc(gDrawRecs[i], path.c_str(), gMatrixRec[j]);
        }
    }
    return 0;
}
