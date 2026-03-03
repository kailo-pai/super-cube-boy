#ifndef UI_H
#define UI_H

#include "raylib.h"
#include "types.h"

extern EditorTool currentTool;
extern int currentCategory;

// Returns true if the mouse is hovering over the UI (the "deadzone")
bool UpdateEditorUI(Vector2 screenMousePos, Camera2D *camera, Vector2 initCamTarget, float initCamZoom);

// Draws the left panel and bottom tray
void DrawEditorUI(const char* currentLevelFile);

#endif