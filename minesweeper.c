#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

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


#define Assert(Expr) {if(!(Expr)) {int __AssertInt = *((volatile int *)0);}}
#define InvalidCodePath Assert(!"Invalid code path");
#define InvalidDefaultCase default: Assert(!"Invalid default case"); break;
#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
#define OffsetOf(type, Member) ((size_t)&(((type *)0)->Member))
#define NotImplemented Assert(!"Not implemented");

#define Kilobytes(Value) ((u64)(Value)*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

extern void logu64(u64);


#pragma pack(push, 1)
struct v2i
{
    union
    {
        struct
        {
            s32 X;
            s32 Y;
        };
        struct
        {
            s32 Width;
            s32 Height;
        };
    };
};
struct image_header
{
    s32 Width;
    s32 Height;
};

struct seed
{
    u64 Value;
};

struct game_button
{
    s32 EndedDown;
    s32 HalfTransitionCount;
};

struct game_input
{
    float ElapsedSeconds;
    u64 SeedValue;
    struct v2i MousePosition;
    struct game_button LeftMouseButton;
    struct game_button RightMouseButton;
};
#pragma pack(pop)

struct v2i V2i(s32 X, s32 Y)
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
    u8 *Memory;
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

u32 RectangleContains(struct rect2i Rectangle, struct v2i Position)
{
    s32 Left = Rectangle.Position.X;
    s32 Right = Rectangle.Position.X + Rectangle.Dimensions.Width;
    s32 Top = Rectangle.Position.Y;
    s32 Bottom = Rectangle.Position.Y + Rectangle.Dimensions.Height;
    return Left <= Position.X && Position.X < Right && Top <= Position.Y && Position.Y < Bottom;
}

// Field State is stored like in 8-bit value
// Bits 3-0 (LSB) store the number of neighboring mines (0-8)
// Bit 4 is whether it's a mine.
// Bit 5 is unused.
// Bits 7-6 (MSB) store the user state of the tile
//   00 = Blank (unrevealed)
//   01 = Flagged
//   10 = Question Marked
//   11 = Revealed

enum tile_user_state
{
    tile_user_state_Blank,
    tile_user_state_Flagged,
    tile_user_state_QuestionMarked,
    tile_user_state_Revealed,
};

struct minesweeper_tile_state
{
    u8 NeighborCount;
    u8 IsMine;
    enum tile_user_state UserState;

    u8 IsDepressed;
};

enum minesweeper_action
{
    MinesweeperAction_None,
    MinesweeperAction_Reveal,
    MinesweeperAction_ToggleUserState,
};

struct minesweeper_tile_state
ExtractMinesweeperTileState(u8 Value)
{
    struct minesweeper_tile_state Result;
    Result.NeighborCount = Value & 0x0F;
    Result.IsMine = (Value & 0x10) == 0x10;
    Result.UserState = (Value >> 6);

    Result.IsDepressed = 0;
    return Result;
}

u8
CompressMinesweeperTileState(struct minesweeper_tile_state State)
{
    u8 Result = (
        (State.NeighborCount << 0)
        | (State.IsMine << 4)
        | (State.UserState << 6));
    return Result;
}

enum draw_flags
{
    draw_flags_None = 0,
    draw_flags_Measure = 1 << 0,
};

struct rect2i DrawRectangle(struct backbuffer *Backbuffer, s32 Stride, struct rect2i Rectangle, struct v4 Color, enum draw_flags Flags)
{
    struct v2i Dimensions = Rectangle.Dimensions;
    struct v2i Position = Rectangle.Position;
    s32 StartX = Max(0, Position.X);
    s32 StartY = Max(0, Position.Y);
    s32 StopX = Min(Position.X + (s32)Dimensions.Width, (s32)Backbuffer->Dimensions.Width);
    s32 StopY = Min(Position.Y + (s32)Dimensions.Height, (s32)Backbuffer->Dimensions.Height);
    struct rect2i Result = {Position, V2i(StopX - Position.X, StopY - Position.Y)};
    if (!(Flags & draw_flags_Measure))
    {
        u8 *Row = Backbuffer->Memory + Stride * StartY + StartX * 4;
        u32 PackedColor = 
            ((u8)(255.0f * Clamp(0.0f, Color.Red,   1.0f)) << 0)
            | ((u8)(255.0f * Clamp(0.0f, Color.Green, 1.0f)) << 8)
            | ((u8)(255.0f * Clamp(0.0f, Color.Blue,  1.0f)) << 16)
            | ((u8)(255.0f * Clamp(0.0f, Color.Alpha, 1.0f)) << 24);
    
        for (s32 IndexY = StartY; IndexY < StopY; IndexY++)
        {
            u32 *At = (u32*)Row;
            for (s32 IndexX = StartX; IndexX < StopX; IndexX++)
            {
                *At++ = PackedColor;
            }
            Row += Stride;
        }
    }
    
