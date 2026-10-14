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

static enum class TEXTUREINDEX_PLANTS : uint8_t {
    CACTUS = 0,
    TREE_OAK = 1,
    TREE_SPRUCE = 2,
    COUNT = 3,
};

static enum class ATLASID : uint8_t {
    NIL = 0,
    CHARSKIN = 1,
    TERRAIN = 2,
    PLANTS = 3,
    COUNT = 4,
};

struct TileTexture
{
    ATLASID atlas;
    std::pair<uint8_t, uint8_t> textureIndex;
};

static enum class TILEID : uint8_t {
    // BLOCK TILES
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
    // SPECIAL TILES
    CACTUS = 11,
    TREE_OAK = 12,
    TREE_SPRUCE = 13,
    COUNT = 14,
};

struct Dimensions {
    int width;
    int height;
};

static const Dimensions lookupTextureAtlasDimensions[(size_t)ATLASID::COUNT] = {
    { 0,0 }, // NIL
    { 4,1 }, // CHARSKIN
    { 2,5 }, // TERRAIN
    { 3,1 }, // PLANTS
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
    { ATLASID::PLANTS, {(uint8_t)TEXTUREINDEX_PLANTS::CACTUS      , (uint8_t)TEXTUREINDEX_PLANTS::COUNT} }, // CACTUS
    { ATLASID::PLANTS, {(uint8_t)TEXTUREINDEX_PLANTS::TREE_OAK    , (uint8_t)TEXTUREINDEX_PLANTS::COUNT} }, // OAK TREE
    { ATLASID::PLANTS, {(uint8_t)TEXTUREINDEX_PLANTS::TREE_SPRUCE , (uint8_t)TEXTUREINDEX_PLANTS::COUNT} }, // SPRUCE TREE
};

enum CAMERAMODE {
    CAMERAMODE_FOLLOWPLAYER,
    CAMERAMODE_FREEROAM
};


