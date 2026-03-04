#include "player.h"
#include <math.h>

const float gravity = 0.6f;
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
    
    // --- NIEUW: Timer Reset ---
    player->levelTimer = 0.0f;
    player->hasMoved = false;
}

void UpdatePlayer(Player *player, LevelObject *objects, int objectCount, Camera2D *camera, Rectangle bounds) {
    
    // --- TIMER LOGICA ---
    if (!player->hasMoved && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyPressed(KEY_SPACE))) {
        player->hasMoved = true;
    }
    if (player->hasMoved && !player->hasWon) {
        player->levelTimer += GetFrameTime();
    }

    // --- RESET DEUREN & KEYS ALS WE STARTEN ---
    if (player->levelTimer == 0.0f && !player->hasMoved) {
        for(int i=0; i<objectCount; i++) {
            if (objects[i].type == OBJ_KEY || objects[i].type == OBJ_DOOR) objects[i].isActive = true;
        }
    }

    // --- KEY & DEUR LOGICA ---
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_KEY && objects[i].isActive) {
            if (CheckCollisionRecs(player->rect, objects[i].rect)) {
                objects[i].isActive = false; // Opgepakt!
            }
        }
    }
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_DOOR) {
            bool locked = false;
            for (int j = 0; j < objectCount; j++) {
                // Als er nog *minstens 1* actieve key is met dezelfde linkID, blijft de deur op slot
                if (objects[j].type == OBJ_KEY && objects[j].linkID == objects[i].linkID && objects[j].isActive) {
                    locked = true; break;
                }
            }
            objects[i].isActive = locked;
        }
    }

    // --- DYNAMISCHE ONDERGROND ---
    float currentAccel = 1.0f; float currentMaxSpeed = 6.0f; float currentFriction = 0.8f; float currentJumpForce = -11.0f;
    Rectangle playerFeetCheck = { player->rect.x + 2, player->rect.y + player->rect.height, player->rect.width - 4, 2 };
    for (int i = 0; i < objectCount; i++) {
        // Zorg dat we niet door inactieve deuren worden beïnvloed!
        if (objects[i].type == OBJ_DOOR && !objects[i].isActive) continue; 
        
        if (CheckCollisionRecs(playerFeetCheck, objects[i].rect)) {
            if (objects[i].type == OBJ_MUD) { currentMaxSpeed = 2.5f; currentJumpForce = -7.5f; currentFriction = 0.5f; }
            else if (objects[i].type == OBJ_ICE) { currentAccel = 0.15f; currentMaxSpeed = 9.0f; currentFriction = 0.98f; }
        }
    }

    // --- TRIGGERS & CHECKPOINTS ---
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_TRIGGER) {
            Rectangle tRect = objects[i].rect; tRect.x += (tRect.width / 2.0f) - 5.0f; tRect.y += (tRect.height / 2.0f) - 5.0f;
            tRect.width = 10.0f; tRect.height = 10.0f;
            bool isTouching = CheckCollisionRecs(player->rect, tRect);
            if (isTouching && !objects[i].isActive) {
                objects[i].isActive = true;
                for (int j = 0; j < objectCount; j++) if (objects[j].type == OBJ_MOVING_BLOCK && objects[j].linkID == objects[i].linkID) objects[j].isActive = true;
            } else if (!isTouching && objects[i].isActive) objects[i].isActive = false;
        }
        else if (objects[i].type == OBJ_CHECKPOINT) {
            if (CheckCollisionRecs(player->rect, objects[i].rect)) { player->spawnX = objects[i].rect.x; player->spawnY = objects[i].rect.y; }
        }
    }

    // --- 3. BEWEGING VAN PLATFORMEN ÉN ZAGEN ---
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_MOVING_BLOCK || objects[i].type == OBJ_MOVING_HAZARD) {
            float oldX = objects[i].rect.x; float oldY = objects[i].rect.y;
            float oldProgress = objects[i].progress; int oldDir = objects[i].moveDir;
            float oldTimer = objects[i].waitTimer; float oldAngle = objects[i].currentAngle;
            bool oldActive = objects[i].isActive;

            Rectangle platformStandCheck = { player->rect.x + 2, player->rect.y + player->rect.height - 2, player->rect.width - 4, 4 };
            bool wasRiding = CheckCollisionRecs(platformStandCheck, objects[i].rect) && player->velocityY >= 0;

            if (objects[i].moveType == MOVE_CIRCLE) {
                if (objects[i].linkID == 0 || objects[i].isActive) {
                    objects[i].currentAngle += objects[i].moveSpeed * 0.02f;
                    objects[i].rect.x = (objects[i].startX + objects[i].moveRange) + cosf(objects[i].currentAngle) * objects[i].moveRange;
                    objects[i].rect.y = objects[i].startY + sinf(objects[i].currentAngle) * objects[i].moveRange;
                }
            } else {
                if (objects[i].linkID == 0) { 
                    if (objects[i].waitTimer > 0.0f) objects[i].waitTimer -= (1.0f / 60.0f);
                    else {
                        objects[i].progress += objects[i].moveSpeed * objects[i].moveDir;
                        if (objects[i].moveRange > 0) {
                            if (objects[i].progress >= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = -1; objects[i].waitTimer = 0.5f; }
                            else if (objects[i].progress <= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = 1; objects[i].waitTimer = 0.5f; }
                        } else {
                            if (objects[i].progress <= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = 1; objects[i].waitTimer = 0.5f; }
                            else if (objects[i].progress >= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = -1; objects[i].waitTimer = 0.5f; }
                        }
                    }
                } else { 
                    if (objects[i].isActive) {
                        objects[i].progress += objects[i].moveSpeed * objects[i].moveDir;
                        if (objects[i].moveRange > 0) {
                            if (objects[i].moveDir == 1 && objects[i].progress >= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = -1; objects[i].isActive = false; } 
                            else if (objects[i].moveDir == -1 && objects[i].progress <= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = 1; objects[i].isActive = false; }
                        } else {
                            if (objects[i].moveDir == -1 && objects[i].progress <= objects[i].moveRange) { objects[i].progress = objects[i].moveRange; objects[i].moveDir = 1; objects[i].isActive = false; } 
                            else if (objects[i].moveDir == 1 && objects[i].progress >= 0.0f) { objects[i].progress = 0.0f; objects[i].moveDir = -1; objects[i].isActive = false; }
                        }
                    }
                }
                if (objects[i].moveType == MOVE_HORZ) objects[i].rect.x = objects[i].startX + objects[i].progress;
                else objects[i].rect.y = objects[i].startY + objects[i].progress;
            }

            // ALLEEN MOVING BLOCKS DUWEN DE SPELER (Zagen snijden door je heen)
            if (objects[i].type == OBJ_MOVING_BLOCK) {
                float deltaX = objects[i].rect.x - oldX; float deltaY = objects[i].rect.y - oldY;
                bool isCrushed = false; bool playerNeedsPush = false; Rectangle testPlayer = player->rect;

                if (wasRiding) {
                    testPlayer.x += deltaX; testPlayer.y = objects[i].rect.y - player->rect.height; playerNeedsPush = true;
                } else if (CheckCollisionRecs(player->rect, objects[i].rect)) {
                    if (objects[i].moveType == MOVE_HORZ) {
                        if (deltaX > 0) testPlayer.x = objects[i].rect.x + objects[i].rect.width;
                        else if (deltaX < 0) testPlayer.x = objects[i].rect.x - player->rect.width;
                    } else if (objects[i].moveType == MOVE_VERT) {
                        if (deltaY > 0) testPlayer.y = objects[i].rect.y + objects[i].rect.height;
                        else if (deltaY < 0) testPlayer.y = objects[i].rect.y - player->rect.height;
                    } else if (objects[i].moveType == MOVE_CIRCLE) {
                        float pCX = player->rect.x + player->rect.width/2; float bCX = objects[i].rect.x + objects[i].rect.width/2;
                        float pCY = player->rect.y + player->rect.height/2; float bCY = objects[i].rect.y + objects[i].rect.height/2;
                        if (fabs(pCX - bCX) > fabs(pCY - bCY)) { if (pCX > bCX) testPlayer.x = objects[i].rect.x + objects[i].rect.width; else testPlayer.x = objects[i].rect.x - player->rect.width; } 
                        else { if (pCY > bCY) testPlayer.y = objects[i].rect.y + objects[i].rect.height; else testPlayer.y = objects[i].rect.y - player->rect.height; }
                    }
                    playerNeedsPush = true;
                }

                if (playerNeedsPush) {
                    Rectangle crushCheck = { testPlayer.x + 1.0f, testPlayer.y + 1.0f, testPlayer.width - 2.0f, testPlayer.height - 2.0f };
                    for (int k = 0; k < objectCount; k++) {
                        if (k != i && (objects[k].type == OBJ_WALL || objects[k].type == OBJ_MUD || objects[k].type == OBJ_ICE || objects[k].type == OBJ_MOVING_BLOCK || (objects[k].type == OBJ_DOOR && objects[k].isActive))) {
                            if (CheckCollisionRecs(crushCheck, objects[k].rect)) { isCrushed = true; break; }
                        }
                    }
                }

                if (isCrushed) {
                    objects[i].rect.x = oldX; objects[i].rect.y = oldY; objects[i].progress = oldProgress; objects[i].moveDir = oldDir;
                    objects[i].waitTimer = oldTimer; objects[i].currentAngle = oldAngle; objects[i].isActive = oldActive;
                } else if (playerNeedsPush) {
                    player->rect = testPlayer; 
                    if (wasRiding || (objects[i].moveType == MOVE_VERT && deltaY < 0 && testPlayer.y < objects[i].rect.y)) { player->velocityY = 0; player->canJump = true; }
                }
            }
        }
    }

    // --- 4. SPELER BESTURING ---
    if (IsKeyDown(KEY_RIGHT)) player->velocityX += currentAccel;
    else if (IsKeyDown(KEY_LEFT)) player->velocityX -= currentAccel;
    else player->velocityX *= currentFriction;

    if (player->velocityX > currentMaxSpeed) player->velocityX = currentMaxSpeed;
    if (player->velocityX < -currentMaxSpeed) player->velocityX = -currentMaxSpeed;

    if (IsKeyPressed(KEY_SPACE)) {
        if (player->canJump) { player->velocityY = currentJumpForce; player->canJump = false; } 
        else if (player->isOnWall) { player->velocityY = -11.0f; player->velocityX = -player->wallDir * 6.0f; player->isOnWall = false; }
    }
    if (IsKeyReleased(KEY_SPACE) && player->velocityY < 0) player->velocityY *= 0.5f; 
    player->velocityY += gravity;
    if (player->isOnWall && player->velocityY > 0 && !player->canJump) { if (player->velocityY > wallSlideSpeed) player->velocityY = wallSlideSpeed; }

    // --- 5. DEATH CHECK (Perfecte Hitboxes) ---
    bool isDead = false;
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_HAZARD) {
            // Spikes: Hitbox is kleiner gemaakt zodat je alleen doodgaat als je hem echt raakt
            Rectangle spikeHitbox = { objects[i].rect.x + 10, objects[i].rect.y + 10, objects[i].rect.width - 20, objects[i].rect.height - 10 };
            if (CheckCollisionRecs(player->rect, spikeHitbox)) isDead = true;
        } else if (objects[i].type == OBJ_MOVING_HAZARD) {
            // Zagen: Perfecte Cirkel Collision
            Vector2 center = { objects[i].rect.x + objects[i].rect.width/2, objects[i].rect.y + objects[i].rect.height/2 };
            if (CheckCollisionCircleRec(center, (objects[i].rect.width/2) - 2.0f, player->rect)) isDead = true;
        }
    }
    if (isDead) {
        player->rect.x = player->spawnX; player->rect.y = player->spawnY;
        player->velocityX = 0; player->velocityY = 0; player->deathCount++;
    }

    // --- 6. SOLIDE X-COLLISION ---
    float oldPlayerX = player->rect.x; player->rect.x += player->velocityX; player->isOnWall = false; 
    Rectangle playerHitboxX = { player->rect.x, player->rect.y + 2.0f, player->rect.width, player->rect.height - 4.0f };
    for (int i = 0; i < objectCount; i++) {
        // Gevaarlijke objecten zijn niet solide muren!
        if (objects[i].type == OBJ_SPAWN || objects[i].type == OBJ_CHECKPOINT || objects[i].type == OBJ_TRIGGER || objects[i].type == OBJ_KEY || objects[i].type == OBJ_HAZARD || objects[i].type == OBJ_MOVING_HAZARD) continue; 
        if (objects[i].type == OBJ_DOOR && !objects[i].isActive) continue; 
        
        if (CheckCollisionRecs(playerHitboxX, objects[i].rect)) {
            if (objects[i].type == OBJ_END) { player->hasWon = true; continue; }
            if (oldPlayerX + player->rect.width <= objects[i].rect.x) { player->rect.x = objects[i].rect.x - player->rect.width; player->velocityX = 0; player->isOnWall = true; player->wallDir = 1; } 
            else if (oldPlayerX >= objects[i].rect.x + objects[i].rect.width) { player->rect.x = objects[i].rect.x + objects[i].rect.width; player->velocityX = 0; player->isOnWall = true; player->wallDir = -1; } 
            else {
                if (player->rect.x + player->rect.width/2.0f < objects[i].rect.x + objects[i].rect.width/2.0f) { player->rect.x = objects[i].rect.x - player->rect.width; player->velocityX = 0; player->isOnWall = true; player->wallDir = 1; } 
                else { player->rect.x = objects[i].rect.x + objects[i].rect.width; player->velocityX = 0; player->isOnWall = true; player->wallDir = -1; }
            }
        }
    }

    // --- 7. SOLIDE Y-COLLISION ---
    float oldPlayerY = player->rect.y; player->rect.y += player->velocityY; player->canJump = false; 
    Rectangle playerHitboxY = { player->rect.x + 2.0f, player->rect.y, player->rect.width - 4.0f, player->rect.height };
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == OBJ_SPAWN || objects[i].type == OBJ_CHECKPOINT || objects[i].type == OBJ_TRIGGER || objects[i].type == OBJ_KEY || objects[i].type == OBJ_HAZARD || objects[i].type == OBJ_MOVING_HAZARD) continue; 
        if (objects[i].type == OBJ_DOOR && !objects[i].isActive) continue; 
        
        if (CheckCollisionRecs(playerHitboxY, objects[i].rect)) {
            if (objects[i].type == OBJ_END) { player->hasWon = true; continue; }
            if (oldPlayerY + player->rect.height <= objects[i].rect.y) { player->rect.y = objects[i].rect.y - player->rect.height; player->velocityY = 0; player->canJump = true; player->isOnWall = false; } 
            else if (oldPlayerY >= objects[i].rect.y + objects[i].rect.height) { player->rect.y = objects[i].rect.y + objects[i].rect.height; player->velocityY = 0; }
            else {
                if (player->rect.y + player->rect.height/2.0f < objects[i].rect.y + objects[i].rect.height/2.0f) { player->rect.y = objects[i].rect.y - player->rect.height; player->velocityY = 0; player->canJump = true; player->isOnWall = false; } 
                else { player->rect.y = objects[i].rect.y + objects[i].rect.height; player->velocityY = 0; }
            }
        }
    }

    // --- 8. CAMERA DEADZONES ---
    float deadzoneX = 100.0f; float deadzoneY = 80.0f; 
    if (player->rect.x > camera->target.x + deadzoneX) camera->target.x = player->rect.x - deadzoneX;
    if (player->rect.x < camera->target.x - deadzoneX) camera->target.x = player->rect.x + deadzoneX;
    if (player->rect.y > camera->target.y + deadzoneY) camera->target.y = player->rect.y - deadzoneY;
    if (player->rect.y < camera->target.y - deadzoneY) camera->target.y = player->rect.y + deadzoneY;

    float halfScreenW = 400.0f; float halfScreenH = 225.0f;
    if (camera->target.x < bounds.x + halfScreenW) camera->target.x = bounds.x + halfScreenW;
    if (camera->target.x > bounds.x + bounds.width - halfScreenW) camera->target.x = bounds.x + bounds.width - halfScreenW;
    if (camera->target.y < bounds.y + halfScreenH) camera->target.y = bounds.y + halfScreenH;
    if (camera->target.y > bounds.y + bounds.height - halfScreenH) camera->target.y = bounds.y + bounds.height - halfScreenH;
}

void DrawPlayer(Player *player) {
    if (player->isOnWall) DrawRectangleRec(player->rect, RED);
    else DrawRectangleRec(player->rect, MAROON);
}