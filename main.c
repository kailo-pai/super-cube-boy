#include "raylib.h"
#include "types.h"
#include "level.h"
#include "player.h"
#include "ui.h"
#include "objects.h"
#include <string.h> 
#include <stdio.h> 

// --- NIEUWE FUNCTIES VOOR HET OPSLAAN VAN TIJDEN ---
float GetBestTime(const char* levelName) {
    char timeFile[128];
    sprintf(timeFile, "%s.time", levelName);
    float bestTime = 0.0f;
    FILE *f = fopen(timeFile, "r");
    if (f) {
        fscanf(f, "%f", &bestTime);
        fclose(f);
        return bestTime;
    }
    return 0.0f; // 0 betekent dat het level nog nooit gehaald is
}

void SaveBestTime(const char* levelName, float newTime) {
    char timeFile[128];
    sprintf(timeFile, "%s.time", levelName);
    
    float oldTime = GetBestTime(levelName);
    
    // Alleen opslaan als we nog geen tijd hadden (0.0f) óf als de nieuwe tijd sneller is!
    if (oldTime == 0.0f || newTime < oldTime) {
        FILE *f = fopen(timeFile, "w");
        if (f) {
            fprintf(f, "%f", newTime);
            fclose(f);
        }
    }
}
// ---------------------------------------------------

