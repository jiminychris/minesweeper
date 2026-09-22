#include <stddef.h>
#include <stdint.h>

#define White 255
#define Gray 192
#define DarkGray 128
#define Black 0

#define Tau 6.28318530717958647692528676655900576839433879875021f
#define Pi 0.5f*Tau
#define HalfPi 0.25f*Tau

#define Min(A, B) ((A) < (B) ? (A) : (B))
#define Max(A, B) ((A) < (B) ? (B) : (A))
#define Clamp(A, X, B) (Max(A, Min(X, B)))

extern void logu64(uint64_t);

struct game_button
{
    uint32_t EndedDown;
    uint32_t HalfTransitionCount;
};

struct v2i
{
    union
    {
        struct
        {
            int32_t X;
            int32_t Y;
        };
        struct
        {
            int32_t Width;
            int32_t Height;
        };
    };
};

struct v2i V2i(int32_t X, int32_t Y)
{
    struct v2i Result = {X, Y};
    return Result;
}

struct v4
{
    union
    {
        struct
        {
            float X;
            float Y;
            float Z;
            float W;
        };
        struct
        {
            float Red;
            float Green;
            float Blue;
            float Alpha;
        };
    };
};

struct backbuffer
{
    struct v2i Dimensions;
    uint8_t *Memory;
};

struct rect2i
{
    struct v2i Position;
    struct v2i Dimensions;
};

uint32_t RectangleContains(struct rect2i Rectangle, struct v2i Position)
{
    int32_t Left = Rectangle.Position.X;
    int32_t Right = Rectangle.Position.X + Rectangle.Dimensions.Width;
    int32_t Top = Rectangle.Position.Y;
    int32_t Bottom = Rectangle.Position.Y + Rectangle.Dimensions.Height;
    return Left <= Position.X && Position.X <= Right && Top <= Position.Y && Position.Y <= Bottom;
}

enum draw_flags
{
    draw_flags_None = 0,
    draw_flags_Measure = 1 << 0,
};

struct rect2i DrawRectangle(struct backbuffer *Backbuffer, int32_t Stride, struct rect2i Rectangle, struct v4 Color, enum draw_flags Flags)
{
    struct v2i Dimensions = Rectangle.Dimensions;
    struct v2i Position = Rectangle.Position;
    int32_t StartX = Max(0, Position.X);
    int32_t StartY = Max(0, Position.Y);
    int32_t StopX = Min(Position.X + (int32_t)Dimensions.Width, (int32_t)Backbuffer->Dimensions.Width);
    int32_t StopY = Min(Position.Y + (int32_t)Dimensions.Height, (int32_t)Backbuffer->Dimensions.Height);
    struct rect2i Result = {Position, V2i(StopX - Position.X, StopY - Position.Y)};
    if (!(Flags & draw_flags_Measure))
    {
        uint8_t *Row = Backbuffer->Memory + Stride * StartY + StartX * 4;
        uint32_t PackedColor = 
            ((uint8_t)(255.0f * Clamp(0.0f, Color.Red,   1.0f)) << 0)
            | ((uint8_t)(255.0f * Clamp(0.0f, Color.Green, 1.0f)) << 8)
            | ((uint8_t)(255.0f * Clamp(0.0f, Color.Blue,  1.0f)) << 16)
            | ((uint8_t)(255.0f * Clamp(0.0f, Color.Alpha, 1.0f)) << 24);
    
        for (int32_t IndexY = StartY; IndexY < StopY; IndexY++)
        {
            uint32_t *At = (uint32_t*)Row;
            for (int32_t IndexX = StartX; IndexX < StopX; IndexX++)
            {
                *At++ = PackedColor;
            }
            Row += Stride;
        }
    }
    
    return Result;
}

