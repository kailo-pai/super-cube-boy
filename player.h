#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "level.h" // Needed for LevelObject

void InitPlayer(Player *player, float startX, float startY);

// --- NEW: Passing Camera and Bounds ---
void UpdatePlayer(Player *player, LevelObject *objects, int objectCount, Camera2D *camera, Rectangle bounds);
void DrawPlayer(Player *player);

#endif