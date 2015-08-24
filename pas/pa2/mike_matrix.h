class GMatrix {
public:
    GMatrix() { this->setIdentity(); }

    float scaleX() const { return fMat[0][0]; }
    float skewY() const { return fMat[0][1]; }

    void setIdentity() {
        fMat[0][0] = fMat[1][1] = 1;
        fMat[0][1] = fMat[1][0] = fMat[2][0] = fMat[2][1] = 0;
    }

    void preTranslate(float x, float y) {
        fMat[2][0] += fMat[0][0] * x + fMat[1][0] * y;
        fMat[2][1] += fMat[0][1] * x + fMat[1][1] * y;
    }
    
    void preScale(float x, float y) {
        fMat[0][0] *= x;
        fMat[0][1] *= x;
        fMat[1][0] *= y;
        fMat[1][1] *= y;
    }
    
    void setRectToRect(const GRect& src, const GRect& dst) {
        this->preTranslate(dst.x(), dst.y());
        this->preScale(dst.width() / src.width(), dst.height() / src.height());
        this->preTranslate(-src.x(), -src.y());
    }

    void mapXY(float x, float y, float result[2]) const {
        result[0] = fMat[0][0] * x + fMat[1][0] * y + fMat[2][0];
        result[1] = fMat[0][1] * x + fMat[1][1] * y + fMat[2][1];
    }

private:
    float fMat[3][2];
};

