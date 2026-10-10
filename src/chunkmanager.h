// C++ standard library
#include <array>
#include <memory>
#include <unordered_map>
// Third-Party libraries
#include "raylib.h"

// Discloser: this was made with Google Gemini

const int TILE_SIZE = 16;
const int CHUNK_WIDTH = 16;
const int SECTOR_WIDTH = 256;

struct Chunk
{
    // 16x16 grid of tile IDs. 
    // Packed tightly so the whole chunk is exactly 256 bytes!
    int x = 0;
    int y = 0;
    bool active = false;
    bool generated = false;
    uint8_t tiles[CHUNK_WIDTH * CHUNK_WIDTH] = { 0 };
    Color debugColor = BLACK;
};

struct Sector
{
    // 256 * 256 * ~268 bytes = ~17.5 MB allocated in ONE continuous block of memory.
    // Zero pointer chasing inside this array.
    std::array<Chunk, SECTOR_WIDTH* SECTOR_WIDTH> chunks;
};

class ChunkManager
{
private:
    std::unordered_map<uint32_t, std::unique_ptr<Sector>> sectors;
    uint32_t cachedSectorKey = 0;
    Sector* cachedSector = nullptr;

    uint32_t PackSectorKey(int16_t x, int16_t y) const
    {
        return ((uint32_t)(uint16_t)x << 16) | ((uint32_t)(uint16_t)y & 0xFFFF);
    }

public:
    Chunk* GetChunk(int chunkX, int chunkY);
};