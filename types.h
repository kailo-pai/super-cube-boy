#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <stdbool.h>

typedef enum { MAIN_MENU, LEVEL_SELECT, EDIT_MODE, PLAY_MODE, PAUSE_MENU, EDITOR_PAUSE_MENU, WIN_SCREEN } GameState;

typedef enum { OBJ_WALL, OBJ_HAZARD, OBJ_SPAWN, OBJ_END, OBJ_CHECKPOINT, OBJ_MOVING_BLOCK, OBJ_TRIGGER } ObjectType;
typedef enum { TOOL_NONE, TOOL_WALL, TOOL_HALF_WALL, TOOL_HAZARD, TOOL_HALF_HAZARD, TOOL_SPAWN, TOOL_END, TOOL_CHECKPOINT, TOOL_DELETE, TOOL_MOVING_WALL, TOOL_HALF_MOVING_WALL, TOOL_TRIGGER } EditorTool;

typedef enum { MOVE_HORZ, MOVE_VERT, MOVE_CIRCLE } MoveType;

typedef struct LevelObject {
    ObjectType type;
    Rectangle rect;
    int rotation; 
    
    // Properties
    int linkID;              
    MoveType moveType;       
    float moveRange;         
    float moveSpeed;         
    
    // Runtime State (For PLAY_MODE)
    bool isActive;           
    float currentAngle;      
    float startX;            
    float startY;     
    
    // NEW: Physics State Machine Variables
    float progress;          // Tracks distance traveled
    int moveDir;             // 1 for forward, -1 for backward
    float waitTimer;         // Used to pause at the ends
} LevelObject;

typedef struct Player {
    Rectangle rect;
    float velocityY;
    float velocityX;
    bool canJump;
    bool isOnWall;
    int wallDir;
    
    float spawnX; 
    float spawnY;
    int deathCount;
    bool hasWon;
} Player;

#endif