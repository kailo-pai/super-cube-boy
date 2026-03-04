#ifndef OBJECTS_H
#define OBJECTS_H

#include "raylib.h"
#include "types.h"

#define MAX_OBJECTS 25000

extern LevelObject levelObjects[MAX_OBJECTS];
extern int objectCount;

extern bool isSelected[MAX_OBJECTS];
extern Rectangle levelBounds;

extern LevelObject clipboard[MAX_OBJECTS];
extern int clipboardCount;

void ClearSelection(void);
bool HasSelection(void);
void InitObjects(void);
void CalculateLevelBounds(void);

void SaveSnapshot(void);
void PerformUndo(void);

void RotateSelectedObjects(bool individual);
void DeleteSelectedObjects(void);
void CopySelectedObjects(void);
void PasteClipboardObjects(int gridX, int gridY);

void DrawObjects(bool isEditMode);

#endif