struct memory_arena
{
    size_t Size;
    size_t Used;
    void *Memory;
};

void
InitializeArena(struct memory_arena *Arena, size_t SizeInit, void *MemoryInit)
{
    Arena->Size = SizeInit;
    Arena->Used = 0;
    Arena->Memory = MemoryInit;
}

enum push_flags
{
    None = 0,
    PushFlag_Zero = 1 << 0,
};

b32
HasRoom(struct memory_arena *Arena, size_t Size, u32 Flags)
{
    b32 Result = ((Arena->Used + Size) <= Arena->Size);
    return(Result);
}

#define HasRoomForStruct(Arena, type, ...) HasRoom(Arena, sizeof(type), ##__VA_ARGS__)
#define HasRoomForArray(Arena, Count, type, ...) HasRoom(Arena, Count*sizeof(type), ##__VA_ARGS__)

b32 IsSet(u32 Flags, u32 Bit)
{
    b32 Result = (Flags & Bit);
    return(Result);
}

void *
PushSize(struct memory_arena *Arena, size_t Size, u32 Flags)
{
    void *Result = 0;
    if((Arena->Used + Size) > Arena->Size)
    {
        Assert(!"Allocated too much memory!");
    }
    else
    {
        Result = (u8 *)Arena->Memory + Arena->Used;
        Arena->Used += Size;
        if(IsSet(Flags, PushFlag_Zero))
        {
            u8 *End = (u8 *)Arena->Memory + Arena->Used;
            for(u8 *At = (u8 *)Result;
                At < End;
                ++At)
            {
                *At = 0;
            }
        }
    }

    return(Result);
}

#define PushStruct(Arena, type, ...) ((type *)PushSize(Arena, sizeof(type), ##__VA_ARGS__))
#define PushArray(Arena, Count, type, ...) ((type *)PushSize(Arena, Count*sizeof(type), ##__VA_ARGS__))

struct memory_arena
SubArena(struct memory_arena *Arena, size_t Size)
{
    void *Memory = PushSize(Arena, Size, 0);
    struct memory_arena Result;
    InitializeArena(&Result, Size, Memory);
    return(Result);
}

struct temporary_memory
{
    size_t Used;
    struct memory_arena *Arena;
};

struct temporary_memory
BeginTemporaryMemory(struct memory_arena *Arena)
{
    struct temporary_memory Result;
    Result.Used = Arena->Used;
    Result.Arena = Arena;
    return(Result);
}

void
EndTemporaryMemory(struct temporary_memory Temp)
{
    Temp.Arena->Used = Temp.Used;
}
