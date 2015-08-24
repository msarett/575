/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GMatrix_DEFINED
#define GMatrix_DEFINED

#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"

class GMatrix {
public:
    GMatrix() { this->setIdentity(); }

    enum TypeMask {
        kIdentity_TypeMask  = 0,
        kTranslate_TypeMask = 1 << 0,   //!< set if either translate field is non-zero
        kScale_TypeMask     = 1 << 1,   //!< set if either scale field is != 1
        kAffine_TypeMask    = 1 << 2,   //!< set if the matrix will rotate or skew
    };
    
    /**
     *  Returns a bit-field describing the complexity of the matrix.
     *
     *  If the matrix is identity, then this returns 0.
     */
    TypeMask getType() const;

    /**
     *  Set this matrix to identity.
     */
    void setIdentity();

    /**
     *  Set this matrix to translate by the specified amounts.
     */
    void setTranslate(float tx, float ty);

    /**
     *  Set this matrix to scale by the specified amounts.
     */
    void setScale(float sx, float sy);

    /**
     *  Set this matrix to rotate by the specified amounts.
     *
     *  Note: since positive-Y goes down, a small angle of rotation will increase Y.
     */
    void setRotate(float radians);

    /**
     *  Set this matrix to the concatenation of the two specified matrices, such that the resulting
     *  matrix, when applied to points (e.g. mapPoints) will have the same effect as first applying
     *  the primo matrix to the points, and then applying the secundo matrix to the resulting
     *  points.
     *
     *  After setConcat...
     *      this->mapPoints(dst, src, N)
     *  will be the same as
     *      primo.mapPoints(tmp, src, N)
     *      secundo.mapPoints(dst, tmp, N)
     */
    void setConcat(const GMatrix& secundo, const GMatrix& primo);

    /**
     *  Transform the set of points in src, storing the resulting points in dst, by applying this
     *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
     *
     *  Note: It is legal for src and dst to point to the same memory (however, they may not
     *  partially overlap). Thus the following is supported.
     *
     *  GPoint pts[] = { ... };
     *  matrix.mapPoints(pts, pts, count);
     */
    void mapPoints(GPoint dst[], const GPoint src[], int count) const;

    /**
     *  Transform the specified rect by this matrix, returning a new rectangle that is the bounds
     *  of the transformed vertices (corners) of the src rectangle.
     *
     *  The returned rectangle will always have its sides "sorted". i.e. it will always return a
     *  rectangle whose fLeft <= fRight, and fTop <= fBottom, regardless of the values in the
     *  matrix.
     */
    GRect mapRect(const GRect&) const;

    /**
     *  If this matrix can be inverted (resulting in a non-singular matrix), return true and return
     *  that inverse in the parameter. If this matrix cannot be inverted, return false and the
     *  inverse parameter is left in an undefined state.
     */
    bool invert(GMatrix* inverse) const;

    // Helper methods

    void preConcat(const GMatrix& primo) {
        this->setConcat(*this, primo);
    }

    void preTranslate(float x, float y) {
        GMatrix trans;
        trans.setTranslate(x, y);
        this->preConcat(trans);
    }

    void preScale(float sx, float sy) {
        GMatrix scale;
        scale.setScale(sx, sy);
        this->preConcat(scale);
    }

    void preRotate(float radians) {
        GMatrix rotate;
        rotate.setRotate(radians);
        this->preConcat(rotate);
    }

    void mapPoints(GPoint pts[], int count) const {
        this->mapPoints(pts, pts, count);
    }

    GPoint mapXY(float x, float y) const {
        GPoint dst, src = { x, y };
        this->mapPoints(&dst, &src, 1);
        return dst;
    }

    float operator[](int index) const {
        GASSERT((unsigned)index < 6);
        return fMat[index];
    }

private:
    float fMat[6];
};

#endif
