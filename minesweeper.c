#include <stdint.h>

#define LightGray 255
#define Gray 192
#define DarkGray 128

#define Tau 6.28318530717958647692528676655900576839433879875021f
#define Pi 0.5f*Tau
#define HalfPi 0.25f*Tau

extern float cosf(float);
extern float roundf(float);
extern void logu64(uint64_t);

uint32_t DrawBorder(uint8_t *Backbuffer, uint32_t Stride, uint32_t Width, uint32_t BorderWidth, uint32_t x, uint32_t y, uint32_t colorTopLeft, uint32_t colorCenter, uint32_t colorBottomRight)
{
    uint32_t row = y * Stride + x * 4;
    uint32_t fullW = Width + 2*BorderWidth;
    for (uint32_t j = 0; j < BorderWidth; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < fullW - j - 1; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = 255;
        for (i = 0; i < j; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
    for (uint32_t j = 0; j < Width; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < BorderWidth; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        at += Width * 4;
        for (i = 0; i < BorderWidth; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
    for (uint32_t j = 0; j < BorderWidth; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < BorderWidth - j - 1; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = 255;
        for (i = 0; i < Width + BorderWidth + j; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
    return BorderWidth;
}

void DrawSquare(uint8_t *Backbuffer, uint32_t Stride, uint32_t Width, uint32_t BorderWidth, uint32_t x, uint32_t y) {
    uint32_t colorTopLeft = LightGray;
    uint32_t colorCenter = Gray;
    uint32_t colorBottomRight = DarkGray;
    uint32_t row = y * Stride + x * 4;
    uint32_t fullW = Width + 2*BorderWidth;
    for (uint32_t j = 0; j < BorderWidth; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < fullW - j - 1; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = 255;
        for (i = 0; i < j; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
    for (uint32_t j = 0; j < Width; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < BorderWidth; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        for (i = 0; i < Width; ++i) {
            Backbuffer[at++] = colorCenter;
            Backbuffer[at++] = colorCenter;
            Backbuffer[at++] = colorCenter;
            Backbuffer[at++] = 255;
        }
        for (i = 0; i < BorderWidth; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
    for (uint32_t j = 0; j < BorderWidth; ++j) {
        uint32_t at = row;
        uint32_t i;
        for (i = 0; i < BorderWidth - j - 1; ++i) {
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = colorTopLeft;
            Backbuffer[at++] = 255;
        }
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = colorCenter;
        Backbuffer[at++] = 255;
        for (i = 0; i < Width + BorderWidth + j; ++i) {
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = colorBottomRight;
            Backbuffer[at++] = 255;
        }
        row += Stride;
    }
}

float Fmod(float a, float b) {
    float q = a / b;
    return q - (uint32_t)q;
}

void GameUpdateAndRender(float ElapsedSeconds, uint32_t Width, uint32_t Height, uint8_t *Backbuffer)
{
    uint32_t Stride = Width*4;
    uint8_t *At = Backbuffer;
    for (uint32_t y = 0; y < Height; ++y) {
        for (uint32_t x = 0; x < Width; ++x) {
//            *At++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (float)x / (float)Width)));
//            *At++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (1 - (float)y / (float)Height))));
//            *At++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (1 - (float)x / (float)Width))));
            *At++ = (uint8_t)(255.0f * (ElapsedSeconds + Fmod(x, (float)Width) + 0.5f));
            *At++ = (uint8_t)(255.0f * (ElapsedSeconds - Fmod(y, (float)Height) + 0.5f));
            *At++ = (uint8_t)(255.0f * (ElapsedSeconds - Fmod(x, (float)Width) + 0.5f));
            *At++ = 0xFF;
        }
    }
#if 1
    uint32_t borderWidth = 2;
    uint32_t innerWidth = 12;
    uint32_t boardWidth = 9;
    uint32_t boardHeight = 9;
    uint32_t squareWidth = innerWidth + 2*borderWidth;
    uint32_t indent = DrawBorder(Backbuffer, Stride, squareWidth * 9 + 6, 6, 0, 0, Gray, Gray, Gray);
    indent += DrawBorder(Backbuffer, Stride, squareWidth * 9, 3, indent, indent, DarkGray, Gray, LightGray);

    for (uint32_t j = 0; j < boardHeight; ++j) {
        for (uint32_t i = 0; i < boardWidth; ++i) {
            DrawSquare(Backbuffer, Stride, innerWidth, borderWidth, indent + i * squareWidth, indent + j * squareWidth);
        }
    }
#endif
}
