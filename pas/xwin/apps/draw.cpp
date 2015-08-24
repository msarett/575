/**
 *  Copyright 2015 Mike Reed
 *
 *  COMP 575 -- Fall 2015
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"

#include <vector>

static const float CORNER_SIZE = 9;

template <typename T> int find_index(const std::vector<T*>& list, T* target) {
    for (int i = 0; i < list.size(); ++i) {
        if (list[i] == target) {
            return i;
        }
    }
    return -1;
}

static GRandom gRand;

static GColor rand_color() {
    return GColor::MakeARGB(0.5f, gRand.nextF(), gRand.nextF(), gRand.nextF());
}

static GRect make_from_pts(const GPoint& p0, const GPoint& p1) {
    return GRect::MakeLTRB(std::min(p0.fX, p1.fX), std::min(p0.fY, p1.fY),
                           std::max(p0.fX, p1.fX), std::max(p0.fY, p1.fY));
}

static bool contains(const GRect& rect, float x, float y) {
    return rect.left() < x && x < rect.right() && rect.top() < y && y < rect.bottom();
}

static GRect offset(const GRect& rect, float dx, float dy) {
    return GRect::MakeLTRB(rect.left() + dx, rect.top() + dy,
                           rect.right() + dx, rect.bottom() + dy);
}

static bool hit_test(float x0, float y0, float x1, float y1) {
    const float dx = fabs(x1 - x0);
    const float dy = fabs(y1 - y0);
    return std::max(dx, dy) <= CORNER_SIZE;
}

static bool in_resize_corner(const GRect& r, float x, float y, GPoint* anchor) {
    if (hit_test(r.left(), r.top(), x, y)) {
        anchor->set(r.right(), r.bottom());
        return true;
    } else if (hit_test(r.right(), r.top(), x, y)) {
        anchor->set(r.left(), r.bottom());
        return true;
    } else if (hit_test(r.right(), r.bottom(), x, y)) {
        anchor->set(r.left(), r.top());
        return true;
    } else if (hit_test(r.left(), r.bottom(), x, y)) {
        anchor->set(r.right(), r.top());
        return true;
    }
    return false;
}

static void draw_corner(GCanvas* canvas, const GColor& c, float x, float y, float dx, float dy) {
    canvas->fillRect(make_from_pts(GPoint::Make(x, y - 1), GPoint::Make(x + dx, y + 1)), c);
    canvas->fillRect(make_from_pts(GPoint::Make(x - 1, y), GPoint::Make(x + 1, y + dy)), c);
}

static void draw_hilite(GCanvas* canvas, const GRect& r) {
    const float size = CORNER_SIZE;
    GColor c = GColor::MakeARGB(1, 0, 0, 0);
    draw_corner(canvas, c, r.fLeft, r.fTop, size, size);
    draw_corner(canvas, c, r.fLeft, r.fBottom, size, -size);
    draw_corner(canvas, c, r.fRight, r.fTop, -size, size);
    draw_corner(canvas, c, r.fRight, r.fBottom, -size, -size);
}

static void constrain_color(GColor* c) {
    c->fA = std::max(std::min(c->fA, 1.f), 0.1f);
    c->fR = std::max(std::min(c->fR, 1.f), 0.f);
    c->fG = std::max(std::min(c->fG, 1.f), 0.f);
    c->fB = std::max(std::min(c->fB, 1.f), 0.f);
}

class Shape {
public:
    virtual ~Shape() {}
    virtual void draw(GCanvas* canvas) {}
    virtual GRect getRect() = 0;
    virtual void setRect(const GRect&) {}
    virtual GColor getColor() = 0;
    virtual void setColor(const GColor&) {}
};

class RectShape : public Shape {
public:
    RectShape(GColor c) : fColor(c) {
        fRect = GRect::MakeXYWH(0, 0, 0, 0);
    }

    virtual void draw(GCanvas* canvas) {
        canvas->fillRect(fRect, fColor);
    }

    virtual GRect getRect() { return fRect; }
    virtual void setRect(const GRect& r) { fRect = r; }
    virtual GColor getColor() { return fColor; }
    virtual void setColor(const GColor& c) { fColor = c; }

private:
    GRect   fRect;
    GColor  fColor;
};

class ResizeClick : public GClick {
    GPoint  fAnchor;
public:
    ResizeClick(GPoint loc, GPoint anchor) : GClick(loc, "resize"), fAnchor(anchor) {}

    GRect makeRect() const { return make_from_pts(this->curr(), fAnchor); }
};

class TestWindow : public GWindow {
    std::vector<Shape*> fList;
    Shape* fShape;

public:
    TestWindow(int w, int h) : GWindow(w, h) {
        fShape = new RectShape(GColor::MakeARGB(1, 0, 0, 1));
    }

    virtual ~TestWindow() {}
    
protected:
    virtual void onDraw(GCanvas* canvas) {
        canvas->fillRect(GRect::MakeXYWH(0, 0, 10000, 10000), GColor::MakeARGB(1, .5, .5, .5));

        for (int i = 0; i < fList.size(); ++i) {
            fList[i]->draw(canvas);
        }
        if (fShape) {
            draw_hilite(canvas, fShape->getRect());
        }
    }

    virtual bool onKeyPress(const XEvent&, KeySym sym) {
        if (NULL == fShape) {
            return false;
        }
        GColor c = fShape->getColor();
        const float delta = 0.1f;
        switch (sym) {
            case XK_Up: {
                int index = find_index(fList, fShape);
                if (index < fList.size() - 1) {
                    std::swap(fList[index], fList[index + 1]);
                    this->requestDraw();
                    return true;
                }
                return false;
            }
            case XK_Down: {
                int index = find_index(fList, fShape);
                if (index > 0) {
                    std::swap(fList[index], fList[index - 1]);
                    this->requestDraw();
                    return true;
                }
                return false;
            }
            case XK_BackSpace:
                this->removeShape(fShape);
                fShape = NULL;
                this->updateTitle();
                this->requestDraw();
                return true;
            case 'a': c.fA -= delta; break;
            case 'A': c.fA += delta; break;
            case 'r': c.fR -= delta; break;
            case 'R': c.fR += delta; break;
            case 'g': c.fG -= delta; break;
            case 'G': c.fG += delta; break;
            case 'b': c.fB -= delta; break;
            case 'B': c.fB += delta; break;
            default:
                return false;
        }
        constrain_color(&c);
        fShape->setColor(c);
        this->updateTitle();
        this->requestDraw();
        return true;
    }

    virtual GClick* onFindClickHandler(GPoint loc) {
        if (fShape) {
            GPoint anchor;
            if (in_resize_corner(fShape->getRect(), loc.x(), loc.y(), &anchor)) {
                return new ResizeClick(loc, anchor);
            }
        }

        for (int i = fList.size() - 1; i >= 0; --i) {
            if (contains(fList[i]->getRect(), loc.x(), loc.y())) {
                fShape = fList[i];
                this->updateTitle();
                return new GClick(loc, "move");
            }
        }
        
        // else create a new shape
        fShape = new RectShape(rand_color());
        fList.push_back(fShape);
        this->updateTitle();
        return new GClick(loc, "create");
    }

    virtual void onHandleClick(GClick* click) {
        if (click->isName("move")) {
            const GPoint curr = click->curr();
            const GPoint prev = click->prev();
            fShape->setRect(offset(fShape->getRect(), curr.x() - prev.x(), curr.y() - prev.y()));
        } else if (click->isName("resize")) {
            fShape->setRect(((ResizeClick*)click)->makeRect());
        } else {
            fShape->setRect(make_from_pts(click->orig(), click->curr()));
        }
        if (NULL != fShape && GClick::kUp_State == click->state()) {
            if (fShape->getRect().isEmpty()) {
                this->removeShape(fShape);
                fShape = NULL;
            }
        }
        this->requestDraw();
    }

private:
    void removeShape(Shape* target) {
        GASSERT(target);

        std::vector<Shape*>::iterator it = std::find(fList.begin(), fList.end(), target);
        if (it != fList.end()) {
            fList.erase(it);
        } else {
            GASSERT(!"shape not found?");
        }
    }

    void updateTitle() {
        char buffer[100];
        buffer[0] = ' ';
        buffer[1] = 0;
        if (fShape) {
            const GColor& c = fShape->getColor();
            sprintf(buffer, "%02X %02X %02X %02X",
                    int(c.fA * 255), int(c.fR * 255), int(c.fG * 255), int(c.fB * 255));
        }
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow(640, 480);

    return wind->run();
}

