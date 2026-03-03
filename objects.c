#include "objects.h"
#include <string.h>
#include <math.h> // Needed for fabsf()

LevelObject levelObjects[MAX_OBJECTS];
int objectCount = 0;
bool isSelected[MAX_OBJECTS] = {0};
Rectangle levelBounds = {0};

LevelObject clipboard[MAX_OBJECTS];
int clipboardCount = 0;

#define MAX_UNDO 15
typedef struct { LevelObject objects[MAX_OBJECTS]; int count; } UndoSnapshot;
UndoSnapshot undoHistory[MAX_UNDO];
int undoIndex = 0; int undoCount = 0; 

void ClearSelection(void) { memset(isSelected, 0, sizeof(isSelected)); }
bool HasSelection(void) { for (int i = 0; i < objectCount; i++) if (isSelected[i]) return true; return false; }
void SaveSnapshot(void) { for (int i = 0; i < objectCount; i++) undoHistory[undoIndex].objects[i] = levelObjects[i]; undoHistory[undoIndex].count = objectCount; undoIndex = (undoIndex + 1) % MAX_UNDO; if (undoCount < MAX_UNDO) undoCount++; }
void PerformUndo(void) { if (undoCount > 0) { undoIndex = (undoIndex - 1 + MAX_UNDO) % MAX_UNDO; objectCount = undoHistory[undoIndex].count; for (int i = 0; i < objectCount; i++) levelObjects[i] = undoHistory[undoIndex].objects[i]; undoCount--; ClearSelection(); } }

void InitObjects(void) {
    levelObjects[0].type = OBJ_WALL;
    levelObjects[0].rect = (Rectangle){ 0, 400, 800, 50 };
    levelObjects[0].rotation = 0;
    levelObjects[0].linkID = 0;
    levelObjects[0].moveType = MOVE_HORZ;
    levelObjects[0].moveRange = 100.0f;
    levelObjects[0].moveSpeed = 1.5f; 
    objectCount = 1; clipboardCount = 0; ClearSelection(); undoCount = 0; undoIndex = 0;
}

void CalculateLevelBounds(void) {
    if (objectCount == 0) { levelBounds = (Rectangle){0, 0, 800, 450}; return; }
    float minX = 999999, minY = 999999, maxX = -999999, maxY = -999999;
    for (int i = 0; i < objectCount; i++) {
        if (levelObjects[i].rect.x < minX) minX = levelObjects[i].rect.x;
        if (levelObjects[i].rect.y < minY) minY = levelObjects[i].rect.y;
        if (levelObjects[i].rect.x + levelObjects[i].rect.width > maxX) maxX = levelObjects[i].rect.x + levelObjects[i].rect.width;
        if (levelObjects[i].rect.y + levelObjects[i].rect.height > maxY) maxY = levelObjects[i].rect.y + levelObjects[i].rect.height;
    }
    levelBounds = (Rectangle){ minX - 400, minY - 300, (maxX - minX) + 800, (maxY - minY) + 600 };
}

void RotateSelectedObjects(bool individual) {
    if (individual) {
        for (int i = 0; i < objectCount; i++) {
            if (isSelected[i]) {
                levelObjects[i].rotation = (levelObjects[i].rotation + 90) % 360;
                float temp = levelObjects[i].rect.width; levelObjects[i].rect.width = levelObjects[i].rect.height; levelObjects[i].rect.height = temp;
            }
        }
    } else {
        float minX = 99999, minY = 99999, maxX = -99999, maxY = -99999;
        for (int i = 0; i < objectCount; i++) {
            if (isSelected[i]) {
                if (levelObjects[i].rect.x < minX) minX = levelObjects[i].rect.x;
                if (levelObjects[i].rect.y < minY) minY = levelObjects[i].rect.y;
                if (levelObjects[i].rect.x + levelObjects[i].rect.width > maxX) maxX = levelObjects[i].rect.x + levelObjects[i].rect.width;
                if (levelObjects[i].rect.y + levelObjects[i].rect.height > maxY) maxY = levelObjects[i].rect.y + levelObjects[i].rect.height;
            }
        }
        float groupH = maxY - minY;
        for (int i = 0; i < objectCount; i++) {
            if (isSelected[i]) {
                float localX = levelObjects[i].rect.x - minX; float localY = levelObjects[i].rect.y - minY;
                float oldW = levelObjects[i].rect.width; float oldH = levelObjects[i].rect.height;
                levelObjects[i].rect.x = minX + (groupH - localY - oldH);
                levelObjects[i].rect.y = minY + localX;
                levelObjects[i].rect.width = oldH; levelObjects[i].rect.height = oldW;
                levelObjects[i].rotation = (levelObjects[i].rotation + 90) % 360;
            }
        }
    }
}

void DeleteSelectedObjects(void) {
    for (int i = objectCount - 1; i >= 0; i--) {
        if (isSelected[i]) { levelObjects[i] = levelObjects[objectCount - 1]; isSelected[i] = isSelected[objectCount - 1]; objectCount--; }
    }
}