int32_t DrawBorder(struct backbuffer *Backbuffer, int32_t Stride, struct v2i Dimensions, int32_t BorderWidth, struct v2i Position, int32_t colorTopLeft, int32_t colorCenter, int32_t colorBottomRight)
{
    int32_t FullWidth = Dimensions.Width + 2*BorderWidth;
    int32_t FullHeight = Dimensions.Height + 2*BorderWidth;
    int32_t StartX = Max(0, Position.X);
    int32_t StartY = Max(0, Position.Y);
    int32_t StopTopY = Min(Position.Y + (int32_t)BorderWidth, (int32_t)Backbuffer->Dimensions.Height);
    int32_t StopMidY = Min(Position.Y + (int32_t)BorderWidth + Dimensions.Height, (int32_t)Backbuffer->Dimensions.Height);
    int32_t StopX = Min(Position.X + (int32_t)FullWidth, (int32_t)Backbuffer->Dimensions.Width);
    int32_t StopY = Min(Position.Y + (int32_t)FullHeight, (int32_t)Backbuffer->Dimensions.Height);
    int32_t StopLeftX = Min(Position.X + (int32_t)BorderWidth, (int32_t)Backbuffer->Dimensions.Width);
    int32_t StartRightX = Max((int32_t)0, Position.X + BorderWidth + Dimensions.Width);
    if (StartRightX < 0)
    {
        logu64(Position.X + BorderWidth + Dimensions.Width);
        logu64(StartRightX);
    }
    uint8_t *Row = Backbuffer->Memory + Stride * StartY + StartX * 4;
    for (int32_t IndexY = StartY; IndexY < StopTopY; IndexY++) {
        uint8_t *At = Row;
        int32_t IndexX;
        int32_t StopTopLeft = Min(Position.X + FullWidth - IndexY + StartY - 1, StopX);
        for (IndexX = StartX; IndexX < StopTopLeft; IndexX++) {
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = 255;
        }
        int32_t StopMid = Min(Position.X + FullWidth - IndexY + StartY, StopX);
        for (; IndexX < StopMid; IndexX++)
        {
            *At++ = colorCenter;
            *At++ = colorCenter;
            *At++ = colorCenter;
            *At++ = 255;
        }
        for (; IndexX < StopX; IndexX++) {
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = 255;
        }
        Row += Stride;
    }
    for (int32_t IndexY = Max(0, Position.Y + BorderWidth); IndexY < StopMidY; IndexY++) {
        uint8_t *At = Row;
        int32_t IndexX;
        for (IndexX = StartX; IndexX < StopLeftX; IndexX++) {
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = 255;
        }
        At = Backbuffer->Memory + Stride * IndexY + StartRightX * 4;
        for (IndexX = StartRightX; IndexX < StopX; IndexX++) {
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = 255;
        }
        Row += Stride;
    }
    StartY = Max(0, Position.Y + BorderWidth + Dimensions.Height);
    for (int32_t IndexY = StartY; IndexY < StopY; IndexY++) {
        uint8_t *At = Row;
        int32_t IndexX;
        int32_t StopBottomLeft = Min(Position.X + BorderWidth - IndexY + StartY - 1, StopX);
        for (IndexX = StartX; IndexX < StopBottomLeft; IndexX++) {
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = 255;
        }
        int32_t StopMid = Min(Position.X + BorderWidth - IndexY + StartY, StopX);
        for (; IndexX < StopMid; IndexX++)
        {
            *At++ = colorCenter;
            *At++ = colorCenter;
            *At++ = colorCenter;
            *At++ = 255;
        }
        for (; IndexX < StopX; IndexX++) {
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = 255;
        }
        Row += Stride;
    }
    return BorderWidth;
}

void DrawTile(struct backbuffer *Backbuffer, int32_t Stride, int32_t Width, int32_t BorderWidth, struct v2i Position) {
    struct rect2i Rectangle;
    Rectangle.Dimensions.Width = Width;
    Rectangle.Dimensions.Height = Width;
    Rectangle.Position.X = Position.X + BorderWidth;
    Rectangle.Position.Y = Position.Y + BorderWidth;
    DrawBorder(Backbuffer, Stride, Rectangle.Dimensions, BorderWidth, Position, White, Gray, DarkGray);
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    DrawRectangle(Backbuffer, Stride, Rectangle, GrayVector, draw_flags_None);
}

float Fmod(float a, float b) {
    float q = a / b;
    return q - (uint32_t)q;
}

#pragma pack(push, 1)
struct image_header
{
    int32_t Width;
    int32_t Height;
};
#pragma pack(pop)

