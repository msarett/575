/**
 *  Copyright 2015 Mike Reed
 *
 *  COMP 575 -- Fall 2015
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRect.h"

class PreviewWindow : public GWindow {
    const GBitmap*  fArray;
    char const* const* fTitles;
    int             fIndex, fCount, fZoom;

public:
    PreviewWindow(const GBitmap array[], char const* const* titles, int count)
        : GWindow(1024, 768)
        , fArray(array)
        , fTitles(titles)
        , fIndex(0)
        , fCount(count)
        , fZoom(0)
    {
        this->updateTitle();
    }

protected:
    float getScale() const {
        float scale = 1;
        if (fZoom > 0) {
            scale = powf(1.25, fZoom);
        } else if (fZoom < 0) {
            scale = 1.0 / powf(1.25, -fZoom);
        }
        return scale;
    }

    void updateTitle() {
        const GBitmap& bm = fArray[fIndex];

        char buffer[400];
        sprintf(buffer, "%s %dx%d  zoom=%g",
                fTitles[fIndex], bm.width(), bm.height(), this->getScale());
        this->setTitle(buffer);
    }

    virtual void onDraw(GCanvas* canvas) {
        const float gray = 0.87f;
        canvas->fillRect(GRect::MakeWH(9999, 9999), GColor::MakeARGB(1, gray, gray, gray));

        int sw = fArray[fIndex].width();
        int sh = fArray[fIndex].height();
        int dw = this->width();
        int dh = this->height();

        GMatrix matrix;
        matrix.setTranslate(dw * 0.5, dh * 0.5);
        matrix.preScale(this->getScale(), this->getScale());
        matrix.preTranslate(-sw * 0.5, -sh * 0.5);
        canvas->setCTM(matrix);

        canvas->fillRectBitmap(GRect::MakeWH(sw, sh), fArray[fIndex]);
    }

    virtual bool onKeyPress(const XEvent&, KeySym sym) {
        switch (sym) {
            case XK_Left:
                if (--fIndex < 0) {
                    fIndex = fCount - 1;
                }
                break;
            case XK_Right:
                fIndex = (fIndex + 1) % fCount;
                break;
            case XK_Up:
                fZoom = std::min(fZoom + 1, 16);
                break;
            case XK_Down:
                fZoom = std::max(fZoom - 1, -16);
                break;
            default:
                return false;
        }
        this->updateTitle();
        this->requestDraw();
        return true;
    }
    
private:
    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    if (argc <= 1) {
        return 0;
    }

    GBitmap* array = new GBitmap[argc - 1];
    int count = 0;
    for (int i = 1; i < argc; ++i) {
        if (!array[i - 1].readFromFile(argv[i])) {
            break;
        }
        count += 1;
    }

    if (count > 0) {
        PreviewWindow* wind = new PreviewWindow(array, argv + 1, count);
        return wind->run();
    }
    return -1;
}

