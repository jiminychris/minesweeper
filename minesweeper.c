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

#define InvalidCodePath Assert(!"Invalid code path");
#define InvalidDefaultCase default: Assert(!"Invalid default case"); break;
#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
#define OffsetOf(type, Member) ((size_t)&(((type *)0)->Member))
#define NotImplemented Assert(!"Not implemented");

#define Kilobytes(Value) ((u64)(Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

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

#include "bitmaps.h"

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

struct rect2i Rect2i(struct v2i Position, struct v2i Dimensions)
{
    struct rect2i Result = {Position, Dimensions};
    return Result;
}

uint32_t RectangleContains(struct rect2i Rectangle, struct v2i Position)
{
    int32_t Left = Rectangle.Position.X;
    int32_t Right = Rectangle.Position.X + Rectangle.Dimensions.Width;
    int32_t Top = Rectangle.Position.Y;
    int32_t Bottom = Rectangle.Position.Y + Rectangle.Dimensions.Height;
    // TODO: Is this < ... <= correct?
    return Left < Position.X && Position.X <= Right && Top < Position.Y && Position.Y <= Bottom;
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

enum smiley_state
{
    smiley_state_Normal,
    smiley_state_Surprised,
    smiley_state_Frowney,
    smiley_state_Cool,
};

void DrawSmiley(struct backbuffer *Backbuffer, struct v2i Position, enum smiley_state State)
{
    int32_t Stride = Backbuffer->Dimensions.Width * 4;
    uint8_t *DestRow = Backbuffer->Memory + Position.Y * Stride + Position.X * 4;
    uint8_t *SourceRow;
    switch (State)
    {
    case smiley_state_Surprised:
    {
        SourceRow = SurprisedBitmap;
    } break;
    case smiley_state_Cool:
    {
        SourceRow = CoolBitmap;
    } break;
    case smiley_state_Frowney:
    {
        SourceRow = FrowneyBitmap;
    } break;
    case smiley_state_Normal:
    default:
    {
        SourceRow = SmileyBitmap;
    } break;
    }
    int32_t StartY = Max(0, Position.Y);
    int32_t StartX = Max(0, Position.X);
    int32_t StopY = Min(Position.Y + SmileyDimensions.Height, Backbuffer->Dimensions.Height);
    int32_t StopX = Min(Position.X + SmileyDimensions.Width, Backbuffer->Dimensions.Width);
    for (int32_t IndexY = StartY; IndexY < StopY; IndexY++)
    {
        uint32_t *Dest = (uint32_t*)DestRow;
        uint8_t *Source = SourceRow;
        for (int32_t IndexX = StartX; IndexX < StopX; IndexX++)
        {
            uint32_t Color = SmileyColors[*Source++];
            if (Color)
            {
                *Dest = Color;
            }
            Dest++;;
        }
        DestRow += Stride;
        SourceRow += SmileyDimensions.Height;
    }
}

void DrawNumber(struct backbuffer *Backbuffer, struct v2i Position, int32_t Number)
{
    int32_t Stride = Backbuffer->Dimensions.Width * 4;
    
    Number = Clamp(0, Number, 999);
    int32_t RemainingDigits = 3;
    int32_t Digits[3];
    while (RemainingDigits--)
    {
        Digits[RemainingDigits] = Number % 10;
        Number = Number / 10;
    }
    RemainingDigits = 3;
    int32_t *AtDigit = Digits;
    while (RemainingDigits--)
    {
        uint8_t *DestRow = Backbuffer->Memory + Position.Y * Stride + Position.X * 4;
        int32_t Digit = *AtDigit++;
        char *SourceRow = NumberBlank;
        if (0 <= Digit && Digit < ArrayCount(NumberBitmaps))
        {
            SourceRow = NumberBitmaps[Digit];
        }
        int32_t StartY = Max(0, Position.Y);
        int32_t StartX = Max(0, Position.X);
        int32_t StopY = Min(Position.Y + NumberDimensions.Height, Backbuffer->Dimensions.Height);
        int32_t StopX = Min(Position.X + NumberDimensions.Width, Backbuffer->Dimensions.Width);
        for (int32_t IndexY = StartY; IndexY < StopY; IndexY++)
        {
            uint32_t *Dest = (uint32_t*)DestRow;
            char *Source = SourceRow;
            for (int32_t IndexX = StartX; IndexX < StopX; IndexX++)
            {
                uint32_t Color = NumberColors[*Source++ - '0'];
                if (Color)
                {
                    *Dest = Color;
                }
                Dest++;
            }
            DestRow += Stride;
            SourceRow += NumberDimensions.Width;
        }

        Position.X += NumberDimensions.X;
    }
}

enum tile_state
{
    tile_state_Normal,
    tile_state_Depressed,
};

    struct rect2i DrawTile(struct backbuffer *Backbuffer, int32_t Stride, int32_t Width, int32_t BorderWidth, struct v2i Position, enum tile_state State)
{
    struct rect2i Result = {Position, {Width + BorderWidth + BorderWidth, Width + BorderWidth + BorderWidth}};
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    switch (State)
    {
    case tile_state_Depressed:
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Result.Dimensions.Width - 2;
        Rectangle.Position.X = Position.X + 1;
        Rectangle.Position.Y = Position.Y + 1;
        DrawBorder(Backbuffer, Stride, Rectangle.Dimensions, 1, Position, DarkGray, DarkGray, DarkGray);
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Rectangle.Dimensions.Width + 1;
        DrawRectangle(Backbuffer, Stride, Rectangle, GrayVector, draw_flags_None);
    } break;
    case tile_state_Normal:
    default:
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Width;
        Rectangle.Position.X = Position.X + BorderWidth;
        Rectangle.Position.Y = Position.Y + BorderWidth;
        DrawBorder(Backbuffer, Stride, Rectangle.Dimensions, BorderWidth, Position, White, Gray, DarkGray);
        DrawRectangle(Backbuffer, Stride, Rectangle, GrayVector, draw_flags_None);
    } break;
    }

    return Result;
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
    uint32_t BoardDimensionsChoice;
    float GameStart;
    struct v2i WindowPosition;
    struct v2i DragOffset;
    struct v2i *DragTarget;
};