struct game_state
{
    uint32_t Initialized;
    struct v2i WindowPosition;
    struct v2i DragOffset;
    struct v2i *DragTarget;
};

void GameUpdateAndRender(float ElapsedSeconds, int32_t Width, int32_t Height, uint8_t *BackbufferMemory, int32_t MouseEndedDown, int32_t MouseHalfTransitionCount, int32_t MouseX, int32_t MouseY, uint8_t *AssetsMemory, size_t GameMemorySize, uint8_t *GameMemory)
{
    struct game_state *GameState = (struct game_state*)GameMemory;

    if (!GameState->Initialized)
    {
        GameState->Initialized = 1;
        GameState->WindowPosition.X = 100;
        GameState->WindowPosition.Y = 100;
    }

    if (GameState->DragTarget)
    {
        if (MouseEndedDown)
        {
            GameState->DragTarget->X = (int32_t)MouseX + GameState->DragOffset.X;
            GameState->DragTarget->Y = (int32_t)MouseY + GameState->DragOffset.Y;
        }
        else
        {
            GameState->DragTarget = 0;
        }
    }
    
    struct v2i MousePosition = {MouseX, MouseY};
    struct image_header *Header = (struct image_header *)AssetsMemory;
    int32_t DestStride = Width*4;

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
    
    
    uint8_t *DestRow = BackbufferMemory;
    uint8_t *Background = AssetsMemory + sizeof(*Header);
    for (int32_t y = 0; y < Height; ++y) {
        float V = ((float)y + 0.5f) * VMultiplier + VOffset;
        uint8_t *Dest = DestRow;
        for (int32_t x = 0; x < Width; ++x) {
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
    int32_t borderWidth = 2;
    int32_t innerWidth = 12;
    int32_t boardWidth = 9;
    int32_t boardHeight = 9;
    int32_t squareWidth = innerWidth + 2*borderWidth;
    int32_t gameWidthInPixels = squareWidth * 9 + 6;

    struct backbuffer _Backbuffer;
    _Backbuffer.Dimensions.Width = Width;
    _Backbuffer.Dimensions.Height = Height;
    _Backbuffer.Memory = BackbufferMemory;
    struct backbuffer *Backbuffer = &_Backbuffer;
    
    struct v2i Position = GameState->WindowPosition;
    int32_t indent = DrawBorder(Backbuffer, DestStride, V2i(gameWidthInPixels + 12, gameWidthInPixels + 12 + 18), 1, Position, Black, Black, Black);
#if 1
    Position.X += indent;
    Position.Y += indent;
    struct rect2i TitleBarRectangle;
    TitleBarRectangle.Dimensions = V2i(gameWidthInPixels + 12, 18);
    TitleBarRectangle.Position = Position;
    struct v4 TitleBarColor = { 0.0f, 0.0f, 1.0f, 1.0f };
    if (RectangleContains(TitleBarRectangle, MousePosition))
    {
        if (MouseEndedDown && (MouseHalfTransitionCount & 1))
        {
            GameState->DragOffset.X = GameState->WindowPosition.X - MouseX;
            GameState->DragOffset.Y = GameState->WindowPosition.Y - MouseY;
            GameState->DragTarget = &GameState->WindowPosition;
        }
    }
    DrawRectangle(Backbuffer, DestStride, TitleBarRectangle, TitleBarColor, draw_flags_None);
    Position.Y += 18;
    indent = DrawBorder(Backbuffer, DestStride, V2i(gameWidthInPixels, gameWidthInPixels), 6, Position, Gray, Gray, Gray);
    Position.X += indent;
    Position.Y += indent;
    indent = DrawBorder(Backbuffer, DestStride, V2i(squareWidth * 9, squareWidth * 9), 3, Position, DarkGray, Gray, White);
    Position.X += indent;
    Position.Y += indent;

    for (int32_t j = 0; j < boardHeight; ++j) {
        for (int32_t i = 0; i < boardWidth; ++i) {
            struct v2i TilePosition = {Position.X + i * squareWidth, Position.Y + j * squareWidth};
            DrawTile(Backbuffer, DestStride, innerWidth, borderWidth, TilePosition);
        }
    }
#endif
}
