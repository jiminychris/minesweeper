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

typedef int32_t b32;

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

#include "memory.h"

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
#include "font.h"

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

struct v4
V4(float X, float Y, float Z, float W)
{
    struct v4 Result = {X, Y, Z, W};
    return Result;
}

s32
V2iEquals(struct v2i A, struct v2i B)
{
    s32 Result = A.X == B.X && A.Y == B.Y;
    return Result;
}

struct backbuffer
{
    struct v2i Dimensions;
    s32 Stride;
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

struct rect2i DrawRectangle(struct backbuffer *Backbuffer, struct rect2i Rectangle, struct v4 Color, enum draw_flags Flags)
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
        u8 *Row = Backbuffer->Memory + Backbuffer->Stride * StartY + StartX * 4;
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
            Row += Backbuffer->Stride;
        }
    }
    
    return Result;
}

s32 DrawBorder(struct backbuffer *Backbuffer, struct v2i Dimensions, s32 BorderWidth, struct v2i Position, s32 colorTopLeft, s32 colorCenter, s32 colorBottomRight)
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
    u8 *Row = Backbuffer->Memory + Backbuffer->Stride * StartY + StartX * 4;
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
        Row += Backbuffer->Stride;
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
        At = Backbuffer->Memory + Backbuffer->Stride * IndexY + StartRightX * 4;
        for (IndexX = StartRightX; IndexX < StopX; IndexX++) {
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = colorBottomRight;
            *At++ = 255;
        }
        Row += Backbuffer->Stride;
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
        Row += Backbuffer->Stride;
    }
    return BorderWidth;
}

void DrawBitmap(struct backbuffer *Backbuffer, struct v2i Position, struct bitmap Bitmap)
{
    char *SourceRow = Bitmap.Bitmap;
    s32 StartY = Max(0, Position.Y);
    s32 StartX = Max(0, Position.X);
    s32 StopY = Min(Position.Y + Bitmap.Dimensions.Height, Backbuffer->Dimensions.Height);
    s32 StopX = Min(Position.X + Bitmap.Dimensions.Width, Backbuffer->Dimensions.Width);
    u8 *DestRow = Backbuffer->Memory + StartY * Backbuffer->Stride + StartX * 4;
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
        DestRow += Backbuffer->Stride;
        SourceRow += Bitmap.Dimensions.Width;
    }
}

struct layout
{
    s32 DefaultKerning;
    struct v2i Position;
    struct font Font;
    struct v4 Color;
};

u32 V4ToU32(struct v4 Color)
{
    u32 Result = 
        ((u8)(255.0f * Clamp(0.0f, Color.Red,   1.0f)) << 0)
        | ((u8)(255.0f * Clamp(0.0f, Color.Green, 1.0f)) << 8)
        | ((u8)(255.0f * Clamp(0.0f, Color.Blue,  1.0f)) << 16)
        | ((u8)(255.0f * Clamp(0.0f, Color.Alpha, 1.0f)) << 24);
    return Result;
}

enum text_flags
{
    text_flags_None = 0,
    text_flags_Underline = 1 << 0,
};

struct string_reference
{
    s32 Length;
    char *String;
};

struct rect2i
DrawText(struct backbuffer *Backbuffer, struct layout *Layout, struct string_reference Text, enum text_flags Flags)
{
    struct rect2i Result = { Layout->Position, {0, 0} };
    u32 Colors[2];
    Colors[0] = 0;
    Colors[1] = V4ToU32(Layout->Color);
    char *At = Text.String;
    s32 Remaining = Text.Length;
    while (Remaining--)
    {
        struct bitmap Bitmap = GetGlyph(Layout->Font, *At++);
        Bitmap.ColorCount = ArrayCount(Colors);
        Bitmap.Colors = Colors;
        if (Backbuffer)
        {
            DrawBitmap(Backbuffer, Layout->Position, Bitmap);
            Layout->Position.X += Bitmap.Dimensions.Width + Layout->DefaultKerning;
        }
        Result.Dimensions.Width += Bitmap.Dimensions.Width;
        Result.Dimensions.Height = Max(Result.Dimensions.Height, Bitmap.Dimensions.Height);
    }

    if (Flags & text_flags_Underline)
    {
        if (Backbuffer)
        {
            struct rect2i Rectangle = Result;
            Rectangle.Dimensions.Height = 1;
            Rectangle.Position.Y += Result.Dimensions.Height + 1;
            DrawRectangle(Backbuffer, Rectangle, Layout->Color, 0);
        }
        Result.Dimensions.Height += 2;
    }
    
    return Result;
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
    DrawBitmap(Backbuffer, Position, Bitmap);
}