void DrawTextureAtAtlasIndex(Texture& txt, Vector2 pos, const int& idx, const Dimensions& dim, const int& tileSize = 0)
{
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

typedef std::string ActionName;
struct InputState
{
    int code = 0;
    bool isDown = false;
    bool isPressed = false;
    bool isReleased = false;
    bool isUp = true;
};


class ActionInputSystem
{
private:
    std::unordered_map<ActionName, InputState> actions;
public:
    void AddOrSetAction(ActionName argActionName, int argCode)
    {
        InputState state;
        state.code = argCode;
        actions[argActionName] = state;
    }
    void Update()
    {
        for (auto& [key, value] : actions)
        {
            value.isDown = IsKeyDown(value.code);
            value.isPressed = IsKeyPressed(value.code);
            value.isReleased = IsKeyReleased(value.code);
            value.isUp = IsKeyUp(value.code);
        }
    }
    InputState* GetInputStateFromAction(ActionName argActionName)
    {
        auto it = actions.find(argActionName);
        if (it != actions.end())
        {
            return &actions.at(argActionName);
        }
        else
        {
            return nullptr;
        }
    }
};

enum class RaycastAxis {
    RAYCAST_AXISX,
    RAYCAST_AXISY
};

enum class RaycastDirection : int {
    RAYCAST_LEFT = -1,
    RAYCAST_RIGHT = 1,
    RAYCAST_UP = -1,
    RAYCAST_DOWN = 1
};

bool IsRaycastHitting(
    RaycastAxis axis,
    RaycastDirection direction,
    float rayOrigin,
    float targetDistance,
    Rectangle box,
    float& outDistance
) {
    float boxMin = 0.0f;
    float boxMax = 0.0f;
    if (axis == RaycastAxis::RAYCAST_AXISX)
    {
        boxMin = box.x;
        boxMax = box.x + box.width;
    }
    if (axis == RaycastAxis::RAYCAST_AXISY)
    {
        boxMin = box.y;
        boxMax = box.y + box.height;
    }

    float t1 = (boxMin - rayOrigin) / (float)direction;
    float t2 = (boxMax - rayOrigin) / (float)direction;

    float tNear = std::min(t1, t2);
    float tFar = std::max(t1, t2);

    if (tFar < 0.0f)
    {
        return false;
    }
    if (tFar > targetDistance)
    {
        return false;
    }
    // Passed both checks, so it must be true.
    outDistance = (tNear < 0.0f) ? 0.0f : tNear;
    return true;
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

    ActionInputSystem inputs;
    inputs.AddOrSetAction("PlayerWalkLeft", KEY_A);
    inputs.AddOrSetAction("PlayerWalkRight", KEY_D);
    inputs.AddOrSetAction("PlayerJump", KEY_SPACE);

    Texture ATLAS_CHARSKIN = LoadTexture("./assets/atlas_characterSkin_4x.png");
    Texture ATLAS_TERRAIN = LoadTexture("./assets/atlas_terrainSheet_2x5.png");
    Texture ATLAS_PLANTS = LoadTexture("./assets/atlas_plantSheet_3x1.png");

    static bool isRunning = true;
    static float elapsed = 0.0f;
    while (!WindowShouldClose() && isRunning)
    {
        float dt = GetFrameTime();
        elapsed += dt;

        inputs.Update();

        static Vector2 playerPos = { 0.0f,0.0f };
        static bool isPlayerTouchingGround = false;

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

        InputState* ptrActionWalkLeft = inputs.GetInputStateFromAction("PlayerWalkLeft");
        InputState* ptrActionWalkRight = inputs.GetInputStateFromAction("PlayerWalkRight");
        int inputDirX = (int)(ptrActionWalkRight ? ptrActionWalkRight->isDown : 0) - (int)(ptrActionWalkLeft ? ptrActionWalkLeft->isDown : 0);

        static const float MOVEMENT_SPEED = TILE_SIZE * 8.0f; // we want to walk 8 tiles in one second.
        static const float JUMP_FORCE = 20.0f;
        static const float FALLING_FORCE = 3.5f;
        const float MOVEMENT_DELTA_X = inputDirX * MOVEMENT_SPEED * dt;
        // const float MOVEMENT_DELTA_Y = inputDirY * MOVEMENT_SPEED * dt;
        InputState* ptrActionJump = inputs.GetInputStateFromAction("PlayerJump");
        
        playerPos.x += MOVEMENT_DELTA_X;
        
        static float yVel = 0.0f;
        //if (IsKeyReleased(KEY_ENTER))
        
        {
            if (!isPlayerTouchingGround)
            {
                if (yVel < FALLING_FORCE * 5)
                {
                    yVel += FALLING_FORCE;
                }
            }
            else
            {
                if (ptrActionJump && ptrActionJump->isDown)
                {
                    yVel = -1.0f * JUMP_FORCE; // is negative.
                }                
            }
            playerPos.y += yVel;
        }

        bool playerIsMoving = (inputDirX != 0);
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
        int playerChunkX = (int)floor( playerPos.x / CHUNK_WORLD_WIDTH );
        int playerChunkY = (int)floor( (float)playerPos.y / CHUNK_WORLD_WIDTH );
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

        Chunk* ptrCollisionChunk = chunkManager.GetChunk(playerChunkX, playerChunkY);
        if (ptrCollisionChunk != nullptr)
        {
            isPlayerTouchingGround = false;
            // handle collisions.
            for (int tilePosX = 0; tilePosX < CHUNK_WIDTH; tilePosX++)
            {
                for (int tilePosY = 0; tilePosY < CHUNK_WIDTH; tilePosY++)
                {
                    uint8_t* currTile = &ptrCollisionChunk->tiles[tilePosY * CHUNK_WIDTH + tilePosX];
                    auto tileIsCollidable = [currTile]() -> bool
                        {
                            if (*currTile == (uint8_t)TILEID::AIR
                                || *currTile == (uint8_t)TILEID::CACTUS
                                || *currTile == (uint8_t)TILEID::TREE_OAK
                                || *currTile == (uint8_t)TILEID::TREE_SPRUCE)
                            {
                                return false;
                            }
                            return true;
                        };
                    if (!tileIsCollidable()) continue;
                    Rectangle boundingBox;
                    boundingBox.x = (ptrCollisionChunk->x * CHUNK_WIDTH * TILE_SIZE) + (tilePosX * TILE_SIZE);
                    boundingBox.y = -TILE_SIZE + (ptrCollisionChunk->y * CHUNK_WIDTH * TILE_SIZE) + (tilePosY * TILE_SIZE);
                    boundingBox.width = TILE_SIZE;
                    boundingBox.height = TILE_SIZE;
                    float outDistance = 0.0f;
                    if (IsRaycastHitting(
                        RaycastAxis::RAYCAST_AXISY,
                        RaycastDirection::RAYCAST_DOWN,
                        playerPos.y,
                        yVel,
                        boundingBox,
                        outDistance
                    ))
                    {
                        int playerTileX = (int)floor(playerPos.x / TILE_SIZE);
                        int localTileX = playerTileX % CHUNK_WIDTH;
                        if (localTileX != tilePosX)
                        {
                            continue;
                        }
                        printf("\n=============\nPlayer colliding at tile = {%d, %d} in chunk = {%d, %d}",
                            tilePosX, tilePosY, ptrCollisionChunk->x, ptrCollisionChunk->y, outDistance);
                        printf("\nOut Distance = %f", outDistance);
                        printf("\nTile ID = %u", *currTile);
                        printf("\n=============\n");
                        isPlayerTouchingGround = true;
                    }
                }
            }
        }
        /*if (ptrCollisionChunk != nullptr)
        {
            printf("Collision Chunk = {%d, %d}", ptrCollisionChunk->x, ptrCollisionChunk->y);
        }*/

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

                        auto getRandPlantIndex = [](const BIOME& biomeType) -> uint8_t
                            {
                                bool doSpawn = (0 == (rand() % 10));
                                if (!doSpawn)
                                {
                                    return (uint8_t)TILEID::AIR;
                                }
                                else
                                {
                                    switch (biomeType)
                                    {
                                    case BIOME::GRASSLAND:
                                        return (uint8_t)TILEID::TREE_OAK;
                                    case BIOME::SNOWLAND:
                                        return (uint8_t)TILEID::TREE_SPRUCE;
                                    case BIOME::SANDLAND:
                                        return (uint8_t)TILEID::CACTUS;
                                    }

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
                                        //else
                                        //{
                                            if (ptrCurrChunk->biome == (uint8_t)BIOME::SANDLAND)
                                            {
                                                ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandSandIndex();
                                            }
                                        //}
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
                                if (ptrCurrChunk->biome == (uint8_t)BIOME::SANDLAND)
                                {
                                    ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = getRandSandIndex();
                                }
                                posY += 1;
                                // 1 in 10 chance to spawn a plant. depends on biome.   
                                uint8_t currPlantId = getRandPlantIndex((BIOME)ptrCurrChunk->biome);
                                ptrCurrChunk->tiles[((CHUNK_WIDTH - 1 - posY) * CHUNK_WIDTH) + posX] = currPlantId;
                                if (currPlantId > 0)
                                {
                                    // printf("Plant of tile ID %d spawning at tile = {X: %d , Y: %d } chunk = {X: %d , Y: %d} \n \t Elevation = { %d } \n",
                                    //    (int)currPlantId, posX, CHUNK_WIDTH - 1 - posY, ptrCurrChunk->x, ptrCurrChunk->y, elevation);
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
                                float renderPosX = (float)(int)(ptrCurrChunk->x * CHUNK_WORLD_WIDTH) + (posX * TILE_SIZE);
                                float renderPosY = (float)(int)(ptrCurrChunk->y * CHUNK_WORLD_WIDTH) + (posY * TILE_SIZE);

                                // todo(jaden): fix visual bug involving rendering of distant chunks.


                                TileTexture currTexture = lookupTileTexture[ptrCurrChunk->tiles[posY * CHUNK_WIDTH + posX]];
                                Texture* ptrCurrAtlas = nullptr;
                                if (currTexture.atlas == ATLASID::TERRAIN)
                                {
                                    ptrCurrAtlas = &ATLAS_TERRAIN;
                                }
                                if (currTexture.atlas == ATLASID::PLANTS)
                                {
                                    ptrCurrAtlas = &ATLAS_PLANTS;
                                }
                                // uint8_t textureIndex = currTexture.textureIndex;
                                if (ptrCurrAtlas != nullptr && currTexture.atlas != ATLASID::NIL)
                                {
                                    bool isSpecialTile = (currTexture.atlas == ATLASID::PLANTS);
                                    if (isSpecialTile)
                                    {
                                        // note: update this later when we arent doing just plants.
                                        renderPosY -= TILE_SIZE;
                                    }
                                    DrawTextureAtAtlasIndex(
                                        *ptrCurrAtlas,
                                        { renderPosX,renderPosY },
                                        (int)currTexture.textureIndex.first,
                                        lookupTextureAtlasDimensions[(int)currTexture.atlas],
                                        (isSpecialTile) ? 0 : TILE_SIZE
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
                                                if (currTexture.atlas == ATLASID::PLANTS)
                                                {
                                                    if (currTexture.textureIndex.second == (uint8_t)TEXTUREINDEX_PLANTS::COUNT)
                                                    {
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
                                                { renderPosX,renderPosY },
                                                (int)currTexture.textureIndex.second,
                                                lookupTextureAtlasDimensions[(int)currTexture.atlas],
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
        ui.printui(TextFormat("Y Velocity: %f", yVel));
        ui.printui(TextFormat("Player Touching Ground: %d", (int)isPlayerTouchingGround));

        static Timer timerPlayerDelta{ 1.0f };
        static Timer timerPrintDuration = Timer{ 0.77789f };
        static Vector2 originalPos = { playerPos.x,playerPos.y };
        static std::string outputStr;
        if (timerPlayerDelta.hasEnded())
        {
            int distancePx = (int)playerPos.x - (int)originalPos.x;
            outputStr = TextFormat("Distance walked in one second: %d Pixels.", distancePx);
            originalPos = { playerPos.x,playerPos.y };
            timerPlayerDelta.start();
            timerPrintDuration.start();
        }
        if (playerIsMoving)
        {
            timerPlayerDelta.tick();
        }
        if (!timerPrintDuration.hasEnded())
        {
            timerPrintDuration.tick();
            ui.setColor(GREEN);
            ui.printui(outputStr.c_str());
        }

        static bool F3_TOGGLE = false;
        if (IsKeyPressed(KEY_F3))
        {
            F3_TOGGLE = !F3_TOGGLE;
        }
        /* Debug User Interface. */ if (F3_TOGGLE) {
            // tatic util_printui ui;
            static const int UI_OURFONTSIZE = 24;
            static const char* UI_GAMEVERSION = "pre-alpha (0.0.0)";
            // ui.resetui();
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