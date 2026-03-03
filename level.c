#include "level.h"
#include <stdio.h>
#include <math.h>

int placementRotation = 0;
Vector2 initialCameraTarget = {0, 0};
float initialCameraZoom = 1.0f;

bool isDragging = false;
bool isDraggingHandle = false; 
float dragOffsetX[MAX_OBJECTS] = {0};
float dragOffsetY[MAX_OBJECTS] = {0};

bool isBoxSelecting = false;
Vector2 selectionStartPos = {0};
Rectangle selectionBox = {0};

void InitLevel(void) {
    InitObjects();
    currentTool = TOOL_NONE;
    currentCategory = 0;
    isDragging = false;
    isBoxSelecting = false;
    isDraggingHandle = false;
    placementRotation = 0;
}

bool UpdateEditor(Camera2D *camera) {
    Vector2 screenMousePos = GetMousePosition();
    Vector2 worldMousePos = GetScreenToWorld2D(screenMousePos, *camera);
    
    bool isCmdCtrlPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
    bool isShiftPressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    float zoomDelta = 0.0f;
    if (IsKeyPressed(KEY_EQUAL)) zoomDelta = 0.15f;
    if (IsKeyPressed(KEY_MINUS)) zoomDelta = -0.15f;
    float wheel = GetMouseWheelMove();
    if (wheel != 0) zoomDelta = wheel * 0.02f; 

    if (zoomDelta != 0.0f) {
        camera->offset = screenMousePos; camera->target = worldMousePos; camera->zoom += zoomDelta;
        if (camera->zoom < 0.25f) camera->zoom = 0.25f;
        if (camera->zoom > 3.0f) camera->zoom = 3.0f;
    }
    worldMousePos = GetScreenToWorld2D(screenMousePos, *camera);

    if (UpdateEditorUI(screenMousePos, camera, initialCameraTarget, initialCameraZoom)) return false;

    if (isCmdCtrlPressed && IsKeyPressed(KEY_Z)) { PerformUndo(); isDragging = false; isBoxSelecting = false; return false; }
    if (IsKeyPressed(KEY_ESCAPE)) { 
        if (currentTool != TOOL_NONE || isDragging || isBoxSelecting || HasSelection()) {
            currentTool = TOOL_NONE; isDragging = false; isBoxSelecting = false; ClearSelection(); 
        } else return true; 
    }
    if (IsKeyPressed(KEY_E)) { currentTool = TOOL_NONE; isDragging = false; isBoxSelecting = false; ClearSelection(); }
    if (IsKeyPressed(KEY_D)) { currentTool = TOOL_DELETE; isDragging = false; isBoxSelecting = false; ClearSelection(); }

    int gridX = (int)(floor(worldMousePos.x / 25.0f)) * 25;
    int gridY = (int)(floor(worldMousePos.y / 25.0f)) * 25;

    int selectedCount = 0; int singleSelectedIdx = -1;
    for (int i = 0; i < objectCount; i++) { if (isSelected[i]) { selectedCount++; singleSelectedIdx = i; } }

    // --- DRAG HANDLE: GRID SNAPPED & NEGATIVE SUPPORT ---
    if (selectedCount == 1 && levelObjects[singleSelectedIdx].type == OBJ_MOVING_BLOCK && currentTool == TOOL_NONE) {
        LevelObject *obj = &levelObjects[singleSelectedIdx];
        Vector2 handlePos = { obj->rect.x + obj->rect.width/2, obj->rect.y + obj->rect.height/2 };
        if (obj->moveType == MOVE_HORZ || obj->moveType == MOVE_CIRCLE) handlePos.x += obj->moveRange;
        else if (obj->moveType == MOVE_VERT) handlePos.y += obj->moveRange;

        Rectangle handleRect = { handlePos.x - 8, handlePos.y - 8, 16, 16 };

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMousePos, handleRect)) {
            isDraggingHandle = true; isDragging = false; SaveSnapshot();
        }

        if (isDraggingHandle) {
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) isDraggingHandle = false;
            else {
                float dist = 0;
                if (obj->moveType == MOVE_HORZ || obj->moveType == MOVE_CIRCLE) dist = worldMousePos.x - (obj->rect.x + obj->rect.width/2);
                else if (obj->moveType == MOVE_VERT) dist = worldMousePos.y - (obj->rect.y + obj->rect.height/2);
                
                // Snap to 25px grid
                dist = roundf(dist / 25.0f) * 25.0f;
                
                // Prevent exactly zero range so it actually moves
                if (dist == 0.0f) dist = 25.0f; 

                obj->moveRange = dist;
            }
            return false; 
        }
    }

    bool hoverExisting = false; int hoveredIndex = -1;
    for (int i = objectCount - 1; i >= 0; i--) {
        if (CheckCollisionPointRec(worldMousePos, levelObjects[i].rect)) { hoverExisting = true; hoveredIndex = i; break; }
    }

    if (currentTool == TOOL_NONE && isCmdCtrlPressed && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isBoxSelecting = true; selectionStartPos = worldMousePos; ClearSelection();
    }
    if (isBoxSelecting) {
        selectionBox.x = fminf(selectionStartPos.x, worldMousePos.x); selectionBox.y = fminf(selectionStartPos.y, worldMousePos.y);
        selectionBox.width = fabsf(worldMousePos.x - selectionStartPos.x); selectionBox.height = fabsf(worldMousePos.y - selectionStartPos.y);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            isBoxSelecting = false;
            for (int i = 0; i < objectCount; i++) if (CheckCollisionRecs(levelObjects[i].rect, selectionBox)) isSelected[i] = true;
        }
        return false;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isCmdCtrlPressed && !isDraggingHandle) {
        if (hoverExisting) {
            if (isShiftPressed) isSelected[hoveredIndex] = !isSelected[hoveredIndex]; 
            else if (!isSelected[hoveredIndex]) { ClearSelection(); isSelected[hoveredIndex] = true; }
            
            if (isSelected[hoveredIndex]) {
                isDragging = true;
                for (int i = 0; i < objectCount; i++) {
                    if (isSelected[i]) { dragOffsetX[i] = levelObjects[i].rect.x - gridX; dragOffsetY[i] = levelObjects[i].rect.y - gridY; }
                }
            }
        } else if (currentTool == TOOL_NONE) ClearSelection();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && currentTool == TOOL_NONE && !hoverExisting && !isDragging && !isCmdCtrlPressed && !isDraggingHandle) {
        Vector2 delta = GetMouseDelta(); camera->target.x -= delta.x / camera->zoom; camera->target.y -= delta.y / camera->zoom;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) isDragging = false; 

    if (isDragging) {
        for (int i = 0; i < objectCount; i++) {
            if (isSelected[i]) { levelObjects[i].rect.x = gridX + dragOffsetX[i]; levelObjects[i].rect.y = gridY + dragOffsetY[i]; }
        }
    }

    if (IsKeyPressed(KEY_R)) {
        if (HasSelection()) { SaveSnapshot(); RotateSelectedObjects(isCmdCtrlPressed); }
        else placementRotation = (placementRotation + 90) % 360;
    }

    if (currentTool == TOOL_DELETE && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && hoverExisting) {
        SaveSnapshot(); ClearSelection(); isSelected[hoveredIndex] = true; DeleteSelectedObjects();
    } 
    else if (IsKeyPressed(KEY_BACKSPACE)) {
        SaveSnapshot(); DeleteSelectedObjects(); isDragging = false;
    }

    if (currentTool != TOOL_NONE && currentTool != TOOL_DELETE && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !isDragging && !isDraggingHandle && objectCount < MAX_OBJECTS) {
        float bH = (currentTool == TOOL_HALF_WALL || currentTool == TOOL_HALF_HAZARD || currentTool == TOOL_HALF_MOVING_WALL) ? 25.0f : 50.0f;
        float fW = (placementRotation == 90 || placementRotation == 270) ? bH : 50.0f;
        float fH = (placementRotation == 90 || placementRotation == 270) ? 50.0f : bH;

        Rectangle proposed = { (float)gridX, (float)gridY, fW, fH };
        bool alreadyExists = false;
        for (int i = 0; i < objectCount; i++) if (CheckCollisionRecs(proposed, levelObjects[i].rect)) { alreadyExists = true; break; }

        if (!alreadyExists) {
            SaveSnapshot(); 
            if (currentTool == TOOL_SPAWN) { for (int i=0; i<objectCount; i++) if (levelObjects[i].type == OBJ_SPAWN) { levelObjects[i] = levelObjects[objectCount-1]; objectCount--; i--; } }
            if (currentTool == TOOL_END) { for (int i=0; i<objectCount; i++) if (levelObjects[i].type == OBJ_END) { levelObjects[i] = levelObjects[objectCount-1]; objectCount--; i--; } }
            
            levelObjects[objectCount].type = (currentTool == TOOL_HAZARD || currentTool == TOOL_HALF_HAZARD) ? OBJ_HAZARD : 
                                             (currentTool == TOOL_SPAWN) ? OBJ_SPAWN : 
                                             (currentTool == TOOL_END) ? OBJ_END : 
                                             (currentTool == TOOL_CHECKPOINT) ? OBJ_CHECKPOINT : 
                                             (currentTool == TOOL_MOVING_WALL || currentTool == TOOL_HALF_MOVING_WALL) ? OBJ_MOVING_BLOCK : 
                                             (currentTool == TOOL_TRIGGER) ? OBJ_TRIGGER : OBJ_WALL;
            
            levelObjects[objectCount].rect = proposed;
            levelObjects[objectCount].rotation = placementRotation;
            
            levelObjects[objectCount].linkID = (levelObjects[objectCount].type == OBJ_TRIGGER) ? 1 : 0; 
            levelObjects[objectCount].moveType = MOVE_HORZ;
            levelObjects[objectCount].moveRange = 100.0f;
            levelObjects[objectCount].moveSpeed = 1.5f; 

            isSelected[objectCount] = false;
            objectCount++;
        }
    }

    if (IsKeyPressed(KEY_C) && HasSelection()) CopySelectedObjects();
    if (clipboardCount > 0 && IsKeyPressed(KEY_V) && currentTool == TOOL_NONE) {
        SaveSnapshot(); PasteClipboardObjects(gridX, gridY);
    }
    
    return false; 
}