void DrawSevenSegment(struct backbuffer *Backbuffer, struct v2i Position, s32 Number)
{
    Number = Clamp(-99, Number, 999);
    s32 RemainingDigits = 3;
    struct bitmap Bitmaps[3];
    s32 Negative = Number < 0;
    if (Negative)
    {
        Number *= -1;
    }

    do
    {
        Bitmaps[--RemainingDigits] = SevenSegmentBitmaps[Number % 10];
        Number = Number / 10;
    } while (Number);
    if (Negative)
    {
        Bitmaps[--RemainingDigits] = SevenSegmentBitmapMinus;
    }
    while (RemainingDigits)
    {
        Bitmaps[--RemainingDigits] = SevenSegmentBitmapBlank;
    }
    RemainingDigits = 3;
    struct bitmap *Bitmap = Bitmaps;
    while (RemainingDigits--)
    {
        DrawBitmap(Backbuffer, Position, *Bitmap);
        Position.X += Bitmap->Dimensions.X;
        Bitmap++;
    }
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
    b32 Initialized;
    b32 IsGameMenuOpen;
    float MenuDebounce;
    enum minesweeper_gameplay_state GameplayState;
    float GameEnd;
    struct v2i TriggeredCoordinate;
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

struct v2i
S32TimesV2i(s32 Scale, struct v2i A)
{
    struct v2i Result = {Scale * A.X, Scale * A.Y};
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
FloodFill(struct game_state *State, struct memory_arena *Arena, struct v2i Coordinate)
{
    struct temporary_memory StackMemory = BeginTemporaryMemory(Arena);

    u32 StackSize = 1;
    struct v2i *Stack = PushStruct(StackMemory.Arena, struct v2i, 0);
    *Stack = Coordinate;

    while (StackSize)
    {
        Coordinate = Stack[--StackSize];
        StackMemory.Arena->Used -= sizeof(Coordinate);
        u8 TileValue = State->Field[Coordinate.Y][Coordinate.X];
        struct minesweeper_tile_state TileState = ExtractMinesweeperTileState(TileValue);
        if (TileState.UserState != tile_user_state_Revealed)
        {
            State->Field[Coordinate.Y][Coordinate.X] = 0xC0 | (TileValue & 0x3F);
            if (!TileState.NeighborCount)
            {
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(-1,-1));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(+0,-1));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(+1,-1));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(-1,+0));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(+1,+0));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(-1,+1));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(+0,+1));
                *PushStruct(StackMemory.Arena, struct v2i, 0) = V2iPlusV2i(Coordinate, V2i(+1,+1));
                StackSize += 8;
            }
        }
    }

    EndTemporaryMemory(StackMemory);
}

void
Reset(struct game_input *Input, struct game_state *State)
{
    State->GameStart = Input->ElapsedSeconds;
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

    State->FlagsRemaining = State->DesiredMineCount;
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

struct rect2i DrawTile(struct backbuffer *Backbuffer, s32 Width, s32 BorderWidth, struct v2i Position, struct game_state *GameState, struct minesweeper_tile_state State, struct v2i Coordinate)
{
    struct rect2i Result = {Position, {Width + BorderWidth + BorderWidth, Width + BorderWidth + BorderWidth}};
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    s32 ShowMine = State.IsMine && GameState->GameplayState == minesweeper_gameplay_state_GameOver;
    if (State.IsDepressed || State.UserState == tile_user_state_Revealed || ShowMine)
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Result.Dimensions.Width - 2;
        Rectangle.Position.X = Position.X + 1;
        Rectangle.Position.Y = Position.Y + 1;
        DrawBorder(Backbuffer, Rectangle.Dimensions, 1, Position, DarkGray, DarkGray, DarkGray);
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Rectangle.Dimensions.Width + 1;
        struct v4 BackgroundColor = GrayVector;
        if (ShowMine && V2iEquals(Coordinate, GameState->TriggeredCoordinate))
        {
            BackgroundColor = V4(1, 0, 0, 1);
        }
        DrawRectangle(Backbuffer, Rectangle, BackgroundColor, draw_flags_None);
        if (ShowMine)
        {
            DrawBitmap(Backbuffer, Position, MineBitmap);
        }
        else if (State.UserState == tile_user_state_Revealed)
        {
            DrawBitmap(Backbuffer, Position, NeighborNumberBitmaps[State.NeighborCount]);
        }
    }
    else
    {
        struct rect2i Rectangle;
        Rectangle.Dimensions.Width = Rectangle.Dimensions.Height = Width;
        Rectangle.Position.X = Position.X + BorderWidth;
        Rectangle.Position.Y = Position.Y + BorderWidth;
        DrawBorder(Backbuffer, Rectangle.Dimensions, BorderWidth, Position, White, Gray, DarkGray);
        DrawRectangle(Backbuffer, Rectangle, GrayVector, draw_flags_None);
        if (State.UserState == tile_user_state_Flagged)
        {
            DrawBitmap(Backbuffer, Rectangle.Position, FlagBitmap);
        }
        else if (State.UserState == tile_user_state_QuestionMarked)
        {
            DrawBitmap(Backbuffer, Rectangle.Position, QuestionMarkBitmap);
        }
    }

    return Result;
}

