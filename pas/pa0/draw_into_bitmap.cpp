/*
 *  Copyright 2015 Matt Sarett
 */

#include "GBitmap.h"
#include "GPixel.h"

extern void cs575_draw_into_bitmap(const GBitmap&);

int min(int a, int b) {
    return a < b ? a : b;
}

/*
 * Detects what percentage of a pixel falls within an ellipse
 *
 * @param x  x-coordinate of the pixel
 * @param y  y-coordinate of the pixel
 * @param rx x-radius of the ellipse
 * @param ry y-radius of the ellipse
 * @return   the percentage of the pixel in the ellipse, where 1.0
 *           represents a pixel fully in the ellipse, and 0.0
 *           represents a pixel fully outside the ellipse
 */
inline float inEllipse(float x, float y, float rx, float ry) {
    // Common sub-expressions
    const float rx2 = rx * rx;
    const float ry2 = ry * ry;

    // Calculate the distance of the input pixel from the origin
    const float d = (x * x * ry2) + (y * y * rx2);

    // Calculate the distance of the edge of the ellipse from the origin
    const float edge = rx2 * ry2;

    if (d < edge - 1.0f) {
        return 1.0f;
    } else if (d < edge) {
        return edge - d;
    } else {
        return 0.0f;
    }
}

/*
 * Draws a quarter of an ellipse in each of the four corners of the bitmap
 *
 * @param bitmap the bitmap that we will draw into
 */
void drawEllipses(const GBitmap& bitmap) {
    // Set color components for tar heel blue
    const float baseRed = 86.0f;
    const float baseGreen = 160.0f;
    const float baseBlue = 211.0f;

    // Get bitmap properties
    const int width = bitmap.width();
    const int height = bitmap.height();
    const size_t rowBytes = bitmap.rowBytes();
    GPixel* pixels = bitmap.pixels();
    const float rx = ((float) width) / 2.0f;
    const float ry = ((float) height) / 2.0f;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Determine what portion of each pixel is in each ellipse
            const float p0 = inEllipse(x, y, rx, ry);
            const float p1 = inEllipse(width - x - 1, y, rx, ry);
            const float p2 = inEllipse(x, height - y - 1, rx, ry);
            const float p3 = inEllipse(width - x - 1, height - y - 1, rx, ry);
            // Given our choice of rx and ry, p should not exceed 1.0f
            const float p = p0 + p1 + p2 + p3;

            // Blend tar heel blue with white, based on value of p
            const float inv = 1.0f - p;
            uint8_t red = (uint8_t) (baseRed + inv * (255.0f - baseRed));
            uint8_t green = (uint8_t) (baseGreen + inv * (255.0f - baseGreen));
            uint8_t blue = (uint8_t) (baseBlue + inv * (255.0f - baseBlue));
            uint8_t alpha = 0xFF;
            pixels[x] = GPixel_PackARGB(alpha, red, green, blue);
        }
        pixels = (GPixel*) (((uint8_t*) pixels) + rowBytes);
    }
}

/*
 * Draws a semi-transparent overlay on the bitmap
 * The overlay becomes more transparent as we move farther from the center
 * Assumes the input bitmap is opaque
 *
 * @param bitmap the bitmap that we will draw into
 */
void drawOverlay(const GBitmap& bitmap) {
    // Set color components for gold
    const float newRed = 255.0f;
    const float newGreen = 215.0f;
    const float newBlue = 0.0f;

    // Get bitmap properties
    const int width = bitmap.width();
    const int height = bitmap.height();
    const size_t rowBytes = bitmap.rowBytes();
    GPixel* pixels = bitmap.pixels();
    const float divisor = (float) (width / 2) * (width / 2);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate a weight based on the distance from the edge of the image
            int dx = min(x, width - x - 1);
            int dy = min(y, height -  y - 1);
            float d = dx * dx + dy * dy;
            float w = d / divisor;

            // Use a weighted average to blend with the original pixel
            GPixel pixel = pixels[x];
            float oldRed = (float) GPixel_GetR(pixel);
            float oldGreen = (float) GPixel_GetG(pixel);
            float oldBlue = (float) GPixel_GetB(pixel);
            uint8_t red = (uint8_t) w * newRed + (1.0f - w) * oldRed;
            uint8_t green = (uint8_t) w * newGreen + (1.0f - w) * oldGreen;
            uint8_t blue = (uint8_t) w * newBlue + (1.0f - w) * oldBlue;
            pixels[x] = GPixel_PackARGB(0xFF, red, green, blue);
        }
        pixels = (GPixel*) (((uint8_t*) pixels) + rowBytes);
    }
}

/*
 * Draws a few partial ellipses beneath another layer
 *
 * @param bitmap the bitmap that we will draw into
 */
void cs575_draw_into_bitmap(const GBitmap& bitmap) {
    if (bitmap.width() <= 0 || bitmap.height() <= 0 || bitmap.rowBytes() == 0 ||
            bitmap.pixels() == NULL) {
        return;
    }

    drawEllipses(bitmap);
    //drawOverlay(bitmap);
}