void DrawLevelWorld(Camera2D camera, bool isEditMode) {
    DrawObjects(isEditMode);
    
    if (isEditMode && isBoxSelecting) {
        DrawRectangleRec(selectionBox, Fade(BLUE, 0.2f));
        DrawRectangleLinesEx(selectionBox, 2.0f, BLUE);
    }

    if (isEditMode && currentTool >= TOOL_WALL && currentTool <= TOOL_TRIGGER) {
        Vector2 m = GetScreenToWorld2D(GetMousePosition(), camera);
        int gx = (int)(floor(m.x / 25.0f)) * 25;
        int gy = (int)(floor(m.y / 25.0f)) * 25;
        float bH = (currentTool == TOOL_HALF_WALL || currentTool == TOOL_HALF_HAZARD || currentTool == TOOL_HALF_MOVING_WALL) ? 25.0f : 50.0f;
        float fw = (placementRotation == 90 || placementRotation == 270) ? bH : 50.0f;
        float fh = (placementRotation == 90 || placementRotation == 270) ? 50.0f : bH;
        
        Color gC = (currentTool == TOOL_HAZARD || currentTool == TOOL_HALF_HAZARD) ? RED : 
                   (currentTool == TOOL_SPAWN) ? GREEN : 
                   (currentTool == TOOL_END) ? YELLOW : 
                   (currentTool == TOOL_CHECKPOINT) ? BLUE : 
                   (currentTool == TOOL_MOVING_WALL || currentTool == TOOL_HALF_MOVING_WALL) ? PURPLE : 
                   (currentTool == TOOL_TRIGGER) ? MAGENTA : DARKGRAY;
                   
        DrawRectangle(gx, gy, fw, fh, Fade(gC, 0.5f)); 
    }
}

