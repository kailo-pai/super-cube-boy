#include "ui.h"
#include "objects.h"
#include <math.h>

EditorTool currentTool = TOOL_NONE; 
int currentCategory = 0; 

bool UpdateEditorUI(Vector2 screenMousePos, Camera2D *camera, Vector2 initCamTarget, float initCamZoom) {
    bool isMouseOverUI = (screenMousePos.y >= 350) || (screenMousePos.x <= 80 && screenMousePos.y <= 160);

    int selectedCount = 0;
    int lastSelectedIndex = -1;
    for (int i = 0; i < objectCount; i++) {
        if (isSelected[i]) { selectedCount++; lastSelectedIndex = i; }
    }
    
    if (selectedCount == 1 && screenMousePos.x >= 600 && screenMousePos.y <= 210) {
        isMouseOverUI = true;
    }

    if (isMouseOverUI && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (selectedCount == 1) {
            LevelObject *obj = &levelObjects[lastSelectedIndex];
            Rectangle leftBtn = { 670, 75, 20, 20 };
            Rectangle rightBtn = { 720, 75, 20, 20 };
            Rectangle cycleBtn = { 670, 110, 60, 20 };
            
            // --- NIEUW: Snelheidsknoppen (- en +) ---
            Rectangle speedMinBtn = { 690, 145, 20, 20 };
            Rectangle speedMaxBtn = { 720, 145, 20, 20 };

            if (CheckCollisionPointRec(screenMousePos, leftBtn)) { SaveSnapshot(); obj->linkID--; if (obj->linkID < 0) obj->linkID = 0; }
            if (CheckCollisionPointRec(screenMousePos, rightBtn)) { SaveSnapshot(); obj->linkID++; }
            
            if (obj->type == OBJ_MOVING_BLOCK || obj->type == OBJ_MOVING_HAZARD) {
                if (CheckCollisionPointRec(screenMousePos, cycleBtn)) {
                    SaveSnapshot();
                    obj->moveType = (obj->moveType == MOVE_HORZ) ? MOVE_VERT : (obj->moveType == MOVE_VERT) ? MOVE_CIRCLE : MOVE_HORZ;
                }
                if (CheckCollisionPointRec(screenMousePos, speedMinBtn)) { 
                    SaveSnapshot(); int s = roundf(obj->moveSpeed / 0.3f); s--; if (s < 1) s = 1; obj->moveSpeed = s * 0.3f; 
                }
                if (CheckCollisionPointRec(screenMousePos, speedMaxBtn)) { 
                    SaveSnapshot(); int s = roundf(obj->moveSpeed / 0.3f); s++; if (s > 10) s = 10; obj->moveSpeed = s * 0.3f; 
                }
            }
        }

        if (screenMousePos.x >= 5 && screenMousePos.x <= 75 && screenMousePos.y >= 5 && screenMousePos.y <= 160) {
            if (screenMousePos.y >= 10 && screenMousePos.y <= 50) { currentTool = TOOL_NONE; ClearSelection(); }
            else if (screenMousePos.y >= 60 && screenMousePos.y <= 100) { currentTool = TOOL_DELETE; ClearSelection(); }
            else if (screenMousePos.y >= 110 && screenMousePos.y <= 150) { camera->target = initCamTarget; camera->offset = (Vector2){ 400, 225 }; camera->zoom = initCamZoom; }
        }
        else if (screenMousePos.y >= 350) {
            if (screenMousePos.y >= 410 && screenMousePos.y <= 440) {
                if (screenMousePos.x >= 100 && screenMousePos.x <= 200) currentCategory = 0; 
                else if (screenMousePos.x >= 210 && screenMousePos.x <= 310) currentCategory = 1; 
                else if (screenMousePos.x >= 320 && screenMousePos.x <= 420) currentCategory = 2; 
                else if (screenMousePos.x >= 430 && screenMousePos.x <= 530) currentCategory = 3; 
            }
            else if (screenMousePos.y >= 360 && screenMousePos.y <= 400) {
                if (currentCategory == 0) {
                    if (screenMousePos.x >= 100 && screenMousePos.x <= 140) currentTool = TOOL_WALL;
                    else if (screenMousePos.x >= 150 && screenMousePos.x <= 200) currentTool = TOOL_HALF_WALL;
                    else if (screenMousePos.x >= 210 && screenMousePos.x <= 250) currentTool = TOOL_MUD;
                    else if (screenMousePos.x >= 260 && screenMousePos.x <= 310) currentTool = TOOL_HALF_MUD;
                    else if (screenMousePos.x >= 320 && screenMousePos.x <= 360) currentTool = TOOL_ICE;
                    else if (screenMousePos.x >= 370 && screenMousePos.x <= 420) currentTool = TOOL_HALF_ICE;
                }
                else if (currentCategory == 1) { // --- NIEUW: M-SAW toegevoegd in Hazards ---
                    if (screenMousePos.x >= 100 && screenMousePos.x <= 150) currentTool = TOOL_HAZARD;
                    else if (screenMousePos.x >= 160 && screenMousePos.x <= 230) currentTool = TOOL_HALF_HAZARD;
                    else if (screenMousePos.x >= 240 && screenMousePos.x <= 290) currentTool = TOOL_MOVING_HAZARD;
                    else if (screenMousePos.x >= 300 && screenMousePos.x <= 370) currentTool = TOOL_HALF_MOVING_HAZARD;
                }
                else if (currentCategory == 2) {
                    if (screenMousePos.x >= 100 && screenMousePos.x <= 160) currentTool = TOOL_SPAWN;
                    else if (screenMousePos.x >= 170 && screenMousePos.x <= 230) currentTool = TOOL_END;
                    else if (screenMousePos.x >= 240 && screenMousePos.x <= 320) currentTool = TOOL_CHECKPOINT;
                    else if (screenMousePos.x >= 330 && screenMousePos.x <= 380) currentTool = TOOL_KEY;
                    else if (screenMousePos.x >= 390 && screenMousePos.x <= 440) currentTool = TOOL_DOOR;
                    else if (screenMousePos.x >= 450 && screenMousePos.x <= 510) currentTool = TOOL_HALF_DOOR;
                }
                else if (currentCategory == 3) {
                    if (screenMousePos.x >= 100 && screenMousePos.x <= 160) currentTool = TOOL_MOVING_WALL;
                    else if (screenMousePos.x >= 170 && screenMousePos.x <= 250) currentTool = TOOL_HALF_MOVING_WALL;
                    else if (screenMousePos.x >= 260 && screenMousePos.x <= 340) currentTool = TOOL_TRIGGER;
                }
                if (currentTool != TOOL_NONE && currentTool != TOOL_DELETE) ClearSelection();
            }
        }
    }
    return isMouseOverUI;
}

