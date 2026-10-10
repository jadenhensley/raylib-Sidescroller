// C standard library
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
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

void DrawTextureAtAtlasIndex(Texture& txt, Vector2 pos, const int& idx, const Dimensions& dim)
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
    dest.width = sprWid;
    dest.height = sprHgt;

    DrawTexturePro(txt, source, dest, { 0.0f,0.0f }, 0.0f, WHITE);
}

int main(int argc, char* argv[])
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Macaroni");
    SetTargetFPS(60);
    srand(time(NULL));

    Vector2 cameraPos{};
    Camera2D camera = { 0 };
    camera.target = cameraPos;
    camera.offset.x = SCREEN_WIDTH / 2.0f; 
    camera.offset.y = SCREEN_HEIGHT / 2.0f; // Center on screen
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;

    ChunkManager chunkManager = {};

    Texture ATLAS_CHARSKIN = LoadTexture("./assets/atlas_characterSkin_4x.png");
    Texture ATLAS_TERRAIN = LoadTexture("./assets/atlas_grassAndDirtSheet_2x2.png");

    static const Dimensions lookupTextureDimensions[(size_t)TEXTUREID::COUNT] = {
        { 4,1 }, // CHARSKIN
        { 2,2 }, // TERRAIN
    };

    static enum class TEXTUREINDEX_TERRAIN : uint8_t {
        GRASS_VAR1 = 0,
        GRASS_VAR2 = 1,
        DIRT_VAR1 = 2,
        DIRT_VAR2 = 3
    };

    static bool isRunning = true;
    static float elapsed = 0.0f;
    while (!WindowShouldClose() && isRunning)
    {
        float dt = GetFrameTime();
        elapsed += dt;
        
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


        // =========================================

        // todo(jaden): make all of this a hashmap and chunking terrain system.

        static std::vector<std::pair<Vector2, std::pair<int, int>>> sprArr;
        // [0] = X,Y
        // [1] = [[0]=sprite index,[1]=elevation]
        static std::vector<std::pair<int, std::pair<int,int> >> grassArr;
        // [0] = X pos
        // [1] = sprite index
        static bool hasInitYet = false;

        auto getIdx = [](const int& r) -> int
            {
                switch (r)
                {
                case 0:
                    return (int)TEXTUREINDEX_TERRAIN::DIRT_VAR2;
                case 1:
                    return (int)TEXTUREINDEX_TERRAIN::DIRT_VAR1;
                case 2:
                    return (int)TEXTUREINDEX_TERRAIN::GRASS_VAR2;
                case 3:
                    return (int)TEXTUREINDEX_TERRAIN::GRASS_VAR1;
                }
            };

        if (!hasInitYet)
        {
            static const siv::PerlinNoise::seed_type seed = 123456u;
            static const siv::PerlinNoise perlin{ seed };

            for (int posX = 0; posX < 32; ++posX) {
                double noise = perlin.noise1D(posX / 8.0);

                // Height ranges from 4 to 20
                int elevation = 12 + (int)(noise * 8);


                for (int posY = 0; posY < elevation /*32*/; ++posY) {

                    int r = rand() % 2;

                    int invPosY = posY * -1;

                    Vector2 drawPos = { (float)posX, (float)invPosY };
                    sprArr.push_back({ drawPos, {getIdx(r), elevation } });

                    if (posY == elevation - 1)
                    {
                        grassArr.push_back({ posX, { getIdx(r) + 2, elevation } });
                        // we add 2 to get grass.
                    }
                }
            }
            hasInitYet = true;
        }

        
        static std::unordered_map<int, int> mapKeyXposValueGrassDepth;
        static bool initMapYet = false;
        if (!initMapYet)
        {
            for (int posX = 0; posX < 32; ++posX)
            {
                mapKeyXposValueGrassDepth[posX] = (-12 + rand() % 4);
            }
            initMapYet = true;
        }

        if (IsKeyPressed(KEY_R))
        {
            for (int posX = 0; posX < 32; ++posX)
            {
                mapKeyXposValueGrassDepth[posX] = (-12 + rand() % 4);
            }
        }

        for (auto& spr : sprArr)
        {
            int sprWid = ATLAS_TERRAIN.width / lookupTextureDimensions[(size_t)TEXTUREID::TERRAIN].width;
            int sprHgt = ATLAS_TERRAIN.height / lookupTextureDimensions[(size_t)TEXTUREID::TERRAIN].height;
            
            
            DrawTextureAtAtlasIndex(ATLAS_TERRAIN,
                Vector2Multiply(spr.first, {(float)sprWid, (float)sprHgt}),
                spr.second.first,
                lookupTextureDimensions[(size_t)TEXTUREID::TERRAIN]
            );

            if ((int)spr.first.y < mapKeyXposValueGrassDepth.at((int)spr.first.x))
            {
                for (auto& grs : grassArr)
                {
                    if (grs.first == spr.first.x)
                    {
                        DrawTextureAtAtlasIndex(ATLAS_TERRAIN,
                            Vector2Multiply(spr.first, { (float)sprWid, (float)sprHgt }),
                            grs.second.first,
                            lookupTextureDimensions[(size_t)TEXTUREID::TERRAIN]
                        );
                    }
                }
            }

        }

        EndMode2D();

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