    return Result;
}

s32 DrawBorder(struct backbuffer *Backbuffer, s32 Stride, struct v2i Dimensions, s32 BorderWidth, struct v2i Position, s32 colorTopLeft, s32 colorCenter, s32 colorBottomRight)
{
    s32 FullWidth = Dimensions.Width + 2*BorderWidth;
    s32 FullHeight = Dimensions.Height + 2*BorderWidth;
    s32 StartX = Max(0, Position.X);
    s32 StartY = Max(0, Position.Y);
    s32 StopTopY = Min(Position.Y + (s32)BorderWidth, (s32)Backbuffer->Dimensions.Height);
    s32 StopMidY = Min(Position.Y + (s32)BorderWidth + Dimensions.Height, (s32)Backbuffer->Dimensions.Height);
    s32 StopX = Min(Position.X + (s32)FullWidth, (s32)Backbuffer->Dimensions.Width);
    s32 StopY = Min(Position.Y + (s32)FullHeight, (s32)Backbuffer->Dimensions.Height);
    s32 StopLeftX = Min(Position.X + (s32)BorderWidth, (s32)Backbuffer->Dimensions.Width);
    s32 StartRightX = Max((s32)0, Position.X + BorderWidth + Dimensions.Width);
    u8 *Row = Backbuffer->Memory + Stride * StartY + StartX * 4;
    for (s32 IndexY = StartY; IndexY < StopTopY; IndexY++) {
        u8 *At = Row;
        s32 IndexX;
        s32 StopTopLeft = Min(Position.X + FullWidth - IndexY + StartY - 1, StopX);
        for (IndexX = StartX; IndexX < StopTopLeft; IndexX++) {
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = 255;
        }
        s32 StopMid = Min(Position.X + FullWidth - IndexY + StartY, StopX);
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
    for (s32 IndexY = Max(0, Position.Y + BorderWidth); IndexY < StopMidY; IndexY++) {
        u8 *At = Row;
        s32 IndexX;
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
    for (s32 IndexY = StartY; IndexY < StopY; IndexY++) {
        u8 *At = Row;
        s32 IndexX;
        s32 StopBottomLeft = Min(Position.X + BorderWidth - IndexY + StartY - 1, StopX);
        for (IndexX = StartX; IndexX < StopBottomLeft; IndexX++) {
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = colorTopLeft;
            *At++ = 255;
        }
        s32 StopMid = Min(Position.X + BorderWidth - IndexY + StartY, StopX);
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

void DrawBitmap(struct backbuffer *Backbuffer, s32 Stride, struct v2i Position, struct bitmap Bitmap)
{
    char *SourceRow = Bitmap.Bitmap;
    s32 StartY = Max(0, Position.Y);
    s32 StartX = Max(0, Position.X);
    s32 StopY = Min(Position.Y + Bitmap.Dimensions.Height, Backbuffer->Dimensions.Height);
    s32 StopX = Min(Position.X + Bitmap.Dimensions.Width, Backbuffer->Dimensions.Width);
    u8 *DestRow = Backbuffer->Memory + StartY * Stride + StartX * 4;
    SourceRow += (StartY - Position.Y) * Bitmap.Dimensions.Width + StartX - Position.X;
    for (s32 IndexY = StartY; IndexY < StopY; IndexY++)
    {
        u32 *Dest = (u32*)DestRow;
        char *Source = SourceRow;
        for (s32 IndexX = StartX; IndexX < StopX; IndexX++)
        {
            s32 ColorIndex = *Source++ - '0';
            u32 Color = 0;
            if (0 <= ColorIndex && ColorIndex < Bitmap.ColorCount)
            {
                Color = Bitmap.Colors[ColorIndex];
            }
            if (Color)
            {
                *Dest = Color;
            }
            Dest++;
        }
        DestRow += Stride;
        SourceRow += Bitmap.Dimensions.Width;
    }
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
    s32 Stride = Backbuffer->Dimensions.Width * 4;
    struct bitmap Bitmap;
    switch (State)
    {
    case smiley_state_Surprised:
    {
        Bitmap = SurprisedBitmap;
    } break;
    case smiley_state_Cool:
    {
        Bitmap = CoolBitmap;
    } break;
    case smiley_state_Frowney:
    {
        Bitmap = FrowneyBitmap;
    } break;
    case smiley_state_Normal:
    default:
    {
        Bitmap = SmileyBitmap;
    } break;
    }
    DrawBitmap(Backbuffer, Stride, Position, Bitmap);
}

void DrawNumber(struct backbuffer *Backbuffer, struct v2i Position, s32 Number)
{
    s32 Stride = Backbuffer->Dimensions.Width * 4;
    
    Number = Clamp(0, Number, 999);
    s32 RemainingDigits = 3;
    s32 Digits[3];
    while (RemainingDigits--)
    {
        Digits[RemainingDigits] = Number % 10;
        Number = Number / 10;
    }
    RemainingDigits = 3;
    s32 *AtDigit = Digits;
    while (RemainingDigits--)
    {
        s32 Digit = *AtDigit++;
        struct bitmap Bitmap = SevenSegmentBitmapBlank;
        if (0 <= Digit && Digit < ArrayCount(SevenSegmentBitmaps))
        {
            Bitmap = SevenSegmentBitmaps[Digit];
        }
        DrawBitmap(Backbuffer, Stride, Position, Bitmap);
        Position.X += Bitmap.Dimensions.X;
    }
}

struct rect2i DrawTile(struct backbuffer *Backbuffer, s32 Stride, s32 Width, s32 BorderWidth, struct v2i Position, struct minesweeper_tile_state State)
{
    struct rect2i Result = {Position, {Width + BorderWidth + BorderWidth, Width + BorderWidth + BorderWidth}};
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    if (State.IsDepressed || State.UserState == tile_user_state_Revealed)
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Result.Dimensions.Width - 2;
        Rectangle.Position.X = Position.X + 1;
        Rectangle.Position.Y = Position.Y + 1;
        DrawBorder(Backbuffer, Stride, Rectangle.Dimensions, 1, Position, DarkGray, DarkGray, DarkGray);
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Rectangle.Dimensions.Width + 1;
        DrawRectangle(Backbuffer, Stride, Rectangle, GrayVector, draw_flags_None);
    }
    else
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Width;
        Rectangle.Position.X = Position.X + BorderWidth;
        Rectangle.Position.Y = Position.Y + BorderWidth;
        DrawBorder(Backbuffer, Stride, Rectangle.Dimensions, BorderWidth, Position, White, Gray, DarkGray);
        DrawRectangle(Backbuffer, Stride, Rectangle, GrayVector, draw_flags_None);

        if (State.UserState == tile_user_state_Flagged)
        {
            DrawBitmap(Backbuffer, Stride, Rectangle.Position, FlagBitmap);
        }
        else if (State.UserState == tile_user_state_QuestionMarked)
        {
            DrawBitmap(Backbuffer, Stride, Rectangle.Position, QuestionMarkBitmap);
        }
    }

    return Result;
}

float Fmod(float a, float b) {
    float q = a / b;
    return q - (u32)q;
}

#define MAX_FIELD_WIDTH 30
#define MAX_FIELD_HEIGHT 24

enum minesweeper_gameplay_state
{
    minesweeper_gameplay_state_Playing,
    minesweeper_gameplay_state_Victorious,
    minesweeper_gameplay_state_GameOver,
};

struct seed
Seed(u64 Value)
{
    struct seed Result;
    Result.Value = Value;
    return(Result);
}

u64
RandomU64(struct seed *Seed)
{
    u64 Result = 6364136223846793005ull*Seed->Value+1442695040888963407ull;
    Seed->Value = Result;
    return(Result);
}

u32
RandomIndex(struct seed *Seed, u32 Count)
{
    s32 Result = RandomU64(Seed)%Count;
    return(Result);
}

struct game_state
{
    u32 Initialized;
    enum minesweeper_gameplay_state GameplayState;
    struct v2i DesiredFieldDimensions;
    struct seed MineSeed;
    s32 DesiredMineCount;
    struct v2i FieldDimensions;
    float GameStart;
    struct v2i WindowPosition;
    struct v2i DragOffset;
    struct v2i *DragTarget;
    s32 FlagsRemaining;
    u8 Field[MAX_FIELD_HEIGHT+2][MAX_FIELD_WIDTH+2];
};

struct v2i
V2iPlusV2i(struct v2i A, struct v2i B)
{
    struct v2i Result = {A.X + B.X, A.Y + B.Y};
    return Result;
}

void *
SetMemory(void *Ptr, int Value, size_t ByteCount)
{
    unsigned char * At = (unsigned char*)Ptr;
    while(ByteCount--)
    {
        *At++ = (unsigned char)Value;
    }
    return Ptr;
}

void
Reset(struct game_state *State)
{
    State->GameplayState = minesweeper_gameplay_state_Playing;
    State->FieldDimensions = State->DesiredFieldDimensions;
    SetMemory(State->Field, 0, sizeof(State->Field));
    struct v2i Apron = V2i(1, 1);
    u8 Revealed0 = 0b11000000;
    u8 Mine = 0b00010000;
    struct v2i FieldDimensionsWithHalfApron = V2iPlusV2i(State->FieldDimensions, Apron);
    for (s32 Index = 0; Index < FieldDimensionsWithHalfApron.X; ++Index)
    {
        State->Field[0][Index] = Revealed0;
        State->Field[FieldDimensionsWithHalfApron.Y][Index+1] = Revealed0;
    }
    for (s32 Index = 0; Index < FieldDimensionsWithHalfApron.Y; ++Index)
    {
        State->Field[Index][FieldDimensionsWithHalfApron.X] = Revealed0;
        State->Field[Index+1][0] = Revealed0;
    }

    struct v2i Coordinate;
    struct v2i FieldEnd = V2iPlusV2i(State->FieldDimensions, Apron);
    struct v2i CoordinatePool[MAX_FIELD_WIDTH*MAX_FIELD_HEIGHT];
    s32 CoordinateCount = 0;
    for (Coordinate.Y = Apron.Y; Coordinate.Y < FieldEnd.Y; ++Coordinate.Y)
    {
        for (Coordinate.X = Apron.X; Coordinate.X < FieldEnd.X; ++Coordinate.X)
        {
            CoordinatePool[CoordinateCount++] = Coordinate;
        }
    }
    Assert(CoordinateCount == State->FieldDimensions.X * State->FieldDimensions.Y);

    s32 MinesRemaining = State->DesiredMineCount;

    while (MinesRemaining--)
    {
        u32 Index = RandomIndex(&State->MineSeed, CoordinateCount);
        Coordinate = CoordinatePool[Index];
        CoordinatePool[Index] = CoordinatePool[--CoordinateCount];
        State->Field[Coordinate.Y][Coordinate.X] = Mine;
    }

    for (Coordinate.Y = Apron.Y; Coordinate.Y < FieldEnd.Y; ++Coordinate.Y)
    {
        for (Coordinate.X = Apron.X; Coordinate.X < FieldEnd.X; ++Coordinate.X)
        {
            u8 NWMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y-1][Coordinate.X-1]).IsMine;
            u8 NNMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y-1][Coordinate.X+0]).IsMine;
            u8 NEMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y-1][Coordinate.X+1]).IsMine;
            u8 WWMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y+0][Coordinate.X-1]).IsMine;
            u8 EEMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y+0][Coordinate.X+1]).IsMine;
            u8 SWMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y+1][Coordinate.X-1]).IsMine;
            u8 SSMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y+1][Coordinate.X+0]).IsMine;
            u8 SEMine = ExtractMinesweeperTileState(State->Field[Coordinate.Y+1][Coordinate.X+1]).IsMine;
            u8 NeighboringMines = NWMine + NNMine + NEMine + WWMine + EEMine + SWMine + SSMine + SEMine;
            State->Field[Coordinate.Y][Coordinate.X] |= (NeighboringMines & 0x0F);
        }
    }
}