void CopySelectedObjects(void) {
    clipboardCount = 0; float minX = 99999, minY = 99999;
    for (int i = 0; i < objectCount; i++) {
        if (isSelected[i]) {
            clipboard[clipboardCount++] = levelObjects[i];
            if (levelObjects[i].rect.x < minX) minX = levelObjects[i].rect.x;
            if (levelObjects[i].rect.y < minY) minY = levelObjects[i].rect.y;
        }
    }
    for (int i = 0; i < clipboardCount; i++) { clipboard[i].rect.x -= minX; clipboard[i].rect.y -= minY; }
}

void PasteClipboardObjects(int gridX, int gridY) {
    if (objectCount + clipboardCount > MAX_OBJECTS) return;
    ClearSelection();
    for (int c = 0; c < clipboardCount; c++) {
        LevelObject newObj = clipboard[c];
        newObj.rect.x += gridX; newObj.rect.y += gridY;
        levelObjects[objectCount] = newObj; isSelected[objectCount] = true; objectCount++;
    }
}

void DrawObjects(bool isEditMode) {
    for (int i = 0; i < objectCount; i++) {
        if (!isEditMode && levelObjects[i].type == OBJ_TRIGGER) continue; 

        Color objColor = DARKGRAY;
        if (levelObjects[i].type == OBJ_HAZARD) objColor = RED;
        if (levelObjects[i].type == OBJ_SPAWN) objColor = GREEN;
        if (levelObjects[i].type == OBJ_END) objColor = YELLOW;
        if (levelObjects[i].type == OBJ_CHECKPOINT) objColor = BLUE;
        if (levelObjects[i].type == OBJ_MOVING_BLOCK) objColor = PURPLE;
        if (levelObjects[i].type == OBJ_TRIGGER) objColor = MAGENTA;
        
        if (levelObjects[i].type == OBJ_TRIGGER) {
            DrawRectangleLinesEx(levelObjects[i].rect, 2.0f, objColor);
        } else {
            DrawRectangleRec(levelObjects[i].rect, objColor);
        }
        
        if (levelObjects[i].type == OBJ_SPAWN) DrawText("S", levelObjects[i].rect.x + 5, levelObjects[i].rect.y + 5, 20, BLACK);
        if (levelObjects[i].type == OBJ_END) DrawText("E", levelObjects[i].rect.x + 5, levelObjects[i].rect.y + 5, 20, BLACK);
        if (levelObjects[i].type == OBJ_CHECKPOINT) DrawText("C", levelObjects[i].rect.x + 5, levelObjects[i].rect.y + 5, 20, RAYWHITE);
        if (levelObjects[i].type == OBJ_TRIGGER) DrawText("T", levelObjects[i].rect.x + 5, levelObjects[i].rect.y + 5, 20, MAGENTA);
        
        if (isEditMode && (levelObjects[i].type == OBJ_MOVING_BLOCK || levelObjects[i].type == OBJ_TRIGGER)) {
            DrawText(TextFormat("%d", levelObjects[i].linkID), levelObjects[i].rect.x + 5, levelObjects[i].rect.y + 5, 10, RAYWHITE);
        }

        // --- PATH VISUALIZATION UPDATE ---
        if (isEditMode && levelObjects[i].type == OBJ_MOVING_BLOCK) {
            Vector2 center = { levelObjects[i].rect.x + levelObjects[i].rect.width/2, levelObjects[i].rect.y + levelObjects[i].rect.height/2 };
            Vector2 end = center;
            
            if (levelObjects[i].moveType == MOVE_HORZ) end.x += levelObjects[i].moveRange;
            if (levelObjects[i].moveType == MOVE_VERT) end.y += levelObjects[i].moveRange;

            if (levelObjects[i].moveType == MOVE_CIRCLE) {
                // The drag handle is the Pivot center!
                float pivotX = center.x + levelObjects[i].moveRange;
                float pivotY = center.y;
                DrawCircleLines(pivotX, pivotY, fabsf(levelObjects[i].moveRange), Fade(PURPLE, 0.4f));
                
                // Draw handle AT the pivot
                if (isSelected[i]) DrawRectangle(pivotX - 6, pivotY - 6, 12, 12, ORANGE);
            } else {
                DrawLineEx(center, end, 2.0f, Fade(PURPLE, 0.4f));
                Rectangle ghost = levelObjects[i].rect;
                ghost.x += (end.x - center.x);
                ghost.y += (end.y - center.y);
                DrawRectangleLinesEx(ghost, 2.0f, Fade(PURPLE, 0.4f));
                
                if (isSelected[i]) DrawRectangle(end.x - 6, end.y - 6, 12, 12, ORANGE);
            }
        }

        if (isEditMode && isSelected[i]) DrawRectangleLinesEx(levelObjects[i].rect, 3.0f, ORANGE);
    }
}