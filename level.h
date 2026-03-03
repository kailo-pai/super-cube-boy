#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include "types.h"
#include "objects.h"
#include "ui.h"

extern int placementRotation;
extern Vector2 initialCameraTarget;
extern float initialCameraZoom;

void InitLevel(void);
// --- CHANGED: Returns true if user wants to open Pause Menu ---
bool UpdateEditor(Camera2D *camera); 
void DrawLevelWorld(Camera2D camera, bool isEditMode); 

void SaveLevel(const char *filename);
void LoadLevel(const char *filename);

#endif