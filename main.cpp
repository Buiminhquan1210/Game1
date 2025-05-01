#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>
#include "graphics.h"
#include "defs.h"
using namespace std;

struct Bullet {
    int x, y;
    int speed;
};

struct EnemyBullet {
    int x, y;
    int speed;
};

int main(int argc, char *argv[])
{
    Graphics graphics;
    graphics.init();
    int x,y;

    bool quit        = false;
    bool isGunOut    = false;
    enum Direction { RIGHT, LEFT };
    Direction npcDirection = RIGHT;
    SDL_Event event;

    // --- NPC position & movement ---
    int xPos = 0;                             // bắt đầu tại trái màn
    const int walkSpeed = 5;
    int npcFrameW = NPC_CLIPS[0][2];
    int npcFrameH = NPC_CLIPS[0][3];

    // --- Nhảy ---
    const int groundY   = 770;
    int       yPos      = groundY;
    int       velocityY = 0;
    const int jumpSpeed = -18;
    const int gravity   = 1;
    bool      isJumping = false;

    // --- Đạn ---
    vector<Bullet> bullets;
    SDL_Texture*   bulletTex   = graphics.loadTexture(BULLET_IMG);
    SDL_Texture*   enemybulletTex = graphics.loadTexture(ENEMY_BULLET_IMG);
    const int      maxBulletX  = SCREEN_WIDTH;

    // --- Enemy & collision (giữ y nguyên, bạn có thể customize) ---
    const int stopR     = 1400;
    const int stopL     = 40;
    bool rightStopped = false, leftStopped = false;
    const int enemySpeed= 1;
    bool        enemyStopped = false;
    int         hitCount     = 0;
    bool        enemyDead    = false;
    int         enemyR       = 1540;
    int         enemyL       = -5;
    const int enemyY_run  = 790;                         // trùng với graphics.rendert(enemyL,790,…)
    const int enemyY_stop = groundY - npcFrameH/2 + 110;
    bool        npcDead     = false;

    // --- Enemy bullets ---
    vector<EnemyBullet> enemyBullets;
    const int enemyBulletSpeed = 20;
    Uint32 lastEnemyShootTime = SDL_GetTicks();
    const Uint32 enemyShootDelay = 3000; // Shoot every 5 seconds

    Sprite      enemy_f, enemy;
    SDL_Texture* enemystandflipTex  = graphics.loadTexture(ENEMY_FLIP_IMG);
    SDL_Texture* enemyFlipTex   = graphics.loadTexture(ENEMY_RUN_FLIP_IMG);
    SDL_Texture* enemyTex       = graphics.loadTexture(ENEMY_IMG);
    SDL_Texture* enemyrunTex    = graphics.loadTexture(ENEMY_RUN_IMG);
    enemy_f.init(enemyFlipTex, ENEMY_FRAMES, ENEMY_CLIPS);
    enemy.init(enemyrunTex, ENEMY_FRAMES, ENEMY_CLIPS);

    // --- Background tĩnh ---
    SDL_Texture* bgTex = graphics.loadTexture(BACKGROUND_IMG);

    // --- NPC sprites ---
    SDL_Texture* runTex     = graphics.loadTexture(NPC_RUN_IMG);
    SDL_Texture* runFlipTex = graphics.loadTexture(NPC_RUN_FLIP_IMG);
    SDL_Texture* standTex   = graphics.loadTexture(NPC_STAND_IMG);
    SDL_Texture* standFlipTex = graphics.loadTexture(NPC_STAND_FLIP_IMG);
    SDL_Texture* gunTex     = graphics.loadTexture(NPC_GUN_IMG);
    SDL_Texture* gunFlipTex = graphics.loadTexture(NPC_GUN_FLIP_IMG);
    SDL_Texture* jumpTex    = graphics.loadTexture(NPC_JUMP_IMG);
    SDL_Texture* jumpFlipTex= graphics.loadTexture(NPC_JUMP_FLIP_IMG);

    Sprite npc_run, npc_run_f;
    npc_run.init(  runTex,      NPC_FRAMES, NPC_CLIPS);
    npc_run_f.init(runFlipTex,  NPC_FRAMES, NPC_CLIPS);

    Uint32 lastRunTime = SDL_GetTicks();
    const Uint32 runDelay = 100; // ms giữa 2 frame chạy

    while (!quit) {
        // 1) XỬ LÝ INPUT sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            SDL_GetMouseState(&x, &y);
            cerr << x << ", " << y << endl;
             if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        // Nhảy
                        if (!isJumping && yPos == groundY) {
                            isJumping  = true;
                            velocityY  = jumpSpeed;
                        }
                        break;
                    case SDLK_SPACE:
                        if (isGunOut && !isJumping) {
                            int dx = (npcDirection == RIGHT) ? 20 : -20;
                            // spawnX: nếu bắn trái thì nên sinh bên cạnh trái sprite
                            int spawnX = (npcDirection == RIGHT)
                                         ? xPos + (npcFrameW/1.5)
                                         : xPos + npcFrameW/5;       // hoặc xPos - 1
                            int spawnY = yPos + (npcFrameH/2.5);

                            bullets.push_back({ spawnX, spawnY, dx });
                        }
                        break;
                    case SDLK_1:
                        // Giơ/cất súng (nếu bạn vẫn cần)
                        isGunOut = !isGunOut;
                        break;
                    default:
                        break;
                }
            }
        }

        // 2) CẬP NHẬT DI CHUYỂN ←/→ và animation chạy
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        bool moving = false;

        if (keys[SDL_SCANCODE_RIGHT]) {
            npcDirection = RIGHT;
            xPos += walkSpeed;
            if (xPos > SCREEN_WIDTH - npcFrameW) xPos = SCREEN_WIDTH - npcFrameW;
            moving = true;
        }
        else if (keys[SDL_SCANCODE_LEFT]) {
            npcDirection = LEFT;
            xPos -= walkSpeed;
            if (xPos <= 0) xPos = 0;
            moving = true;
        }

        // khi đang chạy và không nhảy, tick animation
        if (moving && !isJumping) {
            Uint32 now = SDL_GetTicks();
            if (now - lastRunTime >= runDelay) {
                npc_run.tick();
                npc_run_f.tick();
                lastRunTime = now;
            }
        }

        // 3) CẬP NHẬT NHẢY + gravity
        if (isJumping) {
            yPos      += velocityY;
            velocityY += gravity;
            if (yPos >= groundY) {
                yPos      = groundY;
                isJumping = false;
                velocityY = 0;
            }
        }

        // 4) CẬP NHẬT ĐẠN + va chạm vào enemy khi đã dừng
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->x += it->speed;
            bool erased = false;

            if (enemyStopped && !enemyDead) {
                SDL_Rect enemyBox  = { stopR, groundY - npcFrameH, npcFrameW, npcFrameH };
                SDL_Rect bulletBox = { it->x, it->y, 1, 1 };
                if (SDL_HasIntersection(&enemyBox, &bulletBox)) {
                    hitCount++;
                    it = bullets.erase(it);
                    erased = true;
                    if (hitCount >= 5) enemyDead = true;
                }
            }

            if (!erased) {
                if (it->x > maxBulletX || it->x < 0) it = bullets.erase(it);
                else                    ++it;
            }
        }

        // 5) CẬP NHẬT enemyL → dừng tại stopX
        if (!rightStopped) {
                enemyR -= enemySpeed;
                if (enemyR <= stopR) {
                    enemyR       = stopR;
                    rightStopped = true;
                }
            }
            if (!leftStopped) {
                enemyL += enemySpeed;
                if (enemyL >= stopL) {
                    enemyL      = stopL;
                    leftStopped = true;
                }
            }

            // 6) Animate 2 con nếu chúng còn chạy
            static Uint32 lastAnimTime = SDL_GetTicks();
            const Uint32 animDelay     = 100;
            Uint32 now2 = SDL_GetTicks();
            if (!rightStopped && now2 - lastAnimTime >= animDelay) {
                enemy_f.tick();  // sprite chạy bên phải
                lastAnimTime = now2;
            }
            if (!leftStopped && now2 - lastAnimTime >= animDelay) {
                enemy.tick();    // sprite chạy bên trái
                lastAnimTime = now2;
            }


        // Update enemy bullets
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastEnemyShootTime >= enemyShootDelay) {
            // Left enemy shoots from fixed position
            enemyBullets.push_back({150, 860, enemyBulletSpeed});
            // Right enemy shoots from fixed position
            enemyBullets.push_back({1360, 860, -enemyBulletSpeed});
            lastEnemyShootTime = currentTime;
        }

        // Update enemy bullets position and check collision with NPC
        for (auto it = enemyBullets.begin(); it != enemyBullets.end();) {
            it->x += it->speed;

            // Check collision with NPC
            if (!npcDead) {
                SDL_Rect npcBox = {xPos, yPos, npcFrameW, npcFrameH};
                SDL_Rect bulletBox = {it->x, it->y, 10, 10};
                if (SDL_HasIntersection(&npcBox, &bulletBox)) {
                    npcDead = true;
                    it = enemyBullets.erase(it);
                    continue;
                }
            }

            // Remove bullets that go off screen
            if (it->x < 0 || it->x > SCREEN_WIDTH) {
                it = enemyBullets.erase(it);
            } else {
                ++it;
            }
        }

        // --- RENDER TẤT CẢ ---
        graphics.prepareScene(bgTex);

        // 6.1) Vẽ đạn
        for (auto &b : bullets)
            graphics.renderTexture(bulletTex, b.x, b.y);

        // Render enemy bullets
        for (auto &b : enemyBullets) {
            graphics.renderTexture(enemybulletTex, b.x, b.y);
        }

        // 6.2) Vẽ robot nếu chưa "chết"
        if (!enemyDead) {
            if (!rightStopped)
                graphics.rendert(enemyR, enemyY_run, enemy_f);
            else
                graphics.renderTexture(enemystandflipTex, stopR, enemyY_stop);
            // Vẽ con trái
            if (!leftStopped)
                graphics.rendert(enemyL, enemyY_run, enemy);
            else
                graphics.renderTexture(enemyTex, stopL, enemyY_stop);
        }
        // 6.3) Vẽ NPC
        if (isJumping) {
            if (npcDirection == RIGHT)
                graphics.renderTexture(jumpTex, xPos, yPos);
            else
                graphics.renderTexture(jumpFlipTex, xPos, yPos);
        }
        else if (moving) {
            if (npcDirection == RIGHT)
                graphics.rendert(xPos, yPos, npc_run);
            else
                graphics.rendert(xPos, yPos, npc_run_f);
        }
        else {
            if (npcDirection == RIGHT) {
                if (isGunOut) graphics.renderTexture(gunTex,      xPos, yPos);
                else          graphics.renderTexture(standTex,    xPos, yPos);
            } else {
                if (isGunOut) graphics.renderTexture(gunFlipTex,  xPos, yPos);
                else          graphics.renderTexture(standFlipTex,xPos, yPos);
            }
        }
        graphics.presentScene();
        SDL_Delay(16);
    }

    // --- CLEAN UP ---
    SDL_DestroyTexture(bgTex);
    SDL_DestroyTexture(bulletTex);
    SDL_DestroyTexture(enemyFlipTex);
    SDL_DestroyTexture(enemystandflipTex);
    SDL_DestroyTexture(runTex);
    SDL_DestroyTexture(runFlipTex);
    SDL_DestroyTexture(standTex);
    SDL_DestroyTexture(standFlipTex);
    SDL_DestroyTexture(gunTex);
    SDL_DestroyTexture(gunFlipTex);
    SDL_DestroyTexture(jumpTex);
    SDL_DestroyTexture(jumpFlipTex);
    SDL_DestroyTexture(enemybulletTex);
    graphics.quit();
    return 0;
}