void GameUpdateAndRender(float ElapsedSeconds, int32_t Width, int32_t Height, uint8_t *BackbufferMemory, int32_t MouseEndedDown, int32_t MouseHalfTransitionCount, int32_t MouseX, int32_t MouseY, uint8_t *AssetsMemory, size_t GameMemorySize, uint8_t *GameMemory)
{
    struct game_state *GameState = (struct game_state*)GameMemory;

    struct backbuffer _Backbuffer;
    _Backbuffer.Dimensions.Width = Width;
    _Backbuffer.Dimensions.Height = Height;
    _Backbuffer.Memory = BackbufferMemory;
    struct backbuffer *Backbuffer = &_Backbuffer;

    struct v2i Beginner = {9,9};
    struct v2i Intermediate = {16,16};
    struct v2i Expert = {30,16};
    struct v2i BoardDimensionsOptions[] = {
        Beginner,
        Intermediate,
        Expert,
    };

    if (!GameState->Initialized)
    {
        GameState->Initialized = 1;
        GameState->WindowPosition.X = 100;
        GameState->WindowPosition.Y = 100;
        GameState->GameStart = ElapsedSeconds;
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

    struct rect2i ScreenRectangle = {0, 0, Width, Height};
    struct v4 BackgroundColor = {0, 0.5f, 0.5f, 1.0f};

    if (Header->Width && Header->Height)
    {
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
    }
    else
    {
        DrawRectangle(Backbuffer, DestStride, ScreenRectangle, BackgroundColor, draw_flags_None);
    }
    
    
    int32_t borderWidth = 2;
    int32_t innerWidth = 12;
    int32_t SquareWidth = innerWidth + 2*borderWidth;
    struct v2i BoardDimensions = Beginner;
    if (0 <= GameState->BoardDimensionsChoice && GameState->BoardDimensionsChoice < ArrayCount(BoardDimensionsOptions))
    {
        BoardDimensions = BoardDimensionsOptions[GameState->BoardDimensionsChoice];
    }
    struct v2i GameDimensionsInPixels = {SquareWidth * BoardDimensions.Width + 6, SquareWidth * BoardDimensions.Height + 6};
    struct v2i GameBackgroundDimensions = {GameDimensionsInPixels.Width + 12, GameDimensionsInPixels.Height + 12 + 43};
    struct v2i Position = GameState->WindowPosition;
    struct rect2i TitleBarRectangle;
    TitleBarRectangle.Dimensions = V2i(GameDimensionsInPixels.Width + 12 + 6, 18);
    TitleBarRectangle.Position = Position;
    struct rect2i SeparatorRectangle;
    SeparatorRectangle.Dimensions = V2i(TitleBarRectangle.Dimensions.Width, 1);
    struct v4 TitleBarColor = { 0.0f, 0.0f, 1.0f, 1.0f };
    struct v4 SeparatorColor = {0, 0, 0, 1};
    struct v4 MenuBarColor = {1, 1, 1, 1};
    if (RectangleContains(TitleBarRectangle, MousePosition))
    {
        if (MouseEndedDown && (MouseHalfTransitionCount & 1))
        {
            GameState->DragOffset.X = GameState->WindowPosition.X - MouseX;
            GameState->DragOffset.Y = GameState->WindowPosition.Y - MouseY;
            GameState->DragTarget = &GameState->WindowPosition;
        }
    }
    int32_t Indent = DrawBorder(Backbuffer, DestStride, V2i(GameBackgroundDimensions.Width + 6, GameBackgroundDimensions.Height + 6 + TitleBarRectangle.Dimensions.Height * 2 + SeparatorRectangle.Dimensions.Height * 2), 1, Position, Black, Black, Black);
#if 1
    Position.X += Indent;
    Position.Y += Indent;
    TitleBarRectangle.Position = Position;
    DrawRectangle(Backbuffer, DestStride, TitleBarRectangle, TitleBarColor, draw_flags_None);
    Position.Y += TitleBarRectangle.Dimensions.Height;
    SeparatorRectangle.Position = Position;
    DrawRectangle(Backbuffer, DestStride, SeparatorRectangle, SeparatorColor, draw_flags_None);
    Position.Y += SeparatorRectangle.Dimensions.Height;
    TitleBarRectangle.Position = Position;
    DrawRectangle(Backbuffer, DestStride, TitleBarRectangle, MenuBarColor, draw_flags_None);
    Position.Y += TitleBarRectangle.Dimensions.Height;
    SeparatorRectangle.Position = Position;
    DrawRectangle(Backbuffer, DestStride, SeparatorRectangle, SeparatorColor, draw_flags_None);
    Position.Y += SeparatorRectangle.Dimensions.Height;
    Indent = DrawBorder(Backbuffer, DestStride, V2i(GameBackgroundDimensions.Width, GameBackgroundDimensions.Height), 3, Position, White, Gray, DarkGray);
    Position.X += Indent;
    Position.Y += Indent;
    struct rect2i GameBackgroundRectangle = {Position, GameBackgroundDimensions};
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    DrawRectangle(Backbuffer, DestStride, GameBackgroundRectangle, GrayVector, draw_flags_None);
    Position.X += 6;
    Position.Y += 6;
    int32_t ScoreBackgroundWidth = GameBackgroundDimensions.Width - 16;
    Indent = DrawBorder(Backbuffer, DestStride, V2i(ScoreBackgroundWidth, 33), 2, Position, DarkGray, Gray, White);
    struct v2i ScoreOrigin = {Position.X + Indent, Position.Y + Indent};
    struct v2i ScorePosition = {ScoreOrigin.X + 5, ScoreOrigin.Y + 4};
    struct v2i ScoreDimensions = {39, 23};
    struct v4 ScoreBackgroundColor = {0,0,0,1};
    Indent = DrawBorder(Backbuffer, DestStride, ScoreDimensions, 1, ScorePosition, DarkGray, Gray, White);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    DrawRectangle(Backbuffer, DestStride, Rect2i(ScorePosition, ScoreDimensions), ScoreBackgroundColor, draw_flags_None);
    int32_t FlagsRemaining = 10;
    int32_t GameTimer = (int32_t)(ElapsedSeconds - GameState->GameStart);
    DrawNumber(Backbuffer, ScorePosition, FlagsRemaining);

    int32_t SmileyBorderWidthOuter = 1;
    int32_t SmileyBorderWidthInner = 2;
    struct v2i SmileyDimensionsInner = V2i(20, 20);
    struct v2i SmileyDimensionsOuter = V2i(SmileyDimensionsInner.X + SmileyBorderWidthInner*2, SmileyDimensionsInner.Y + SmileyBorderWidthInner*2);
    int32_t SmileyWidth = SmileyDimensionsOuter.X + SmileyBorderWidthOuter*2;
    ScorePosition = V2i(ScoreOrigin.X + (ScoreBackgroundWidth - SmileyWidth) / 2, ScoreOrigin.Y + 4);
    Indent = DrawBorder(Backbuffer, DestStride, SmileyDimensionsOuter, SmileyBorderWidthOuter, ScorePosition, DarkGray, Gray, DarkGray);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    Indent = DrawBorder(Backbuffer, DestStride, SmileyDimensionsInner, SmileyBorderWidthInner, ScorePosition, White, Gray, DarkGray);
    struct v2i SmileyPosition = V2i(ScorePosition.X + Indent + 2, ScorePosition.Y + Indent + 2);

    ScorePosition = V2i(ScoreOrigin.X + ScoreBackgroundWidth - 5 - 1 - ScoreDimensions.Width, ScoreOrigin.Y + 4);
    Indent = DrawBorder(Backbuffer, DestStride, ScoreDimensions, 1, ScorePosition, DarkGray, Gray, White);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    DrawRectangle(Backbuffer, DestStride, Rect2i(ScorePosition, ScoreDimensions), ScoreBackgroundColor, draw_flags_None);
    DrawNumber(Backbuffer, ScorePosition, GameTimer);
    Position.Y += 33 + 4 + 6;
    Indent = DrawBorder(Backbuffer, DestStride, V2i(SquareWidth * BoardDimensions.Width, SquareWidth * BoardDimensions.Height), 3, Position, DarkGray, Gray, White);
    Position.X += Indent;
    Position.Y += Indent;

    int32_t TapCount = (MouseHalfTransitionCount + !!MouseEndedDown) / 2;

    enum smiley_state SmileyState = smiley_state_Normal;
    for (int32_t j = 0; j < BoardDimensions.Height; ++j) {
        for (int32_t i = 0; i < BoardDimensions.Width; ++i) {
            struct v2i TilePosition = {Position.X + i * SquareWidth, Position.Y + j * SquareWidth};
            struct rect2i TileRectangle = {TilePosition, {innerWidth + borderWidth + borderWidth, innerWidth + borderWidth + borderWidth}};
            if (RectangleContains(TileRectangle, MousePosition) && MouseEndedDown)
            {
                SmileyState = smiley_state_Surprised;
                DrawTile(Backbuffer, DestStride, innerWidth, borderWidth, TilePosition, tile_state_Depressed);
//                GameState->BoardDimensionsChoice = (GameState->BoardDimensionsChoice + TapCount) % ArrayCount(BoardDimensionsOptions);
            }
            else
            {
                DrawTile(Backbuffer, DestStride, innerWidth, borderWidth, TilePosition, tile_state_Normal);
            }
        }
    }
    DrawSmiley(Backbuffer, SmileyPosition, SmileyState);
#endif
}