void DrawEditorUI(const char* currentLevelFile) {
    DrawRectangle(5, 5, 70, 150, Fade(BLACK, 0.7f));
    DrawRectangle(10, 10, 60, 40, (currentTool == TOOL_NONE) ? ORANGE : DARKGRAY);
    DrawText("EDIT", 26, 25, 10, WHITE);
    DrawRectangle(10, 60, 60, 40, (currentTool == TOOL_DELETE) ? RED : DARKGRAY);
    DrawText("DEL", 30, 75, 10, WHITE);
    DrawRectangle(10, 110, 60, 40, DARKGRAY);
    DrawText("RESET", 24, 125, 10, WHITE);
    
    DrawRectangle(0, 350, 800, 100, Fade(BLACK, 0.85f));
    DrawRectangle(100, 410, 100, 30, (currentCategory == 0) ? GRAY : DARKGRAY);
    DrawText("BLOCKS", 125, 420, 10, WHITE);
    DrawRectangle(210, 410, 100, 30, (currentCategory == 1) ? GRAY : DARKGRAY);
    DrawText("HAZARDS", 235, 420, 10, WHITE);
    DrawRectangle(320, 410, 100, 30, (currentCategory == 2) ? GRAY : DARKGRAY);
    DrawText("TECH", 355, 420, 10, WHITE);
    DrawRectangle(430, 410, 100, 30, (currentCategory == 3) ? GRAY : DARKGRAY);
    DrawText("MOVING", 460, 420, 10, WHITE);
    
    if (currentCategory == 0) { 
        DrawRectangle(100, 360, 40, 40, (currentTool == TOOL_WALL) ? ORANGE : DARKGRAY); DrawText("WALL", 105, 375, 10, WHITE);
        DrawRectangle(150, 360, 50, 40, (currentTool == TOOL_HALF_WALL) ? ORANGE : DARKGRAY); DrawText("1/2 W", 160, 375, 10, WHITE);
        DrawRectangle(210, 360, 40, 40, (currentTool == TOOL_MUD) ? ORANGE : DARKGRAY); DrawText("MUD", 215, 375, 10, WHITE);
        DrawRectangle(260, 360, 50, 40, (currentTool == TOOL_HALF_MUD) ? ORANGE : DARKGRAY); DrawText("1/2 M", 270, 375, 10, WHITE);
        DrawRectangle(320, 360, 40, 40, (currentTool == TOOL_ICE) ? ORANGE : DARKGRAY); DrawText("ICE", 325, 375, 10, WHITE);
        DrawRectangle(370, 360, 50, 40, (currentTool == TOOL_HALF_ICE) ? ORANGE : DARKGRAY); DrawText("1/2 I", 380, 375, 10, WHITE);
    }
    else if (currentCategory == 1) { 
        DrawRectangle(100, 360, 50, 40, (currentTool == TOOL_HAZARD) ? ORANGE : DARKGRAY); DrawText("SPIKE", 110, 375, 10, WHITE);
        DrawRectangle(160, 360, 70, 40, (currentTool == TOOL_HALF_HAZARD) ? ORANGE : DARKGRAY); DrawText("1/2 SPIKE", 170, 375, 10, WHITE);
        DrawRectangle(240, 360, 50, 40, (currentTool == TOOL_MOVING_HAZARD) ? ORANGE : DARKGRAY); DrawText("SAW", 255, 375, 10, WHITE);
        DrawRectangle(300, 360, 70, 40, (currentTool == TOOL_HALF_MOVING_HAZARD) ? ORANGE : DARKGRAY); DrawText("1/2 SAW", 310, 375, 10, WHITE);
    }
    else if (currentCategory == 2) { 
        DrawRectangle(100, 360, 60, 40, (currentTool == TOOL_SPAWN) ? ORANGE : DARKGRAY); DrawText("SPAWN", 112, 375, 10, WHITE);
        DrawRectangle(170, 360, 60, 40, (currentTool == TOOL_END) ? ORANGE : DARKGRAY); DrawText("END", 188, 375, 10, WHITE);
        DrawRectangle(240, 360, 80, 40, (currentTool == TOOL_CHECKPOINT) ? ORANGE : DARKGRAY); DrawText("CHECKPT", 255, 375, 10, WHITE);
        DrawRectangle(330, 360, 50, 40, (currentTool == TOOL_KEY) ? ORANGE : DARKGRAY); DrawText("KEY", 345, 375, 10, WHITE);
        DrawRectangle(390, 360, 50, 40, (currentTool == TOOL_DOOR) ? ORANGE : DARKGRAY); DrawText("DOOR", 400, 375, 10, WHITE);
        DrawRectangle(450, 360, 60, 40, (currentTool == TOOL_HALF_DOOR) ? ORANGE : DARKGRAY); DrawText("1/2 DOOR", 455, 375, 10, WHITE);
    }
    else if (currentCategory == 3) { 
        DrawRectangle(100, 360, 60, 40, (currentTool == TOOL_MOVING_WALL) ? ORANGE : DARKGRAY); DrawText("M-WALL", 110, 375, 10, WHITE);
        DrawRectangle(170, 360, 80, 40, (currentTool == TOOL_HALF_MOVING_WALL) ? ORANGE : DARKGRAY); DrawText("1/2 M-WALL", 180, 375, 10, WHITE);
        DrawRectangle(260, 360, 80, 40, (currentTool == TOOL_TRIGGER) ? ORANGE : DARKGRAY); DrawText("TRIGGER", 275, 375, 10, WHITE);
    }
    
    DrawRectangle(80, 5, 670, 30, Fade(BLACK, 0.5f));
    DrawText(TextFormat("Editing: %s | Press 'S' to Save", currentLevelFile), 530, 13, 15, SKYBLUE);

    int selectedCount = 0; int lastSelectedIndex = -1;
    for (int i = 0; i < objectCount; i++) {
        if (isSelected[i]) { selectedCount++; lastSelectedIndex = i; }
    }

    if (selectedCount == 1) {
        LevelObject *obj = &levelObjects[lastSelectedIndex];
        DrawRectangle(600, 40, 190, 170, Fade(BLACK, 0.85f)); // Venster groter gemaakt!
        DrawText("PROPERTIES", 645, 50, 15, ORANGE);
        DrawLine(610, 70, 780, 70, DARKGRAY);

        DrawText(TextFormat("Link ID: %d", obj->linkID), 610, 80, 15, WHITE);
        DrawRectangle(670, 75, 20, 20, GRAY); DrawText("<", 676, 80, 10, BLACK);
        DrawRectangle(720, 75, 20, 20, GRAY); DrawText(">", 726, 80, 10, BLACK);

        if (obj->type == OBJ_MOVING_BLOCK || obj->type == OBJ_MOVING_HAZARD) {
            const char* mType = (obj->moveType == MOVE_HORZ) ? "HORZ" : (obj->moveType == MOVE_VERT) ? "VERT" : "CIRCLE";
            DrawText(TextFormat("Move: %s", mType), 610, 115, 15, WHITE);
            DrawRectangle(680, 110, 60, 20, GRAY); DrawText("CYCLE", 692, 115, 10, BLACK);
            
            // --- NIEUW: Snelheids UI ---
            int speedLvl = (int)roundf(obj->moveSpeed / 0.3f);
            DrawText(TextFormat("Speed: %d", speedLvl), 610, 150, 15, WHITE);
            DrawRectangle(690, 145, 20, 20, GRAY); DrawText("-", 696, 150, 10, BLACK);
            DrawRectangle(720, 145, 20, 20, GRAY); DrawText("+", 726, 150, 10, BLACK);
            
            DrawText(TextFormat("Range: %.0f", obj->moveRange), 610, 185, 15, WHITE);
            DrawText("(Drag Handle in world)", 610, 200, 10, GRAY);
        }
    } 
    else if (selectedCount > 1) {
        DrawRectangle(600, 40, 190, 50, Fade(BLACK, 0.85f)); DrawText("Multiple Selected", 630, 55, 15, GRAY);
    }
}