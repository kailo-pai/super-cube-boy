#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <stdbool.h>

typedef enum { MAIN_MENU, LEVEL_SELECT, EDIT_MODE, PLAY_MODE, PAUSE_MENU, EDITOR_PAUSE_MENU, WIN_SCREEN } GameState;

// --- NIEUW: OBJ_MOVING_HAZARD toegevoegd ---
typedef enum { OBJ_WALL, OBJ_HAZARD, OBJ_SPAWN, OBJ_END, OBJ_CHECKPOINT, OBJ_MOVING_BLOCK, OBJ_TRIGGER, OBJ_MUD, OBJ_ICE, OBJ_DOOR, OBJ_KEY, OBJ_MOVING_HAZARD } ObjectType;

// --- NIEUW: Tools voor de draaiende zagen toegevoegd ---
typedef enum { TOOL_NONE, TOOL_WALL, TOOL_HALF_WALL, TOOL_HAZARD, TOOL_HALF_HAZARD, TOOL_SPAWN, TOOL_END, TOOL_CHECKPOINT, TOOL_DELETE, TOOL_MOVING_WALL, TOOL_HALF_MOVING_WALL, TOOL_TRIGGER, TOOL_MUD, TOOL_HALF_MUD, TOOL_ICE, TOOL_HALF_ICE, TOOL_DOOR, TOOL_HALF_DOOR, TOOL_KEY, TOOL_MOVING_HAZARD, TOOL_HALF_MOVING_HAZARD } EditorTool;

typedef enum { MOVE_HORZ, MOVE_VERT, MOVE_CIRCLE } MoveType;

typedef struct LevelObject {
    ObjectType type;
    Rectangle rect;
    int rotation; 
    
    int linkID;              
    MoveType moveType;       
    float moveRange;         
    float moveSpeed;         
    
    bool isActive;           
    float currentAngle;      
    float startX;            
    float startY;     
    
    float progress;          
    int moveDir;             
    float waitTimer;         
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

    float levelTimer;
    bool hasMoved;
} Player;

#endif