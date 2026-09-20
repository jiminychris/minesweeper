#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define Assert(Expr) {if(!(Expr)) {int __AssertInt = *((volatile int *)0);}}

int main()
{
    size_t MemorySize = 1024*1024*1024;
    u8 *Memory = (u8 *)malloc(MemorySize);
    FILE *BackgroundFile = fopen("background.ppm", "r");
    size_t BackgroundFileSize = fread(Memory, 1, MemorySize, BackgroundFile);
    u8 *At = Memory;
    Assert(*At++ == 'P');
    Assert(*At++ == '6');
    Assert(*At++ == '\n');
    while (*At == '#')
    {
        while (*At++ != '\n');
    }
    u32 Width = 0;
    u32 Height = 0;
    u8 Byte;
    while ((Byte = *At++) != ' ')
    {
        Width = Width * 10 + Byte - '0';
    }
    while ((Byte = *At++) != '\n')
    {
        Height = Height * 10 + Byte - '0';
    }
    while (*At++ != '\n');
    FILE *AssetFile = fopen("assets.msp", "w");
    fwrite(&Width, sizeof(Width), 1, AssetFile);
    fwrite(&Height, sizeof(Height), 1, AssetFile);
    fwrite(At, 1, Width * Height * 3, AssetFile);
    return 0;
}
