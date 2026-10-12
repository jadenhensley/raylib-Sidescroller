// C standard library
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"
// C++ standard library
#include <iostream>
#include <vector>
#include <utility> // for std::pair
#include <unordered_map>
#include <memory>
// Third-Party libraries
#include "raylib.h"
#include "raymath.h"
#include "src/PerlinNoise.hpp"
// My headers
#include "src/timer.h"
#include "src/util_printui.h"
#include "src/chunkmanager.h"


#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 400

enum class TEXTUREID : uint8_t {
    CHARSKIN,
    TERRAIN,
    COUNT
};

struct Dimensions {
    int width;
    int height;
};

static const Dimensions lookupTextureDimensions[(size_t)TEXTUREID::COUNT] = {
    { 4,1 }, // CHARSKIN
    { 2,5 }, // TERRAIN
};

static enum class TEXTUREINDEX_TERRAIN : uint8_t {
    GRASS_VAR1 = 0,
    GRASS_VAR2 = 1,
    DIRT_VAR1 = 2,
    DIRT_VAR2 = 3,
    STONE_VAR1 = 4,
    STONE_VAR2 = 5,
    SAND_VAR1 = 6,
    SAND_VAR2 = 7,
    SNOW_VAR1 = 8,
    SNOW_VAR2 = 9,
    COUNT = 10,
};

static enum class ATLASID : uint8_t {
    NIL = 0,
    CHARSKIN = 1,
    TERRAIN = 2,
    COUNT = 3,
};

struct TileTexture
{
    ATLASID atlas;
    std::pair<uint8_t, uint8_t> textureIndex;
};

static enum class TILEID : uint8_t {
    AIR = 0,
    DIRT_VAR1 = 1,
    DIRT_VAR2 = 2,
    GRASS_VAR1 = 3,
    GRASS_VAR2 = 4,
    STONE_VAR1 = 5,
    STONE_VAR2 = 6,
    SAND_VAR1 = 7,
    SAND_VAR2 = 8,
    SNOW_VAR1 = 9,
    SNOW_VAR2 = 10,
    COUNT = 11,
};