void SaveLevel(const char *filename) {
    FILE *file = fopen(filename, "w"); 
    if (!file) return; 
    fprintf(file, "%d\n", objectCount); 
    for (int i=0; i<objectCount; i++) {
        fprintf(file, "%d %f %f %f %f %d %d %d %f %f\n", 
            levelObjects[i].type, levelObjects[i].rect.x, levelObjects[i].rect.y, 
            levelObjects[i].rect.width, levelObjects[i].rect.height, levelObjects[i].rotation,
            levelObjects[i].linkID, levelObjects[i].moveType, levelObjects[i].moveRange, 
            levelObjects[i].moveSpeed);
    }
    fclose(file); 
}

void LoadLevel(const char *filename) {
    FILE *file = fopen(filename, "r"); 
    if (!file) return; 
    fscanf(file, "%d", &objectCount);
    for (int i=0; i<objectCount; i++) {
        int t, mType; 
        fscanf(file, "%d %f %f %f %f %d %d %d %f %f", 
            &t, &levelObjects[i].rect.x, &levelObjects[i].rect.y, 
            &levelObjects[i].rect.width, &levelObjects[i].rect.height, &levelObjects[i].rotation,
            &levelObjects[i].linkID, &mType, &levelObjects[i].moveRange, 
            &levelObjects[i].moveSpeed);
            
        levelObjects[i].type = (ObjectType)t;
        levelObjects[i].moveType = (MoveType)mType;
    }
    fclose(file);
}