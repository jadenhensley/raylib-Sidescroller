#include "chunkmanager.h"

// Discloser: this was made with Google Gemini

Chunk* ChunkManager::GetChunk(int chunkX, int chunkY) {
    int16_t sectorX = chunkX >> 8;
    int16_t sectorY = chunkY >> 8;
    uint32_t sectorKey = PackSectorKey(sectorX, sectorY);

    Sector* targetSector = nullptr;
    if (cachedSector && sectorKey == cachedSectorKey) {
        targetSector = cachedSector;
    }
    else {
        auto it = sectors.find(sectorKey);
        if (it != sectors.end()) {
            targetSector = it->second.get();
        }
        else {
            // Instantly allocates the entire 17.5 MB contiguous sector block
            sectors[sectorKey] = std::make_unique<Sector>();
            targetSector = sectors[sectorKey].get();
        }
        cachedSector = targetSector;
        cachedSectorKey = sectorKey;
    }

    int localX = chunkX & 255;
    int localY = chunkY & 255;
    int localIndex = (localX << 8) | localY;

    // Get a direct reference to the chunk sitting inside the flat array
    Chunk& chunk = targetSector->chunks[localIndex];

    // Lazily initialize data if it's the first time visiting
    if (!chunk.active) {
        chunk.x = chunkX;
        chunk.y = chunkY;
        chunk.active = true;
        chunk.debugColor = { (unsigned char)GetRandomValue(50, 200),
                                    (unsigned char)GetRandomValue(50, 200),
                                    (unsigned char)GetRandomValue(50, 200), 255 };
        // Generate your tile layout here...
    }

    return &chunk;
}
