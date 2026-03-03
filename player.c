#include "player.h"
#include <math.h>

const float gravity = 0.6f;
const float jumpForce = -11.0f;
const float acceleration = 1.0f;
const float maxSpeed = 6.0f;
const float friction = 0.8f;
const float wallSlideSpeed = 2.0f;

void InitPlayer(Player *player, float startX, float startY) {
    player->rect = (Rectangle){ startX, startY, 30, 30 };
    player->spawnX = startX;
    player->spawnY = startY;
    player->velocityX = 0;
    player->velocityY = 0;
    player->canJump = false;
    player->isOnWall = false;
    player->wallDir = 0;
    player->deathCount = 0;
    player->hasWon = false;
}

void UpdatePlayer(Player *player, LevelObject *objects, int objectCount, Camera2D *camera, Rectangle bounds) {
    
    // ==========================================
    // --- 1. PROCESS OVERLAPS (Triggers & Checkpoints) ---
    // ==========================================
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_TRIGGER) {
            Rectangle tRect = objects[i].rect;
            tRect.x += (tRect.width / 2.0f) - 5.0f;
            tRect.y += (tRect.height / 2.0f) - 5.0f;
            tRect.width = 10.0f;
            tRect.height = 10.0f;

            bool isTouching = CheckCollisionRecs(player->rect, tRect);
            
            if (isTouching && !objects[i].isActive) {
                objects[i].isActive = true;
                for (int j = 0; j < objectCount; j++) {
                    if (objects[j].type == OBJ_MOVING_BLOCK && objects[j].linkID == objects[i].linkID) {
                        objects[j].isActive = true;
                    }
                }
            } else if (!isTouching && objects[i].isActive) {
                objects[i].isActive = false;
            }
        }
        // FIXED CHECKPOINT LOGIC: Processed safely without collision walls!
        else if (objects[i].type == OBJ_CHECKPOINT) {
            if (CheckCollisionRecs(player->rect, objects[i].rect)) {
                player->spawnX = objects[i].rect.x;
                player->spawnY = objects[i].rect.y;
            }
        }
    }

    // ==========================================
    // --- 2. MOVE PLATFORMS ---
    // ==========================================
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_MOVING_BLOCK) {
            float oldX = objects[i].rect.x;
            float oldY = objects[i].rect.y;

            if (objects[i].moveType == MOVE_CIRCLE) {
                if (objects[i].linkID == 0 || objects[i].isActive) {
                    objects[i].currentAngle += objects[i].moveSpeed * 0.02f;
                    
                    // The magic pivot math! Starts exactly where placed.
                    float pivotX = objects[i].startX + objects[i].moveRange;
                    float pivotY = objects[i].startY;
                    objects[i].rect.x = pivotX + cosf(objects[i].currentAngle) * objects[i].moveRange;
                    objects[i].rect.y = pivotY + sinf(objects[i].currentAngle) * objects[i].moveRange;
                }
            } else {
                // Linear Physics State Machine (Supports Negative Ranges!)
                if (objects[i].linkID == 0) { 
                    if (objects[i].waitTimer > 0.0f) {
                        objects[i].waitTimer -= (1.0f / 60.0f);
                    } else {
                        objects[i].progress += objects[i].moveSpeed * objects[i].moveDir;
                        
                        if (objects[i].moveRange > 0) {
                            if (objects[i].progress >= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = -1; objects[i].waitTimer = 0.5f; }
                            else if (objects[i].progress <= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = 1; objects[i].waitTimer = 0.5f; }
                        } else {
                            // Negative drag support!
                            if (objects[i].progress <= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = 1; objects[i].waitTimer = 0.5f; }
                            else if (objects[i].progress >= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = -1; objects[i].waitTimer = 0.5f; }
                        }
                    }
                } else { 
                    if (objects[i].isActive) {
                        objects[i].progress += objects[i].moveSpeed * objects[i].moveDir;
                        
                        if (objects[i].moveRange > 0) {
                            if (objects[i].moveDir == 1 && objects[i].progress >= objects[i].moveRange) {
                                objects[i].progress = objects[i].moveRange; objects[i].moveDir = -1; objects[i].isActive = false;
                            } else if (objects[i].moveDir == -1 && objects[i].progress <= 0.0f) {
                                objects[i].progress = 0.0f; objects[i].moveDir = 1; objects[i].isActive = false;
                            }
                        } else {
                            if (objects[i].moveDir == -1 && objects[i].progress <= objects[i].moveRange) {
                                objects[i].progress = objects[i].moveRange; objects[i].moveDir = 1; objects[i].isActive = false;
                            } else if (objects[i].moveDir == 1 && objects[i].progress >= 0.0f) {
                                objects[i].progress = 0.0f; objects[i].moveDir = -1; objects[i].isActive = false;
                            }
                        }
                    }
                }

                if (objects[i].moveType == MOVE_HORZ) objects[i].rect.x = objects[i].startX + objects[i].progress;
                else objects[i].rect.y = objects[i].startY + objects[i].progress;
            }

            Rectangle playerFeet = { player->rect.x, player->rect.y + player->rect.height, player->rect.width, 2 };
            if (CheckCollisionRecs(playerFeet, objects[i].rect) && player->velocityY >= 0) {
                player->rect.x += (objects[i].rect.x - oldX);
                player->rect.y += (objects[i].rect.y - oldY);
            }
        }
    }

    // ==========================================
    // --- 3. STANDARD PLAYER LOGIC ---
    // ==========================================
    if (IsKeyDown(KEY_RIGHT)) player->velocityX += acceleration;
    else if (IsKeyDown(KEY_LEFT)) player->velocityX -= acceleration;
    else player->velocityX *= friction;

    if (player->velocityX > maxSpeed) player->velocityX = maxSpeed;
    if (player->velocityX < -maxSpeed) player->velocityX = -maxSpeed;

    if (IsKeyPressed(KEY_SPACE)) {
        if (player->canJump) { 
            player->velocityY = jumpForce;
            player->canJump = false;
        } else if (player->isOnWall) { 
            player->velocityY = jumpForce;
            player->velocityX = -player->wallDir * maxSpeed; 
            player->isOnWall = false;
        }
    }
    if (IsKeyReleased(KEY_SPACE) && player->velocityY < 0) {
        player->velocityY *= 0.5f; 
    }

    player->velocityY += gravity;
    if (player->isOnWall && player->velocityY > 0 && !player->canJump) {
        if (player->velocityY > wallSlideSpeed) player->velocityY = wallSlideSpeed; 
    }

    player->rect.x += player->velocityX;
    player->isOnWall = false; 
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_SPAWN || objects[i].type == OBJ_CHECKPOINT || objects[i].type == OBJ_TRIGGER) continue; 
        if (CheckCollisionRecs(player->rect, objects[i].rect)) {
            
            if (objects[i].type == OBJ_END) { player->hasWon = true; continue; }

            if (objects[i].type == OBJ_HAZARD) {
                player->rect.x = player->spawnX; player->rect.y = player->spawnY;
                player->velocityX = 0; player->velocityY = 0;
                player->deathCount++; 
                break; 
            }

            if (player->velocityX > 0) { 
                player->rect.x = objects[i].rect.x - player->rect.width;
                player->velocityX = 0;
                player->isOnWall = true;
                player->wallDir = 1; 
            } else if (player->velocityX < 0) { 
                player->rect.x = objects[i].rect.x + objects[i].rect.width;
                player->velocityX = 0;
                player->isOnWall = true;
                player->wallDir = -1; 
            }
        }
    }

    player->rect.y += player->velocityY;
    player->canJump = false; 
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_SPAWN || objects[i].type == OBJ_CHECKPOINT || objects[i].type == OBJ_TRIGGER) continue; 
        if (CheckCollisionRecs(player->rect, objects[i].rect)) {
            
            if (objects[i].type == OBJ_END) { player->hasWon = true; continue; }

            if (objects[i].type == OBJ_HAZARD) {
                player->rect.x = player->spawnX; player->rect.y = player->spawnY;
                player->velocityX = 0; player->velocityY = 0;
                player->deathCount++; 
                break; 
            }

            if (player->velocityY > 0) { 
                player->rect.y = objects[i].rect.y - player->rect.height;
                player->velocityY = 0;
                player->canJump = true;
                player->isOnWall = false; 
            } else if (player->velocityY < 0) { 
                player->rect.y = objects[i].rect.y + objects[i].rect.height;
                player->velocityY = 0;
            }
        }
    }

    float deadzoneX = 100.0f;
    float deadzoneY = 80.0f; 

    if (player->rect.x > camera->target.x + deadzoneX) camera->target.x = player->rect.x - deadzoneX;
    if (player->rect.x < camera->target.x - deadzoneX) camera->target.x = player->rect.x + deadzoneX;
    if (player->rect.y > camera->target.y + deadzoneY) camera->target.y = player->rect.y - deadzoneY;
    if (player->rect.y < camera->target.y - deadzoneY) camera->target.y = player->rect.y + deadzoneY;

    float halfScreenW = 400.0f;
    float halfScreenH = 225.0f;

    if (camera->target.x < bounds.x + halfScreenW) camera->target.x = bounds.x + halfScreenW;
    if (camera->target.x > bounds.x + bounds.width - halfScreenW) camera->target.x = bounds.x + bounds.width - halfScreenW;
    if (camera->target.y < bounds.y + halfScreenH) camera->target.y = bounds.y + halfScreenH;
    if (camera->target.y > bounds.y + bounds.height - halfScreenH) camera->target.y = bounds.y + bounds.height - halfScreenH;
}

void DrawPlayer(Player *player) {
    if (player->isOnWall) DrawRectangleRec(player->rect, RED);
    else DrawRectangleRec(player->rect, MAROON);
}