static const TileTexture lookupTileTexture[(size_t)TILEID::COUNT] = {
    { ATLASID::NIL, {0,0} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR1 , (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR2 , (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR1 , (uint8_t)TEXTUREINDEX_TERRAIN::GRASS_VAR1} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR2 , (uint8_t)TEXTUREINDEX_TERRAIN::GRASS_VAR2} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::STONE_VAR1, (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::STONE_VAR2, (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::SAND_VAR1 , (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::SAND_VAR2 , (uint8_t)TEXTUREINDEX_TERRAIN::COUNT} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR1 , (uint8_t)TEXTUREINDEX_TERRAIN::SNOW_VAR1} },
    { ATLASID::TERRAIN, {(uint8_t)TEXTUREINDEX_TERRAIN::DIRT_VAR2 , (uint8_t)TEXTUREINDEX_TERRAIN::SNOW_VAR2} },
};

enum CAMERAMODE {
    CAMERAMODE_FOLLOWPLAYER,
    CAMERAMODE_FREEROAM
};


void DrawTextureAtAtlasIndex(Texture& txt, Vector2 pos, const int& idx, const Dimensions& dim, const int& tileSize = 0)
{
    //if (idx == dim.width * dim.height)
    //{
    //    printf("NOT THIS \n");
    //    // it is too big, ignore it, so we dont wrap around to another texture if we want to use COUNT as a placeholder for NIL.
    //    // note: we use == because it is 0-indicing.
    //    return;
    //}
    Rectangle source{};
    int col = idx % dim.width;
    int row = idx / dim.width;
    int normIdx = row * dim.width + col;
    bool isValid = (normIdx >= 0 && normIdx < dim.width * dim.height);
    int sprWid = txt.width / dim.width;
    int sprHgt = txt.height / dim.height;
    // height is still useful for checking validity.
    source.x = col * sprWid;
    source.y = row * sprHgt;
    source.width = sprWid;
    source.height = sprHgt;

    Rectangle dest{};
    dest.x = pos.x;
    dest.y = pos.y;
    if (tileSize == 0)
    {
        dest.width = sprWid;
        dest.height = sprHgt;
    }
    else if (tileSize > 0)
    {
        dest.width = tileSize;
        dest.height = tileSize;
    }

    DrawTexturePro(txt, source, dest, { 0.0f,0.0f }, 0.0f, WHITE);
}

int main(int argc, char* argv[])
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Macaroni");
    SetTargetFPS(60);
    srand(time(NULL));

    Vector2 cameraPos{};
    Camera2D camera = { 0 };
    camera.target = cameraPos; // free roam position
    camera.offset.x = SCREEN_WIDTH / 2.0f; 
    camera.offset.y = SCREEN_HEIGHT / 2.0f; // Center on screen
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;
    CAMERAMODE cameraMode = CAMERAMODE_FREEROAM;

    ChunkManager chunkManager = {};

    Texture ATLAS_CHARSKIN = LoadTexture("./assets/atlas_characterSkin_4x.png");
    Texture ATLAS_TERRAIN = LoadTexture("./assets/atlas_terrainSheet_2x5.png");




    static bool isRunning = true;
    static float elapsed = 0.0f;
    while (!WindowShouldClose() && isRunning)
    {
        float dt = GetFrameTime();
        elapsed += dt;

        static Vector2 playerPos = { 0.0f,0.0f };
        
        static Timer timerOnClickCooldown;
        static bool isClickPressedThisFrame = false;
        /* Click Feedback */ {
            timerOnClickCooldown.setDuration(dt * 3.0f); // i.e. 3 frames for each click. guesstimate. Meaning: 20 clicks per second.
        
            timerOnClickCooldown.tick();

            isClickPressedThisFrame = false;
            if (timerOnClickCooldown.hasEnded())
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    isClickPressedThisFrame = true;
                    timerOnClickCooldown.start();
                }
            }

            if (isClickPressedThisFrame)
            {
                printf("Clicked at: %f \n", elapsed);
                printf("Cooldown: %f \n", dt * 3.0f);
            }
        }

        /* Camera Feedback */ {
            camera.zoom += ((float)GetMouseWheelMove() * 0.05f);
            if (camera.zoom < 0.1f) camera.zoom = 0.1f;

            if (IsKeyDown(KEY_SPACE) && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                CAMERAMODE lastMode = cameraMode;
                cameraMode = CAMERAMODE_FREEROAM;
                if (lastMode == CAMERAMODE_FOLLOWPLAYER)
                {
                    cameraPos.x = playerPos.x;
                    cameraPos.y = playerPos.y;
                }
                Vector2 mouseDelta = GetMouseDelta();
                mouseDelta = Vector2Scale(mouseDelta, -1.0f / camera.zoom);
                cameraPos.x += mouseDelta.x;
                cameraPos.y += mouseDelta.y;
            }
        }

        static const Color BG_CLR = SKYBLUE;
        BeginDrawing();
        ClearBackground(BG_CLR);

        BeginMode2D(camera);
        camera.target = cameraPos; // always assign.

        int inputDirX = (int)(IsKeyDown(KEY_RIGHT)) - (int)(IsKeyDown(KEY_LEFT));
        int inputDirY = (int)(IsKeyDown(KEY_DOWN)) - (int)(IsKeyDown(KEY_UP));
        static const float MOVEMENT_SPEED = 320.0f;
        const float MOVEMENT_DELTA_X = inputDirX * MOVEMENT_SPEED * dt;
        const float MOVEMENT_DELTA_Y = inputDirY * MOVEMENT_SPEED * dt;
        playerPos.x += MOVEMENT_DELTA_X;
        playerPos.y += MOVEMENT_DELTA_Y;

        bool playerIsMoving = (inputDirX != 0 || inputDirY != 0);
        if (playerIsMoving)
        {
            cameraMode = CAMERAMODE_FOLLOWPLAYER;
        }

        switch (cameraMode)
        {
        case CAMERAMODE_FREEROAM:
            camera.target = cameraPos;
            break;
        case CAMERAMODE_FOLLOWPLAYER:
            camera.target = playerPos;
            break;
        }

        
        const int CHUNK_WORLD_WIDTH = TILE_SIZE * CHUNK_WIDTH;
        int playerChunkX = playerPos.x / CHUNK_WORLD_WIDTH;
        int playerChunkY = playerPos.y / CHUNK_WORLD_WIDTH;
        const int SPAWN_CHUNK_X = 0;
        const int SPAWN_CHUNK_Y = 0;
        const int CHUNK_LOADING_RADIUS_X = 8;
        const int CHUNK_LOADING_RADIUS_Y = 1;
        const int *centerChunkX = &playerChunkX;
        const int *centerChunkY = &playerChunkY;
        const int minChunkLoadX = *centerChunkX - CHUNK_LOADING_RADIUS_X;
        const int maxChunkLoadX = *centerChunkX + CHUNK_LOADING_RADIUS_X;
        
        const int minChunkLoadY = *centerChunkY - 1;
        const int maxChunkLoadY = *centerChunkY + 4;
        for (int currChunkX = minChunkLoadX; currChunkX <= maxChunkLoadX; ++currChunkX)
        {
            for (int currChunkY = minChunkLoadY; currChunkY <= maxChunkLoadY; ++currChunkY)
            {
                Chunk* ptrCurrChunk = chunkManager.GetChunk(currChunkX, currChunkY);
                if (ptrCurrChunk != nullptr)
                {
                    // const int CHUNK_WORLD_WIDTH = TILE_SIZE * CHUNK_WIDTH;
                    DrawRectangle(
                        ptrCurrChunk->x* CHUNK_WORLD_WIDTH,
                        ptrCurrChunk->y* CHUNK_WORLD_WIDTH,
                        CHUNK_WORLD_WIDTH,
                        CHUNK_WORLD_WIDTH, ptrCurrChunk->debugColor);

                    if (!ptrCurrChunk->generated)
                    {

                        auto getRandDirtIndex = []() -> uint8_t
                            {
                                int r = rand() % 2;
                                switch (r)
                                {
                                case 0:
                                    return (uint8_t)TILEID::DIRT_VAR1;
                                case 1:
                                    return (uint8_t)TILEID::DIRT_VAR2;
                                }
                            };

                        auto getRandStoneIndex = []() -> uint8_t
                            {
                                int r = rand() % 2;
                                switch (r)
                                {
                                case 0:
                                    return (uint8_t)TILEID::STONE_VAR1;
                                case 1:
                                    return (uint8_t)TILEID::STONE_VAR2;
                                }
                            };

                        auto getRandGrassIndex = []() -> uint8_t
                            {
                                int r = rand() % 2;
                                switch (r)
                                {
                                case 0:
                                    return (uint8_t)TILEID::GRASS_VAR1;
                                case 1:
                                    return (uint8_t)TILEID::GRASS_VAR2;
                                }
                            };

                        auto getRandSnowIndex = []() -> uint8_t
                            {
                                int r = rand() % 2;
                                switch (r)
                                {
                                case 0:
                                    return (uint8_t)TILEID::SNOW_VAR1;
                                case 1:
                                    return (uint8_t)TILEID::SNOW_VAR2;
                                }
                            };

                        auto getRandSandIndex = []() -> uint8_t
                            {
                                int r = rand() % 2;
                                switch (r)
                                {
                                case 0:
                                    return (uint8_t)TILEID::SAND_VAR1;
                                case 1:
                                    return (uint8_t)TILEID::SAND_VAR2;
                                }
                            };

                        static const siv::PerlinNoise::seed_type seed = 123456u;
                        static const siv::PerlinNoise perlin{ seed };

                        for (int posX = 0; posX < CHUNK_WIDTH; ++posX)
                        {
                            const int worldX = (ptrCurrChunk->x * CHUNK_WIDTH) + posX;
                            double noise = perlin.noise1D(worldX / 8.0f);
                    
                            int elevation = 8 + (int)(noise * 7);
                            // (8-7=1)...(8+7=15)
                            if (ptrCurrChunk->y == 0)
                            {
                                // assign dirt or sand
                                for (int posY = 0; posY < elevation; ++posY)
                                {
                                    if (posY >= 0 && posY < CHUNK_WIDTH)
                                    {
                                        //printf("[%d]", CHUNK_WIDTH - 1 - posY);
                                        if (ptrCurrChunk->biome == (uint8_t)BIOME::GRASSLAND
                                            || ptrCurrChunk->biome == (uint8_t)BIOME::SNOWLAND)
                                        {
                                            ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandDirtIndex();
                                        }
                                        else
                                        {
                                            if (ptrCurrChunk->biome == (uint8_t)BIOME::SANDLAND)
                                            {
                                                ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandSandIndex();
                                            }
                                        }
                                    }
                                    // int elevation = 12 + (int)(noise * 8);
                                    // Height ranges from 4 to 20
                                    // (12-8)...(12+8)
                                }
                                // add grass or snow.
                                int posY = elevation;
                                if (ptrCurrChunk->biome == (uint8_t)BIOME::GRASSLAND)
                                {
                                    ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandGrassIndex();
                                }
                                if (ptrCurrChunk->biome == (uint8_t)BIOME::SNOWLAND)
                                {
                                    ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandSnowIndex();
                                }
                            }
                            if (ptrCurrChunk->y > 0)
                            {
                                for (int posY = 0; posY < CHUNK_WIDTH; ++posY)
                                {
                                    ptrCurrChunk->tiles[(posY * CHUNK_WIDTH) + posX] = getRandStoneIndex();
                                }
                            }

                        }

                        ptrCurrChunk->generated = true;
                    }
                    else
                    {
                        // chunk has been initialized
                        for (int posY = 0; posY < CHUNK_WIDTH; ++posY)
                        {
                            for (int posX = 0; posX < CHUNK_WIDTH; ++posX)
                            {
                                // world chunk Y
                                // (int)floor((double)worldY / CHUNK_HEIGHT);

                                // e.g. world pos 
                                float atPosX = (float)(int)(ptrCurrChunk->x * CHUNK_WORLD_WIDTH) + (posX * TILE_SIZE);
                                float atPosY = (float)(int)(ptrCurrChunk->y * CHUNK_WORLD_WIDTH) + (posY * TILE_SIZE);

                                // todo(jaden): fix visual bug involving rendering of distant chunks.


                                TileTexture currTexture = lookupTileTexture[ptrCurrChunk->tiles[posY * CHUNK_WIDTH + posX]];
                                Texture* ptrCurrAtlas = nullptr;
                                if (currTexture.atlas == ATLASID::TERRAIN)
                                {
                                    ptrCurrAtlas = &ATLAS_TERRAIN;
                                }
                                // uint8_t textureIndex = currTexture.textureIndex;
                                if (ptrCurrAtlas != nullptr && currTexture.atlas != ATLASID::NIL)
                                {
                                    DrawTextureAtAtlasIndex(
                                        *ptrCurrAtlas,
                                        { atPosX,atPosY },
                                        (int)currTexture.textureIndex.first,
                                        lookupTextureDimensions[(int)TEXTUREID::TERRAIN],
                                        TILE_SIZE
                                    );
                                    if (currTexture.textureIndex.first != currTexture.textureIndex.second)
                                    {
                                        auto isSecondTextureNil = [currTexture]() -> bool
                                            {
                                                if (currTexture.atlas == ATLASID::TERRAIN)
                                                {
                                                    if (currTexture.textureIndex.second == (uint8_t)TEXTUREINDEX_TERRAIN::COUNT)
                                                    {
                                                        // We treat COUNT as NIL.
                                                        // Do this for every texture atlas if this is how its designed.
                                                        return true;
                                                    }
                                                }
                                                return false;
                                            };
                                        bool nilFound = isSecondTextureNil();
                                        if (!nilFound)
                                        {
                                            DrawTextureAtAtlasIndex(
                                                *ptrCurrAtlas,
                                                { atPosX,atPosY },
                                                (int)currTexture.textureIndex.second,
                                                lookupTextureDimensions[(int)TEXTUREID::TERRAIN],
                                                TILE_SIZE
                                            );
                                        }
                                    }
                                }
                                // assert(textureIndex != 3, "equals four.");
                            }
                        }
                    }

                }
            }
        }

        DrawCircle((int)playerPos.x, (int)playerPos.y, 8.0f, RED);

        EndMode2D();

        static util_printui ui;
        ui.resetui();
        ui.setFontSize(36);
        ui.printui("Coordinates");
        ui.setFontSize(24);
        ui.printui(TextFormat("Player Chunk X: %d", playerChunkX));
        ui.printui(TextFormat("Player Chunk Y: %d", playerChunkY));

        static bool F3_TOGGLE = false;
        if (IsKeyPressed(KEY_F3))
        {
            F3_TOGGLE = !F3_TOGGLE;
        }
        /* Debug User Interface. */ if (F3_TOGGLE) {
            static util_printui ui;
            static const int UI_OURFONTSIZE = 24;
            static const char* UI_GAMEVERSION = "pre-alpha (0.0.0)";
            ui.resetui();
            ui.uiOffsetX += 10;

            ui.setColor(BLACK);
            ui.setFontSize(24);
            ui.printui("Project Name Here");
            ui.printui(UI_GAMEVERSION);
            
            ui.setFontSize(UI_OURFONTSIZE);
        
            // draw a line.
            ui.advanceByRect({ 0.0f,0.0f,0.0f,(float)UI_OURFONTSIZE / 2});
            DrawLineEx({ (float)ui.uiOffsetX, (float)ui.uiOffsetY }, { (float)ui.uiOffsetX + ui.uiLongestWidth, (float)ui.uiOffsetY }, 4.0f, BLACK);
            ui.advanceByRect({ 0.0f,0.0f,0.0f, (float)UI_OURFONTSIZE / 2});
        
        
            ui.printui("Elapsed Time: ");
            ui.printui(elapsed);
        
            // draw a line.
            ui.advanceByRect({ 0.0f,0.0f,0.0f,(float)UI_OURFONTSIZE / 2 });
            DrawLineEx({ (float)ui.uiOffsetX, (float)ui.uiOffsetY }, { (float)ui.uiOffsetX + ui.uiLongestWidth, (float)ui.uiOffsetY }, 4.0f, BLACK);
            ui.advanceByRect({ 0.0f,0.0f,0.0f,(float)UI_OURFONTSIZE / 2 });

            ui.printui("Delta Time: (frame time):");
            ui.printui(dt);

            ui.bumpui();
        }

        EndDrawing();
    }
    CloseWindow();
    isRunning = false;

    return 0;
}