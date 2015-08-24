/*
 *  Copyright 2015 Mike Reed
 */

#include "GMatrix.h"
#include "mike_utils.h"

/*
    [ a b c ]
    [ d e f ] = [ a d b e c f ]
    [ 0 0 1 ]
*/

GMatrix::TypeMask GMatrix::getType() const {
    unsigned mask = 0;
    if (fMat[0] != 1 || fMat[3] != 1) {
        mask |= GMatrix::kScale_TypeMask;
    }
    if (fMat[1] != 0 || fMat[2] != 0) {
        mask |= GMatrix::kAffine_TypeMask;
    }
    if (fMat[4] != 0 || fMat[5] != 0) {
        mask |= GMatrix::kTranslate_TypeMask;
    }
    return (GMatrix::TypeMask)mask;
}

void GMatrix::setIdentity() {
    fMat[0] = fMat[3] = 1;
    fMat[1] = fMat[2] = fMat[4] = fMat[5] = 0;
}

void GMatrix::setTranslate(float tx, float ty) {
    fMat[0] = fMat[3] = 1;
    fMat[1] = fMat[2] = 0;
    fMat[4] = tx;
    fMat[5] = ty;
}

void GMatrix::setScale(float sx, float sy) {
    fMat[0] = sx;
    fMat[3] = sy;
    fMat[1] = fMat[2] = fMat[4] = fMat[5] = 0;
}

void GMatrix::setRotate(float radians) {
    const float s = sinf(radians);
    const float c = cosf(radians);

    fMat[0] = fMat[3] = c;
    fMat[1] = s;
    fMat[2] = -s;
    fMat[4] = fMat[5] = 0;
}

void GMatrix::setConcat(const GMatrix& A, const GMatrix& B) {
    const float a = A[0] * B[0] + A[2] * B[1];
    const float b = A[0] * B[2] + A[2] * B[3];
    const float c = A[0] * B[4] + A[2] * B[5] + A[4];

    const float d = A[1] * B[0] + A[3] * B[1];
    const float e = A[1] * B[2] + A[3] * B[3];
    const float f = A[1] * B[4] + A[3] * B[5] + A[5];

    fMat[0] = a; fMat[2] = b; fMat[4] = c;
    fMat[1] = d; fMat[3] = e; fMat[5] = f;
}

static double dcross(double a, double b, double c, double d) {
    return a * b - c * d;
}

bool GMatrix::invert(GMatrix* inverse) const {
    double det = dcross(fMat[0], fMat[3], fMat[1], fMat[2]);
    if (0 == det) {
        return false;
    }
    double idet = 1 / det;

    float a =  fMat[3] * idet;
    float b = -fMat[2] * idet;
    float c = dcross(fMat[2], fMat[5], fMat[3], fMat[4]) * idet;

    float d = -fMat[1] * idet;
    float e =  fMat[0] * idet;
    float f = dcross(fMat[1], fMat[4], fMat[0], fMat[5]) * idet;

    inverse->fMat[0] = a; inverse->fMat[2] = b; inverse->fMat[4] = c;
    inverse->fMat[1] = d; inverse->fMat[3] = e; inverse->fMat[5] = f;
    return true;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float x = fMat[0] * src[i].fX + fMat[2] * src[i].fY + fMat[4];
        float y = fMat[1] * src[i].fX + fMat[3] * src[i].fY + fMat[5];
        dst[i].set(x, y);
    }
}

GPoint* to_quad(const GRect& r, GPoint quad[4]) {
    quad[0].set(r.left(),  r.top());
    quad[1].set(r.right(), r.top());
    quad[2].set(r.right(), r.bottom());
    quad[3].set(r.left(),  r.bottom());
    return quad;
}

GRect GMatrix::mapRect(const GRect& src) const {
    GPoint quad[4];
    this->mapPoints(to_quad(src, quad), 4);
    return compute_bounds(quad, 4);
}

void set_rect_to_rect(GMatrix* matrix, const GRect& src, const GRect& dst) {
    matrix->setTranslate(dst.x(), dst.y());
    matrix->preScale(dst.width() / src.width(), dst.height() / src.height());
    matrix->preTranslate(-src.x(), -src.y());
}