enum menu_item_type
{
    menu_item_type_New,
    menu_item_type_Separator,
    menu_item_type_Beginner,
    menu_item_type_Intermediate,
    menu_item_type_Expert,
    menu_item_type_Custom,
};

s32 StringLength(char *String)
{
    s32 Result = 0;
    while (*String++)
    {
        Result++;
    }
    return Result;
}

struct string_reference StringReference(char *String)
{
    struct string_reference Result = {StringLength(String), String};
    return Result;
}

struct string_reference Substring(struct string_reference Ref, s32 Start, s32 Length)
{
    if (Length < 0)
    {
        Length = Ref.Length - Start;
    }
    struct string_reference Result = {Length, Ref.String + Start};
    return Result;
}

struct menu_item
{
    enum menu_item_type Type;
    struct string_reference Name;
    s32 ShortcutCharIndex;
};

void GameUpdateAndRender(s32 Width, s32 Height, u8 *BackbufferMemory, u8 *AssetsMemory, size_t GameMemorySize, u8 *GameMemory)
{
    struct game_input *GameInput = (struct game_input*)GameMemory;
    struct game_state *GameState = (struct game_state*)(GameInput + 1);
    u8 *ScratchMemory = (u8*)(GameState + 1);
    struct memory_arena ScratchArena;
    InitializeArena(&ScratchArena, GameMemorySize - (ScratchMemory - GameMemory), ScratchMemory);
    float ElapsedSeconds = GameInput->ElapsedSeconds;

    struct v2i MousePosition = GameInput->MousePosition;

    s32 LeftMouseEndedDown = GameInput->LeftMouseButton.EndedDown;
    s32 LeftMouseHalfTransitionCount = GameInput->LeftMouseButton.HalfTransitionCount;

    s32 RightMouseEndedDown = GameInput->RightMouseButton.EndedDown;
    s32 RightMouseHalfTransitionCount = GameInput->RightMouseButton.HalfTransitionCount;

    s32 LeftPressCount = (LeftMouseHalfTransitionCount + !!LeftMouseEndedDown) / 2;
    s32 LeftReleaseCount = (LeftMouseHalfTransitionCount + !LeftMouseEndedDown) / 2;
    s32 RightPressCount = (RightMouseHalfTransitionCount + !!RightMouseEndedDown) / 2;
    s32 RightReleaseCount = (RightMouseHalfTransitionCount + !RightMouseEndedDown) / 2;

    struct backbuffer _Backbuffer;
    _Backbuffer.Dimensions.Width = Width;
    _Backbuffer.Dimensions.Height = Height;
    _Backbuffer.Memory = BackbufferMemory;
    _Backbuffer.Stride = Width*4;
    struct backbuffer *Backbuffer = &_Backbuffer;

    if (!GameState->Initialized)
    {
        InitializeSystemFont();
        GameState->Initialized = 1;
        GameState->MineSeed = Seed(GameInput->SeedValue);
        GameState->WindowPosition.X = 100;
        GameState->WindowPosition.Y = 100;
        GameState->DesiredFieldDimensions = V2i(9, 9);
        GameState->DesiredMineCount = 10;
        Reset(GameInput, GameState);
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
            DestRow += Backbuffer->Stride;
        }
    }
    else
    {
        DrawRectangle(Backbuffer, ScreenRectangle, BackgroundColor, draw_flags_None);
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
    struct v4 TitleBarColor = { 0.0f, 0.0f, 0.5f, 1.0f };
    struct v4 SeparatorColor = {0, 0, 0, 1};
    struct v4 MenuBarColor = {1, 1, 1, 1};
    s32 Indent = DrawBorder(Backbuffer, V2i(GameBackgroundDimensions.Width + 6, GameBackgroundDimensions.Height + 6 + TitleBarRectangle.Dimensions.Height * 2 + SeparatorRectangle.Dimensions.Height * 2), 1, Position, Black, Black, Black);
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
    DrawRectangle(Backbuffer, TitleBarRectangle, TitleBarColor, draw_flags_None);

    struct string_reference TitleText = StringReference("Minesweeper");
    struct layout Layout;
    Layout.DefaultKerning = 0;
    Layout.Color = V4(1, 1, 1, 1);
    Layout.Font = SystemFont;
    struct rect2i TitleTextRectangle = DrawText(0, &Layout, TitleText, 0);
    Layout.Position = Position;
    Layout.Position.X += (TitleBarRectangle.Dimensions.Width - TitleTextRectangle.Dimensions.Width) / 2;
    Layout.Position.Y += 4;
    DrawText(Backbuffer, &Layout, TitleText, 0);

    Position = TitleBarRectangle.Position;
    Position.Y += TitleBarRectangle.Dimensions.Height;
    SeparatorRectangle.Position = Position;
    DrawRectangle(Backbuffer, SeparatorRectangle, SeparatorColor, draw_flags_None);
    Position.Y += SeparatorRectangle.Dimensions.Height;
    TitleBarRectangle.Position = Position;
    DrawRectangle(Backbuffer, TitleBarRectangle, MenuBarColor, draw_flags_None);

    struct string_reference GameText = StringReference("Game");
    struct v2i MenuButtonPadding = {8, 3};
    struct rect2i MenuButtonHitbox = DrawText(0, &Layout, GameText, text_flags_Underline);
    MenuButtonHitbox.Position = Position;
    MenuButtonHitbox.Dimensions = V2iPlusV2i(MenuButtonHitbox.Dimensions, S32TimesV2i(2, MenuButtonPadding));
    if (RectangleContains(MenuButtonHitbox, MousePosition))
    {
        if (!GameState->IsGameMenuOpen)
        {
            if (LeftPressCount)
            {
                GameState->MenuDebounce = GameInput->ElapsedSeconds + 0.5f;
                GameState->IsGameMenuOpen = 1;
            }
        }
        else if (LeftReleaseCount && GameState->MenuDebounce < GameInput->ElapsedSeconds)
        {
            GameState->IsGameMenuOpen = 0;
        }
    }
    b32 IsGameFocused = !GameState->IsGameMenuOpen;
    if (GameState->IsGameMenuOpen)
    {
        DrawRectangle(Backbuffer, MenuButtonHitbox, V4(0, 0, 0, 1), 0);
        struct rect2i MenuButtonHighlight = MenuButtonHitbox;
        MenuButtonHighlight.Position.X += 1;
        MenuButtonHighlight.Position.Y += 1;
        MenuButtonHighlight.Dimensions.Width -= 2;
        MenuButtonHighlight.Dimensions.Height -= 2;
        DrawRectangle(Backbuffer, MenuButtonHighlight, V4(0, 0, 0.5f, 1), 0);
        Layout.Color = V4(1, 1, 1, 1);
    }
    else
    {
        Layout.Color = V4(0, 0, 0, 1);
    }
    Layout.Position = V2iPlusV2i(Position, MenuButtonPadding);
    DrawText(Backbuffer, &Layout, Substring(GameText, 0, 1), text_flags_Underline);
    DrawText(Backbuffer, &Layout, Substring(GameText, 1, 3), 0);

    Position = TitleBarRectangle.Position;
    Position.Y += TitleBarRectangle.Dimensions.Height;
    SeparatorRectangle.Position = Position;
    DrawRectangle(Backbuffer, SeparatorRectangle, SeparatorColor, draw_flags_None);
    Position.Y += SeparatorRectangle.Dimensions.Height;

    Indent = DrawBorder(Backbuffer, V2i(GameBackgroundDimensions.Width, GameBackgroundDimensions.Height), 3, Position, White, Gray, DarkGray);
    Position.X += Indent;
    Position.Y += Indent;
    struct rect2i GameBackgroundRectangle = {Position, GameBackgroundDimensions};
    struct v4 GrayVector = {(float)Gray/255.0f, (float)Gray/255.0f, (float)Gray/255.0f, 1.0f};
    DrawRectangle(Backbuffer, GameBackgroundRectangle, GrayVector, draw_flags_None);
    Position.X += 6;
    Position.Y += 6;
    s32 ScoreBackgroundWidth = GameBackgroundDimensions.Width - 16;
    Indent = DrawBorder(Backbuffer, V2i(ScoreBackgroundWidth, 33), 2, Position, DarkGray, Gray, White);
    struct v2i ScoreOrigin = {Position.X + Indent, Position.Y + Indent};
    struct v2i ScorePosition = {ScoreOrigin.X + 5, ScoreOrigin.Y + 4};
    struct v2i ScoreDimensions = {39, 23};
    struct v4 ScoreBackgroundColor = {0,0,0,1};
    Indent = DrawBorder(Backbuffer, ScoreDimensions, 1, ScorePosition, DarkGray, Gray, White);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    DrawRectangle(Backbuffer, Rect2i(ScorePosition, ScoreDimensions), ScoreBackgroundColor, draw_flags_None);
    DrawSevenSegment(Backbuffer, ScorePosition, GameState->FlagsRemaining);

    s32 SmileyBorderWidthOuter = 1;
    s32 SmileyBorderWidthInner = 2;
    struct v2i SmileyDimensionsInner = V2i(20, 20);
    struct v2i SmileyDimensionsOuter = V2i(SmileyDimensionsInner.X + SmileyBorderWidthInner*2, SmileyDimensionsInner.Y + SmileyBorderWidthInner*2);
    s32 SmileyWidth = SmileyDimensionsOuter.X + SmileyBorderWidthOuter*2;
    ScorePosition = V2i(ScoreOrigin.X + (ScoreBackgroundWidth - SmileyWidth) / 2, ScoreOrigin.Y + 4);
    Indent = DrawBorder(Backbuffer, SmileyDimensionsOuter, SmileyBorderWidthOuter, ScorePosition, DarkGray, Gray, DarkGray);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    struct rect2i SmileyHitbox = {ScorePosition, SmileyDimensionsOuter};
    struct v2i SmileyPosition = V2iPlusV2i(ScorePosition, V2i(4, 4));
    s32 SmileyColorTopLeft = White;
    s32 SmileyColorMid = Gray;
    s32 SmileyColorBottomRight = DarkGray;
    if (IsGameFocused && RectangleContains(SmileyHitbox, MousePosition))
    {
        if (LeftMouseEndedDown)
        {
            SmileyDimensionsInner = V2iPlusV2i(SmileyDimensionsInner, V2i(2, 2));
            SmileyBorderWidthInner = 1;
            SmileyColorTopLeft = DarkGray;
            SmileyColorMid = DarkGray;
            SmileyColorBottomRight = Gray;
            SmileyPosition = V2iPlusV2i(ScorePosition, V2i(5, 5));
        }
        if (LeftReleaseCount)
        {
            Reset(GameInput, GameState);
        }
    }
    DrawBorder(Backbuffer, SmileyDimensionsInner, SmileyBorderWidthInner, ScorePosition, SmileyColorTopLeft, SmileyColorMid, SmileyColorBottomRight);

    ScorePosition = V2i(ScoreOrigin.X + ScoreBackgroundWidth - 5 - 1 - ScoreDimensions.Width, ScoreOrigin.Y + 4);
    Indent = DrawBorder(Backbuffer, ScoreDimensions, 1, ScorePosition, DarkGray, Gray, White);
    ScorePosition.X += Indent;
    ScorePosition.Y += Indent;
    DrawRectangle(Backbuffer, Rect2i(ScorePosition, ScoreDimensions), ScoreBackgroundColor, draw_flags_None);
    struct v2i TimerPosition = ScorePosition;
    Position.Y += 33 + 4 + 6;
    Indent = DrawBorder(Backbuffer, V2i(SquareWidth * FieldDimensions.Width, SquareWidth * FieldDimensions.Height), 3, Position, DarkGray, Gray, White);
    Position.X += Indent;
    Position.Y += Indent;

    s32 TilesRemaining = 0;
    enum smiley_state SmileyState = smiley_state_Normal;
    for (s32 j = 0; j < FieldDimensions.Height; ++j) {
        for (s32 i = 0; i < FieldDimensions.Width; ++i) {
            struct v2i FieldCoordinate = {i+1, j+1};
            u8 TileValue = GameState->Field[FieldCoordinate.Y][FieldCoordinate.X];
            struct minesweeper_tile_state TileState = ExtractMinesweeperTileState(TileValue);
            struct v2i TilePosition = {Position.X + i * SquareWidth, Position.Y + j * SquareWidth};
            struct rect2i TileRectangle = {TilePosition, {innerWidth + borderWidth + borderWidth, innerWidth + borderWidth + borderWidth}};
            s32 Hover = IsGameFocused && RectangleContains(TileRectangle, MousePosition);
            if (GameState->GameplayState == minesweeper_gameplay_state_Playing && TileState.UserState != tile_user_state_Revealed && Hover)
            {
                if (LeftReleaseCount)
                {
                    if (TileState.UserState == tile_user_state_Flagged)
                    {
                    }
                    else if (TileState.IsMine)
                    {
                        GameState->TriggeredCoordinate = FieldCoordinate;
                        GameState->GameEnd = ElapsedSeconds;
                        GameState->GameplayState = minesweeper_gameplay_state_GameOver;
                    }
                    else
                    {
                        TileState.UserState = tile_user_state_Revealed;
                        FloodFill(GameState, &ScratchArena, FieldCoordinate);
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
                            GameState->FlagsRemaining--;
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
            TilesRemaining += TileState.UserState != tile_user_state_Revealed;
            TilesRemaining -= TileState.IsMine;

            DrawTile(Backbuffer, innerWidth, borderWidth, TilePosition, GameState, TileState, FieldCoordinate);
        }
    }
    if (GameState->GameplayState == minesweeper_gameplay_state_Playing && !TilesRemaining)
    {
        GameState->GameEnd = ElapsedSeconds;
        GameState->GameplayState = minesweeper_gameplay_state_Victorious;
    }

    float TimerEnd = ElapsedSeconds;
    switch (GameState->GameplayState)
    {
        case minesweeper_gameplay_state_Victorious:
        {
            SmileyState = smiley_state_Cool;
            TimerEnd = GameState->GameEnd;
        } break;
        case minesweeper_gameplay_state_GameOver:
        {
            SmileyState = smiley_state_Frowney;
            TimerEnd = GameState->GameEnd;
        } break;
        default:
        {
        } break;
    }
    s32 GameTimer = (s32)(TimerEnd - GameState->GameStart);
    DrawSevenSegment(Backbuffer, TimerPosition, GameTimer);
    DrawSmiley(Backbuffer, SmileyPosition, SmileyState);

    if (GameState->IsGameMenuOpen)
    {
        s32 MenuItemVerticalPadding = 2;
        struct v2i MenuItemDimensions = {16*8 + 10, Layout.Font.Ascent + Layout.Font.Descent + 2*MenuItemVerticalPadding};
        struct rect2i DropdownRectangle = TitleBarRectangle;
        DropdownRectangle.Position.Y += TitleBarRectangle.Dimensions.Height;
        DropdownRectangle.Dimensions.Width = MenuItemDimensions.Width + 2;
        DropdownRectangle.Dimensions.Height = 2;
        struct menu_item MenuItems[] = {
            { menu_item_type_New, StringReference("New") },
            { menu_item_type_Separator },
            { menu_item_type_Beginner, StringReference("Beginner") },
            { menu_item_type_Intermediate, StringReference("Intermediate") },
            { menu_item_type_Expert, StringReference("Expert") },
#if 0
            { menu_item_type_Custom, StringReference("Custom...") },
#endif
        };

        s32 MenuItemCount = ArrayCount(MenuItems);
        while (MenuItemCount--)
        {
            s32 MenuItemHeight = MenuItemDimensions.Height;
            if (MenuItems[MenuItemCount].Type == menu_item_type_Separator)
            {
                MenuItemHeight = 1 + 2*MenuItemVerticalPadding;
            }
            DropdownRectangle.Dimensions.Height += MenuItemHeight;
        }
        
        DrawRectangle(Backbuffer, DropdownRectangle, V4(0, 0, 0, 1), 0);
        DropdownRectangle.Position.X += 1;
        DropdownRectangle.Position.Y += 1;
        DropdownRectangle.Dimensions.Width -= 2;
        DropdownRectangle.Dimensions.Height -= 2;
        DrawRectangle(Backbuffer, DropdownRectangle, V4(1, 1, 1, 1), 0);
        struct rect2i MenuItemRectangle = DropdownRectangle;
        MenuItemRectangle.Dimensions = MenuItemDimensions;

        struct menu_item *MenuItem = MenuItems;
        MenuItemCount = ArrayCount(MenuItems);
        while (MenuItemCount--)
        {
            if (MenuItem->Type == menu_item_type_Separator)
            {
                struct rect2i SeparatorRectangle = MenuItemRectangle;
                SeparatorRectangle.Position.Y += MenuItemVerticalPadding;
                SeparatorRectangle.Dimensions.Height = 1;
                DrawRectangle(Backbuffer, SeparatorRectangle, V4(0, 0, 0, 1), 0);
                MenuItemRectangle.Position.Y += SeparatorRectangle.Dimensions.Height + MenuItemVerticalPadding*2;
            }
            else
            {
                if (RectangleContains(MenuItemRectangle, MousePosition))
                {
                    struct v4 MenuItemColor = V4(0, 0, 1, 1);
                    if (LeftMouseEndedDown)
                    {
                        MenuItemColor.Blue *= 0.5f;
                    }
                    DrawRectangle(Backbuffer, MenuItemRectangle, MenuItemColor, 0);
                    Layout.Color = V4(1, 1, 1, 1);

                    if (LeftReleaseCount)
                    {
                        switch (MenuItem->Type)
                        {
                            case menu_item_type_New:
                            {
                                Reset(GameInput, GameState);
                                GameState->IsGameMenuOpen = 0;
                            } break;
                            case menu_item_type_Beginner:
                            {
                                GameState->DesiredFieldDimensions.X = 9;
                                GameState->DesiredFieldDimensions.Y = 9;
                                GameState->DesiredMineCount = 10;
                                Reset(GameInput, GameState);
                                GameState->IsGameMenuOpen = 0;
                            } break;
                            case menu_item_type_Intermediate:
                            {
                                GameState->DesiredFieldDimensions.X = 16;
                                GameState->DesiredFieldDimensions.Y = 16;
                                GameState->DesiredMineCount = 40;
                                Reset(GameInput, GameState);
                                GameState->IsGameMenuOpen = 0;
                            } break;
                            case menu_item_type_Expert:
                            {
                                GameState->DesiredFieldDimensions.X = 30;
                                GameState->DesiredFieldDimensions.Y = 16;
                                GameState->DesiredMineCount = 99;
                                Reset(GameInput, GameState);
                                GameState->IsGameMenuOpen = 0;
                            } break;
                            case menu_item_type_Custom:
                            {
                            } break;
                            default:
                            {
                            } break;
                        }
                    }
                }
                else
                {
                    Layout.Color = V4(0, 0, 0, 1);
                }
                Layout.Position = MenuItemRectangle.Position;
                Layout.Position.Y += MenuItemVerticalPadding;
                if (0 <= MenuItem->ShortcutCharIndex && MenuItem->ShortcutCharIndex < MenuItem->Name.Length)
                {
                    DrawText(Backbuffer, &Layout, Substring(MenuItem->Name, 0, MenuItem->ShortcutCharIndex), 0);
                    DrawText(Backbuffer, &Layout, Substring(MenuItem->Name, MenuItem->ShortcutCharIndex, 1), text_flags_Underline);
                    DrawText(Backbuffer, &Layout, Substring(MenuItem->Name, MenuItem->ShortcutCharIndex + 1, -1), 0);
                }
                else
                {
                    DrawText(Backbuffer, &Layout, MenuItem->Name, 0);
                }
                MenuItemRectangle.Position.Y += MenuItemDimensions.Height;
            }
            MenuItem++;
        }

        if (LeftMouseHalfTransitionCount && !RectangleContains(DropdownRectangle, MousePosition) && !RectangleContains(MenuButtonHitbox, MousePosition))
        {
            GameState->IsGameMenuOpen = 0;
        }
    }
#endif
}