void GameUpdateAndRender(s32 Width, s32 Height, u8 *BackbufferMemory, u8 *AssetsMemory, size_t GameMemorySize, u8 *GameMemory)
{
    struct game_input *GameInput = (struct game_input*)GameMemory;
    struct game_state *GameState = (struct game_state*)(GameInput + 1);
    float ElapsedSeconds = GameInput->ElapsedSeconds;

    struct v2i MousePosition = GameInput->MousePosition;

    s32 LeftMouseEndedDown = GameInput->LeftMouseButton.EndedDown;
    s32 LeftMouseHalfTransitionCount = GameInput->LeftMouseButton.HalfTransitionCount;

    s32 RightMouseEndedDown = GameInput->RightMouseButton.EndedDown;
    s32 RightMouseHalfTransitionCount = GameInput->RightMouseButton.HalfTransitionCount;

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
        GameState->MineSeed = Seed(GameInput->SeedValue);
        GameState->WindowPosition.X = 100;
        GameState->WindowPosition.Y = 100;
        GameState->FlagsRemaining = 10;
        GameState->GameStart = ElapsedSeconds;
        GameState->DesiredFieldDimensions = Beginner;
        GameState->DesiredMineCount = 10;
        Reset(GameState);
    }

    if (GameState->DragTarget)
    {
        if (LeftMouseEndedDown)
        {
            GameState->DragTarget->X = MousePosition.X + GameState->DragOffset.X;
            GameState->DragTarget->Y = MousePosition.Y + GameState->DragOffset.Y;
        }
        else
        {
            GameState->DragTarget = 0;
        }
    }
    
    struct image_header *Header = (struct image_header *)AssetsMemory;
    s32 DestStride = Width*4;

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
        u8 *DestRow = BackbufferMemory;
        u8 *Background = AssetsMemory + sizeof(*Header);
        for (s32 y = 0; y < Height; ++y) {
            float V = ((float)y + 0.5f) * VMultiplier + VOffset;
            u8 *Dest = DestRow;
            for (s32 x = 0; x < Width; ++x) {
                float U = ((float)x + 0.5f) * UMultiplier + UOffset;
                u32 SourceX = (u32)(U * Header->Width);
                u32 SourceY = (u32)(V * Header->Height);
                u8 *Source = Background + (SourceY * Header->Width + SourceX) * 3;
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
                *Dest++ = (u8)(255.0f * (ElapsedSeconds + Fmod(x, (float)Width) + 0.5f));
                *Dest++ = (u8)(255.0f * (ElapsedSeconds - Fmod(y, (float)Height) + 0.5f));
                *Dest++ = (u8)(255.0f * (ElapsedSeconds - Fmod(x, (float)Width) + 0.5f));
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
    
    
    s32 borderWidth = 2;
    s32 innerWidth = 12;
    s32 SquareWidth = innerWidth + 2*borderWidth;
    struct v2i FieldDimensions = GameState->FieldDimensions;
    struct v2i GameDimensionsInPixels = {SquareWidth * FieldDimensions.Width + 6, SquareWidth * FieldDimensions.Height + 6};
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
    s32 Indent = DrawBorder(Backbuffer, DestStride, V2i(GameBackgroundDimensions.Width + 6, GameBackgroundDimensions.Height + 6 + TitleBarRectangle.Dimensions.Height * 2 + SeparatorRectangle.Dimensions.Height * 2), 1, Position, Black, Black, Black);
#if 1
    Position.X += Indent;
    Position.Y += Indent;
    TitleBarRectangle.Position = Position;
    if (RectangleContains(TitleBarRectangle, MousePosition))
    {
        if (LeftMouseEndedDown && (LeftMouseHalfTransitionCount & 1))
        {
            GameState->DragOffset.X = GameState->WindowPosition.X - MousePosition.X;
            GameState->DragOffset.Y = GameState->WindowPosition.Y - MousePosition.Y;
            GameState->DragTarget = &GameState->WindowPosition;
        }
    }
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
    s32 ScoreBackgroundWidth = GameBackgroundDimensions.Width - 16;
    Indent = DrawBorder(Backbuffer, DestStride, V2i(ScoreBackgroundWidth, 33), 2, Position, DarkGray, Gray, White);
    struct v2i ScoreOrigin = {Position.X + Indent, Position.Y + Indent};
    struct v2i ScorePosition = {ScoreOrigin.X + 5, ScoreOrigin.Y + 4};
    struct v2i ScoreDimensions = {39, 23};
    struct v4 ScoreBackgroundColor = {0,0,0,1};
    Indent = DrawBorder(Backbuffer, DestStride, ScoreDimensions, 1, ScorePosition, DarkGray, Gray, White);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    DrawRectangle(Backbuffer, DestStride, Rect2i(ScorePosition, ScoreDimensions), ScoreBackgroundColor, draw_flags_None);
    s32 GameTimer = (s32)(ElapsedSeconds - GameState->GameStart);
    DrawNumber(Backbuffer, ScorePosition, GameState->FlagsRemaining);

    s32 SmileyBorderWidthOuter = 1;
    s32 SmileyBorderWidthInner = 2;
    struct v2i SmileyDimensionsInner = V2i(20, 20);
    struct v2i SmileyDimensionsOuter = V2i(SmileyDimensionsInner.X + SmileyBorderWidthInner*2, SmileyDimensionsInner.Y + SmileyBorderWidthInner*2);
    s32 SmileyWidth = SmileyDimensionsOuter.X + SmileyBorderWidthOuter*2;
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
    Indent = DrawBorder(Backbuffer, DestStride, V2i(SquareWidth * FieldDimensions.Width, SquareWidth * FieldDimensions.Height), 3, Position, DarkGray, Gray, White);
    Position.X += Indent;
    Position.Y += Indent;

    s32 LeftReleaseCount = (LeftMouseHalfTransitionCount + !LeftMouseEndedDown) / 2;
    s32 RightReleaseCount = (RightMouseHalfTransitionCount + !RightMouseEndedDown) / 2;

    enum smiley_state SmileyState = smiley_state_Normal;
    for (s32 j = 0; j < FieldDimensions.Height; ++j) {
        for (s32 i = 0; i < FieldDimensions.Width; ++i) {
            struct v2i FieldCoordinate = {i+1, j+1};
            u8 TileValue = GameState->Field[FieldCoordinate.Y][FieldCoordinate.X];
            struct minesweeper_tile_state TileState = ExtractMinesweeperTileState(TileValue);
            struct v2i TilePosition = {Position.X + i * SquareWidth, Position.Y + j * SquareWidth};
            struct rect2i TileRectangle = {TilePosition, {innerWidth + borderWidth + borderWidth, innerWidth + borderWidth + borderWidth}};
            s32 Hover = RectangleContains(TileRectangle, MousePosition);
            if (TileState.UserState != tile_user_state_Revealed && Hover)
            {
                if (LeftReleaseCount)
                {
                    if (TileState.UserState == tile_user_state_Flagged)
                    {
                    }
                    else if (TileState.IsMine)
                    {
                        GameState->GameplayState = minesweeper_gameplay_state_GameOver;
                    }
                    else
                    {
                        TileState.UserState = tile_user_state_Revealed;
//                        FloodFill(State, &TranState->Arena, FieldCoordinate);
                    }
                }

                if (TileState.UserState != tile_user_state_Revealed)
                {
                    enum tile_user_state OldUserState = TileState.UserState;
                    TileState.UserState = (TileState.UserState + RightReleaseCount) % 3;
                    if (OldUserState != TileState.UserState)
                    {
                        if (TileState.UserState == tile_user_state_Flagged)
                        {
                            if (GameState->FlagsRemaining <= 0)
                            {
                                TileState.UserState = tile_user_state_QuestionMarked;
                            }
                            else
                            {
                                GameState->FlagsRemaining--;
                            }
                        }
                        else if (OldUserState == tile_user_state_Flagged)
                        {
                            GameState->FlagsRemaining++;
                        }
                    }
                    if (LeftMouseEndedDown && TileState.UserState != tile_user_state_Flagged)
                    {
                        SmileyState = smiley_state_Surprised;
                        TileState.IsDepressed = 1;
                    }
                }
            }
            GameState->Field[FieldCoordinate.Y][FieldCoordinate.X] = CompressMinesweeperTileState(TileState);

            DrawTile(Backbuffer, DestStride, innerWidth, borderWidth, TilePosition, TileState);
        }
    }
    DrawSmiley(Backbuffer, SmileyPosition, SmileyState);
#endif
}
