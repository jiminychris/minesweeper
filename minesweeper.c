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

#pragma pack(push, 1)
struct image_header
{
    uint32_t Width;
    uint32_t Height;
};
#pragma pack(pop)

#define Min(A, B) (A < B ? A : B)
#define Max(A, B) (A < B ? B : A)

void GameUpdateAndRender(float ElapsedSeconds, uint32_t Width, uint32_t Height, uint8_t *Backbuffer, uint8_t *AssetsMemory)
{
    struct image_header *Header = (struct image_header *)AssetsMemory;
    uint32_t DestStride = Width*4;

    float VMultiplier = 1.0f / (float)Height;
    float UMultiplier = 1.0f / (float)Width;
    float VOffset = 0.0f;
    float UOffset = 0.0f;

    if (Width * Header->Height < Header->Width * Height) {
        UMultiplier = (float)Header->Height / (float)Header->Width / (float)Height;
        UOffset = 0.5f * (1 - UMultiplier * Width);
    } else if (Width * Header->Height > Header->Width * Height) {
        VMultiplier = (float)Header->Width / (float)Width / (float)Header->Height;
        VOffset = 0.5f * (1 - VMultiplier * Height);
    }
    
    
    uint8_t *DestRow = Backbuffer;
    uint8_t *Background = AssetsMemory + sizeof(*Header);
    for (uint32_t y = 0; y < Height; ++y) {
        float V = ((float)y + 0.5f) * VMultiplier + VOffset;
        uint8_t *Dest = DestRow;
        for (uint32_t x = 0; x < Width; ++x) {
            float U = ((float)x + 0.5f) * UMultiplier + UOffset;
            uint32_t SourceX = (uint32_t)(U * Header->Width);
            uint32_t SourceY = (uint32_t)(V * Header->Height);
            uint8_t *Source = Background + (SourceY * Header->Width + SourceX) * 3;
#if 1
            *Dest++ = *Source++;
            *Dest++ = *Source++;
            *Dest++ = *Source++;
#endif
#if 0
            *Dest++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (float)x / (float)Width)));
            *Dest++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (1 - (float)y / (float)Height))));
            *Dest++ = roundf(127.5 * (1 - cosf(ElapsedSeconds + Pi * (1 - (float)x / (float)Width))));
#endif
#if 0
            *Dest++ = (uint8_t)(255.0f * (ElapsedSeconds + Fmod(x, (float)Width) + 0.5f));
            *Dest++ = (uint8_t)(255.0f * (ElapsedSeconds - Fmod(y, (float)Height) + 0.5f));
            *Dest++ = (uint8_t)(255.0f * (ElapsedSeconds - Fmod(x, (float)Width) + 0.5f));
#endif
            *Dest++ = 0xFF;
        }
        DestRow += DestStride;
    }
#if 1
    uint32_t borderWidth = 2;
    uint32_t innerWidth = 12;
    uint32_t boardWidth = 9;
    uint32_t boardHeight = 9;
    uint32_t squareWidth = innerWidth + 2*borderWidth;
    uint32_t indent = DrawBorder(Backbuffer, DestStride, squareWidth * 9 + 6, 6, 0, 0, Gray, Gray, Gray);
    indent += DrawBorder(Backbuffer, DestStride, squareWidth * 9, 3, indent, indent, DarkGray, Gray, LightGray);

    for (uint32_t j = 0; j < boardHeight; ++j) {
        for (uint32_t i = 0; i < boardWidth; ++i) {
            DrawSquare(Backbuffer, DestStride, innerWidth, borderWidth, indent + i * squareWidth, indent + j * squareWidth);
        }
    }
#endif
}