int main(void)
{
    InitWindow(800, 450, "Super Cube Boy - Pro Level Editor");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); 

    GameState currentState = MAIN_MENU; 
    GameState targetState = PLAY_MODE; 

    Player player;
    InitLevel();

    Camera2D editorCamera = { 0 };
    editorCamera.target = (Vector2){ 400, 225 };
    editorCamera.offset = (Vector2){ 400, 225 }; 
    editorCamera.rotation = 0.0f;
    editorCamera.zoom = 1.0f;

    Camera2D playCamera = { 0 };
    playCamera.target = (Vector2){ 400, 225 };
    playCamera.offset = (Vector2){ 400, 225 }; 
    playCamera.rotation = 0.0f;
    playCamera.zoom = 1.0f;

    Rectangle playButton = { 300, 200, 200, 50 };
    Rectangle editButton = { 300, 280, 200, 50 };
    Rectangle level1Button = { 300, 150, 200, 50 };
    Rectangle level2Button = { 300, 230, 200, 50 };
    Rectangle level3Button = { 300, 310, 200, 50 };
    
    Rectangle xBtn = { 760, 5, 30, 30 };

    char currentLevelFile[64] = "level_1.txt"; 
    bool quitGame = false;

    while (!WindowShouldClose() && !quitGame)
    {
        Vector2 screenMousePos = GetMousePosition();

        if (currentState == MAIN_MENU) {
            if (IsKeyPressed(KEY_ESCAPE)) quitGame = true; 
            
            if (CheckCollisionPointRec(screenMousePos, playButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                targetState = PLAY_MODE; currentState = LEVEL_SELECT;
            }
            if (CheckCollisionPointRec(screenMousePos, editButton) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                targetState = EDIT_MODE; currentState = LEVEL_SELECT;
            }
        }
        else if (currentState == LEVEL_SELECT) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MAIN_MENU; 
            
            bool levelPicked = false;
            if (CheckCollisionPointRec(screenMousePos, level1Button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { strcpy(currentLevelFile, "level_1.txt"); levelPicked = true; }
            if (CheckCollisionPointRec(screenMousePos, level2Button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { strcpy(currentLevelFile, "level_2.txt"); levelPicked = true; }
            if (CheckCollisionPointRec(screenMousePos, level3Button) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { strcpy(currentLevelFile, "level_3.txt"); levelPicked = true; }

            if (levelPicked) {
                InitLevel(); 
                if (FileExists(currentLevelFile)) LoadLevel(currentLevelFile); 
                float startX = 50, startY = 50; 
                for (int i = 0; i < objectCount; i++) {
                    if (levelObjects[i].type == OBJ_SPAWN) { startX = levelObjects[i].rect.x; startY = levelObjects[i].rect.y; break; }
                }
                InitPlayer(&player, startX, startY); 
                CalculateLevelBounds();
                
                editorCamera.target = (Vector2){ startX, startY };
                editorCamera.zoom = 1.0f; 
                initialCameraTarget = editorCamera.target;
                initialCameraZoom = editorCamera.zoom;
                
                playCamera.target = (Vector2){ startX, startY };
                
                if (targetState == PLAY_MODE) {
                    for (int i = 0; i < objectCount; i++) {
                        // FIX: OBJ_MOVING_HAZARD (de zagen) onthouden nu ook perfect hun startpositie!
                        if (levelObjects[i].type == OBJ_MOVING_BLOCK || levelObjects[i].type == OBJ_MOVING_HAZARD) {
                            levelObjects[i].startX = levelObjects[i].rect.x;
                            levelObjects[i].startY = levelObjects[i].rect.y;
                            levelObjects[i].currentAngle = 3.14159f; 
                            levelObjects[i].progress = 0.0f;
                            levelObjects[i].moveDir = (levelObjects[i].moveRange < 0) ? -1 : 1; 
                            levelObjects[i].waitTimer = 0.5f; 
                            levelObjects[i].isActive = (levelObjects[i].linkID == 0);
                        }
                        if (levelObjects[i].type == OBJ_TRIGGER) {
                            levelObjects[i].isActive = false;
                        }
                    }
                }
                
                currentState = targetState; 
            }
        }
        else if (currentState == EDIT_MODE) {
            if (CheckCollisionPointRec(screenMousePos, xBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                currentState = EDITOR_PAUSE_MENU;
            } else {
                if (UpdateEditor(&editorCamera)) currentState = EDITOR_PAUSE_MENU;
            }
            if (IsKeyPressed(KEY_S)) SaveLevel(currentLevelFile); 
        }
        else if (currentState == PLAY_MODE) {
            if (IsKeyPressed(KEY_ESCAPE) || (CheckCollisionPointRec(screenMousePos, xBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                currentState = PAUSE_MENU;
            } else {
                UpdatePlayer(&player, levelObjects, objectCount, &playCamera, levelBounds);
                if (player.hasWon) {
                    SaveBestTime(currentLevelFile, player.levelTimer);
                    currentState = WIN_SCREEN;
                }
            }
        }
        else if (currentState == PAUSE_MENU || currentState == EDITOR_PAUSE_MENU) {
            Rectangle continueBtn = { 300, 180, 200, 50 };
            Rectangle backBtn = { 300, 260, 200, 50 };
            
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MAIN_MENU;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointRec(screenMousePos, continueBtn)) {
                    currentState = (currentState == PAUSE_MENU) ? PLAY_MODE : EDIT_MODE;
                }
                if (CheckCollisionPointRec(screenMousePos, backBtn)) currentState = MAIN_MENU;
            }
        }
        else if (currentState == WIN_SCREEN) {
            if (IsKeyPressed(KEY_ESCAPE)) currentState = MAIN_MENU;
            Rectangle backBtn = { 300, 280, 200, 50 }; 
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(screenMousePos, backBtn)) currentState = MAIN_MENU;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (currentState == MAIN_MENU) 
            {
                DrawText("SUPER CUBE BOY", 210, 100, 40, DARKGRAY);
                DrawRectangleRec(playButton, CheckCollisionPointRec(screenMousePos, playButton) ? LIGHTGRAY : DARKGRAY);
                DrawText("PLAY", (int)playButton.x + 70, (int)playButton.y + 15, 20, CheckCollisionPointRec(screenMousePos, playButton) ? DARKGRAY : WHITE);
                DrawRectangleRec(editButton, CheckCollisionPointRec(screenMousePos, editButton) ? LIGHTGRAY : DARKGRAY);
                DrawText("LEVEL EDITOR", (int)editButton.x + 25, (int)editButton.y + 15, 20, CheckCollisionPointRec(screenMousePos, editButton) ? DARKGRAY : WHITE);
                DrawText("Press 'ESC' to Quit", 10, 10, 20, MAROON);
            }
            else if (currentState == LEVEL_SELECT)
            {
                DrawText(targetState == PLAY_MODE ? "SELECT LEVEL TO PLAY" : "SELECT LEVEL TO EDIT", 220, 80, 30, DARKGRAY);
                
                DrawRectangleRec(level1Button, CheckCollisionPointRec(screenMousePos, level1Button) ? LIGHTGRAY : DARKGRAY);
                DrawText(FileExists("level_1.txt") ? "Level 1" : "Level 1 (Empty)", (int)level1Button.x + 20, (int)level1Button.y + 15, 20, WHITE);
                float t1 = GetBestTime("level_1.txt");
                if (t1 > 0.0f && targetState == PLAY_MODE) DrawText(TextFormat("Best: %.2fs", t1), (int)level1Button.x + 210, (int)level1Button.y + 15, 20, DARKGREEN);

                DrawRectangleRec(level2Button, CheckCollisionPointRec(screenMousePos, level2Button) ? LIGHTGRAY : DARKGRAY);
                DrawText(FileExists("level_2.txt") ? "Level 2" : "Level 2 (Empty)", (int)level2Button.x + 20, (int)level2Button.y + 15, 20, WHITE);
                float t2 = GetBestTime("level_2.txt");
                if (t2 > 0.0f && targetState == PLAY_MODE) DrawText(TextFormat("Best: %.2fs", t2), (int)level2Button.x + 210, (int)level2Button.y + 15, 20, DARKGREEN);

                DrawRectangleRec(level3Button, CheckCollisionPointRec(screenMousePos, level3Button) ? LIGHTGRAY : DARKGRAY);
                DrawText(FileExists("level_3.txt") ? "Level 3" : "Level 3 (Empty)", (int)level3Button.x + 20, (int)level3Button.y + 15, 20, WHITE);
                float t3 = GetBestTime("level_3.txt");
                if (t3 > 0.0f && targetState == PLAY_MODE) DrawText(TextFormat("Best: %.2fs", t3), (int)level3Button.x + 210, (int)level3Button.y + 15, 20, DARKGREEN);

                DrawText("Press 'ESC' for Main Menu", 10, 10, 20, MAROON);
            }
            else if (currentState == EDIT_MODE) 
            {
                BeginMode2D(editorCamera);
                    for (int i = -2000; i < 2000; i += 50) {
                        DrawLine(i, -2000, i, 2000, Fade(LIGHTGRAY, 0.3f));
                        DrawLine(-2000, i, 2000, i, Fade(LIGHTGRAY, 0.3f));
                    }
                    DrawLevelWorld(editorCamera, true);
                EndMode2D();
                
                DrawEditorUI(currentLevelFile);
                
                DrawRectangleRec(xBtn, CheckCollisionPointRec(screenMousePos, xBtn) ? RED : MAROON);
                DrawText("X", xBtn.x + 8, xBtn.y + 5, 20, RAYWHITE);
            } 
            else if (currentState == PLAY_MODE) 
            {
                BeginMode2D(playCamera);
                    DrawLevelWorld(playCamera, false);
                    DrawPlayer(&player);
                EndMode2D();
                
                DrawText(TextFormat("Deaths: %d", player.deathCount), 10, 10, 20, RED);
                DrawText(TextFormat("Time: %.2f", player.levelTimer), 10, 35, 20, BLACK);
                
                DrawRectangleRec(xBtn, CheckCollisionPointRec(screenMousePos, xBtn) ? RED : MAROON);
                DrawText("X", xBtn.x + 8, xBtn.y + 5, 20, RAYWHITE);
            }
            else if (currentState == PAUSE_MENU || currentState == EDITOR_PAUSE_MENU) 
            {
                DrawRectangle(0, 0, 800, 450, Fade(BLACK, 0.7f));
                DrawText(currentState == PAUSE_MENU ? "GAME PAUSED" : "EDITOR PAUSED", 260, 100, 40, RAYWHITE);
                
                Rectangle continueBtn = { 300, 180, 200, 50 };
                Rectangle backBtn = { 300, 260, 200, 50 };
                
                DrawRectangleRec(continueBtn, CheckCollisionPointRec(screenMousePos, continueBtn) ? LIGHTGRAY : DARKGRAY);
                DrawText("CONTINUE", 350, 195, 20, CheckCollisionPointRec(screenMousePos, continueBtn) ? DARKGRAY : WHITE);

                DrawRectangleRec(backBtn, CheckCollisionPointRec(screenMousePos, backBtn) ? LIGHTGRAY : DARKGRAY);
                DrawText("MAIN MENU", 345, 275, 20, CheckCollisionPointRec(screenMousePos, backBtn) ? DARKGRAY : WHITE);
                
                DrawText("Press 'ESC' to instantly return to Menu", 210, 350, 20, LIGHTGRAY);
            }
            else if (currentState == WIN_SCREEN) 
            {
                DrawRectangle(0, 0, 800, 450, Fade(RAYWHITE, 0.8f));
                DrawText("LEVEL COMPLETE!", 210, 120, 40, DARKGREEN);
                
                DrawText(TextFormat("Time: %.2f seconds", player.levelTimer), 290, 180, 20, BLACK);
                DrawText(TextFormat("Total Deaths: %d", player.deathCount), 310, 210, 20, MAROON);
                
                Rectangle backBtn = { 300, 280, 200, 50 };
                DrawRectangleRec(backBtn, CheckCollisionPointRec(screenMousePos, backBtn) ? LIGHTGRAY : DARKGRAY);
                DrawText("MAIN MENU", 345, 295, 20, CheckCollisionPointRec(screenMousePos, backBtn) ? DARKGRAY : WHITE);
            }
            
        EndDrawing();
    }
    CloseWindow();
    return 0;
}