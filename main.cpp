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

struct Enemy {
    int x, y;
    bool isDead;
    bool isStopped;
    int hitCount;
    bool isMovingRight;  // true if moving right
    int stopPosition;
};

// Thêm biến đếm đạn toàn cục
const int MAX_BULLETS = 20;  // Số đạn tối đa
int bulletCount = MAX_BULLETS;  // Số đạn hiện tại
bool canShoot = true;  // Trạng thái có thể bắn

// Thêm struct cho bullet box
struct BulletBox {
    int x;
    bool isVisible;
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

    // --- Enemy wave management ---
    vector<Enemy> waveEnemies;
    Uint32 lastWaveTime = SDL_GetTicks();
    const Uint32 waveDelay = 10000; // 10 seconds

    // --- NPC position & movement ---
    int xPos = 650;                             // bắt đầu tại trái màn
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
    SDL_Texture*   bulletbox      = graphics.loadTexture(BULLET_BOX_IMG);
    const int      maxBulletX  = SCREEN_WIDTH;

    // --- Enemy & collision
    const int stopR     = 1400;
    const int stopL     = 40;
    bool rightStopped = false, leftStopped = false;
    const int enemySpeed= 1;
    bool enemyStopped = false;
    int hitCountR = 0;     // Số lần enemy phải bị bắn trúng
    int hitCountL = 0;     // Số lần enemy trái bị bắn trúng
    bool enemyRDead = false;  // Trạng thái chết của enemy phải
    bool enemyLDead = false;  // Trạng thái chết của enemy trái
    int enemyR = 1540;
    int enemyL = -5;
    const int enemyY_run = 790;
    const int enemyY_stop = groundY - npcFrameH/2 + 110;
    bool npcDead = false;
    bool gameOver = false;  // Trạng thái kết thúc game

    // --- Enemy bullets ---
    vector<EnemyBullet> enemyBullets;
    const int enemyBulletSpeed = 20;
    Uint32 lastEnemyShootTime = SDL_GetTicks();
    const Uint32 enemyShootDelay = 3000; // Shoot every 3 seconds

    // --- Enemy sprites ---
    Sprite enemy_run, enemy_run_flip;
    SDL_Texture* enemystandflipTex  = graphics.loadTexture(ENEMY_FLIP_IMG);
    SDL_Texture* enemyFlipTex   = graphics.loadTexture(ENEMY_RUN_FLIP_IMG);
    SDL_Texture* enemyTex       = graphics.loadTexture(ENEMY_IMG);
    SDL_Texture* enemyrunTex    = graphics.loadTexture(ENEMY_RUN_IMG);
    enemy_run.init(enemyrunTex, ENEMY_FRAMES, ENEMY_CLIPS);
    enemy_run_flip.init(enemyFlipTex, ENEMY_FRAMES, ENEMY_CLIPS);

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

    // Khởi tạo bullet box
    BulletBox bulletBox = {400, false};
    Uint32 lastBulletBoxTime = SDL_GetTicks();
    const Uint32 bulletBoxDelay = 10000; // 10 seconds
    const int bulletBoxY = 870;

    while (!quit && !gameOver) {
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
                        if (isGunOut && !isJumping && canShoot && bulletCount > 0) {
                            int dx = (npcDirection == RIGHT) ? 20 : -20;
                            // spawnX: nếu bắn trái thì nên sinh bên cạnh trái sprite
                            int spawnX = (npcDirection == RIGHT)
                                         ? xPos + (npcFrameW/1.5)
                                         : xPos + npcFrameW/5;
                            int spawnY = yPos + (npcFrameH/2.5);

                            bullets.push_back({ spawnX, spawnY, dx });
                            bulletCount--;  // Giảm số đạn

                        }
                        break;
                    case SDLK_1:
                        // Giơ/cất súng
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
            // Cập nhật vị trí đạn
            it->x += it->speed;

            bool bulletHit = false;

            const float ENEMY_HIT_RATIO = 0.7f;
            int hitW = int(npcFrameW * ENEMY_HIT_RATIO);
            int hitH = int(npcFrameH * ENEMY_HIT_RATIO);
            int offX = (npcFrameW - hitW) / 2;
            int offY = (npcFrameH - hitH) / 2;


            // Kiểm tra va chạm với enemy phải
            if (!enemyRDead && rightStopped) {
                   SDL_Rect enemyBox = {
                    stopR + offX,
                    enemyY_stop + offY,
                    hitW,
                    hitH
                };
                SDL_Rect bulletBox = {it->x, it->y, 3, 3};  // Giảm kích thước đạn
                if (SDL_HasIntersection(&enemyBox, &bulletBox)) {
                    hitCountR++;
                    bulletHit = true;
                    if (hitCountR >= 10) {
                        enemyRDead = true;
                    }
                }
            }


            // Kiểm tra va chạm với enemy trái
            if (!enemyLDead && leftStopped && !bulletHit) {
                SDL_Rect enemyBox = {
                    stopL - offX*3,           // cộng chứ không gán
                    enemyY_stop + offY,
                    hitW,
                    hitH
                };  // Giảm kích thước hitbox
                SDL_Rect bulletBox = {it->x, it->y, 5, 5};  // Giảm kích thước đạn
                if (SDL_HasIntersection(&enemyBox, &bulletBox)) {
                    hitCountL++;
                    bulletHit = true;
                    if (hitCountL >= 10) {
                        enemyLDead = true;
                    }
                }
            }

            // Xóa đạn nếu trúng hoặc ra khỏi màn hình
            if (bulletHit || it->x > SCREEN_WIDTH || it->x < 0) {
                it = bullets.erase(it);
            } else {
                ++it;
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
                enemy_run.tick();  // sprite chạy bên phải
                lastAnimTime = now2;
            }
            if (!leftStopped && now2 - lastAnimTime >= animDelay) {
                enemy_run_flip.tick();    // sprite chạy bên trái
                lastAnimTime = now2;
            }

        // Spawn new wave every 10 seconds
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastWaveTime >= waveDelay) {
            // Calculate offset based on number of existing waves
            int offset = waveEnemies.size() / 2 * -1;  // -1 for each pair of enemies

            // Spawn left enemy
            Enemy leftEnemy;
            leftEnemy.x = -5;
            leftEnemy.y = enemyY_run;
            leftEnemy.isDead = false;
            leftEnemy.isStopped = false;
            leftEnemy.hitCount = 0;
            leftEnemy.isMovingRight = true;
            leftEnemy.stopPosition = stopL + offset;  // Add offset to stop position
            waveEnemies.push_back(leftEnemy);

            // Spawn right enemy
            Enemy rightEnemy;
            rightEnemy.x = 1540;
            rightEnemy.y = enemyY_run;
            rightEnemy.isDead = false;
            rightEnemy.isStopped = false;
            rightEnemy.hitCount = 0;
            rightEnemy.isMovingRight = false;
            rightEnemy.stopPosition = stopR - offset;  // Subtract offset for right side
            waveEnemies.push_back(rightEnemy);

            lastWaveTime = currentTime;
        }

        // Update wave enemies
        for (auto& enemy : waveEnemies) {
            if (!enemy.isDead && !enemy.isStopped) {
                if (enemy.isMovingRight) {
                    enemy.x += enemySpeed;
                    if (enemy.x >= enemy.stopPosition) {  // Use individual stop position
                        enemy.x = enemy.stopPosition;
                        enemy.isStopped = true;
                        enemy.y = enemyY_stop;
                    }
                } else {
                    enemy.x -= enemySpeed;
                    if (enemy.x <= enemy.stopPosition) {  // Use individual stop position
                        enemy.x = enemy.stopPosition;
                        enemy.isStopped = true;
                        enemy.y = enemyY_stop;
                    }
                }
            }
        }

        // Update bullet collisions with wave enemies
        for (auto it = bullets.begin(); it != bullets.end();) {
            bool bulletHit = false;

            const float ENEMY_HIT_RATIO = 0.7f;
            int hitW = int(npcFrameW * ENEMY_HIT_RATIO);
            int hitH = int(npcFrameH * ENEMY_HIT_RATIO);
            int offX = (npcFrameW - hitW) / 2;
            int offY = (npcFrameH - hitH) / 2;

            // Check collision with wave enemies
            for (auto& enemy : waveEnemies) {
                if (!enemy.isDead && enemy.isStopped) {
                    SDL_Rect enemyBox = {
                        enemy.x + (enemy.isMovingRight ? -offX*3 : offX),
                        enemy.y + offY,
                        hitW,
                        hitH
                    };
                    SDL_Rect bulletBox = {it->x, it->y, 3, 3};

                    if (SDL_HasIntersection(&enemyBox, &bulletBox)) {
                        enemy.hitCount++;
                        bulletHit = true;
                        if (enemy.hitCount >= 10) {
                            enemy.isDead = true;
                        }
                        break;
                    }
                }
            }

            if (bulletHit || it->x > SCREEN_WIDTH || it->x < 0) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }

        // Wave enemies shooting
        for (const auto& enemy : waveEnemies) {
            if (!enemy.isDead && enemy.isStopped &&
                currentTime - lastEnemyShootTime >= enemyShootDelay) {
                int bulletSpeed = enemy.isMovingRight ? enemyBulletSpeed : -enemyBulletSpeed;
                enemyBullets.push_back({enemy.x, enemyY_stop + 50, bulletSpeed});
            }
        }

        // Original enemies shooting
        if (currentTime - lastEnemyShootTime >= enemyShootDelay) {
            if (!enemyLDead && leftStopped) {
                enemyBullets.push_back({stopL, enemyY_stop +50, enemyBulletSpeed});
            }
            if (!enemyRDead && rightStopped) {
                enemyBullets.push_back({stopR, enemyY_stop +50, -enemyBulletSpeed});
            }
            lastEnemyShootTime = currentTime;
        }

        // Update enemy bullets
        for (auto it = enemyBullets.begin(); it != enemyBullets.end();) {
            it->x += it->speed;

            // Check collision with player
            const float NPC_HIT_RATIO = 0.25f;
            int hitW = int(npcFrameW * NPC_HIT_RATIO);
            int hitH = int(npcFrameH * NPC_HIT_RATIO);
            int offX = (npcFrameW - hitW) / 2.5;
            int offY = (npcFrameH - hitH) / 2;

            SDL_Rect npcBox = {
                xPos + offX,
                yPos + offY,
                hitW,
                hitH
            };

            SDL_Rect bulletBox = {it->x, it->y, 5, 5};

            if (SDL_HasIntersection(&npcBox, &bulletBox)) {
                gameOver = true;
                break;
            }

            if (it->x > SCREEN_WIDTH || it->x < 0) {
                it = enemyBullets.erase(it);
            } else {
                ++it;
            }
        }

        // Update wave enemies movement and animation
        static Uint32 lastWaveAnimTime = SDL_GetTicks();
        const Uint32 waveAnimDelay = 200;

        for (auto& enemy : waveEnemies) {
            if (!enemy.isDead && !enemy.isStopped) {
                // Update movement
                if (enemy.isMovingRight) {
                    enemy.x += enemySpeed;
                    if (enemy.x >= enemy.stopPosition) {  // Use individual stop position
                        enemy.x = enemy.stopPosition;
                        enemy.isStopped = true;
                        enemy.y = enemyY_stop;
                    }
                } else {
                    enemy.x -= enemySpeed;
                    if (enemy.x <= enemy.stopPosition) {  // Use individual stop position
                        enemy.x = enemy.stopPosition;
                        enemy.isStopped = true;
                        enemy.y = enemyY_stop;
                    }
                }

                // Only animate if still moving
                if (!enemy.isStopped && currentTime - lastWaveAnimTime >= waveAnimDelay) {
                    if (enemy.isMovingRight) {
                        enemy_run.tick();
                    } else {
                        enemy_run_flip.tick();
                    }
                    lastWaveAnimTime = currentTime;
                }
            }
        }

        // Update animation timer after processing all enemies
        if (currentTime - lastWaveAnimTime >= waveAnimDelay) {
            lastWaveAnimTime = currentTime;
        }

        // Cập nhật vị trí bullet box mỗi 10 giây
        if (currentTime - lastBulletBoxTime >= bulletBoxDelay) {
            bulletBox.x = rand() % (SCREEN_WIDTH - 50); // 50 là chiều rộng của bulletbox
            bulletBox.isVisible = true;
            lastBulletBoxTime = currentTime;
        }

        // Kiểm tra va chạm với bullet box
        if (bulletBox.isVisible) {
            SDL_Rect playerBox = {
                xPos + (npcFrameW/4),
                yPos + (npcFrameH/4),
                npcFrameW/2,
                npcFrameH/2
            };

            SDL_Rect ammoBox = {
                bulletBox.x,
                bulletBoxY,
                50,  // chiều rộng bulletbox
                50   // chiều cao bulletbox
            };

            if (SDL_HasIntersection(&playerBox, &ammoBox)) {
                bulletBox.isVisible = false;
                bulletCount = MAX_BULLETS;  // Nạp lại đạn
                canShoot = true;
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

        // Render wave enemies
        for (const auto& enemy : waveEnemies) {
            if (!enemy.isDead) {
                if (!enemy.isStopped) {
                    if (enemy.isMovingRight) {
                        graphics.rendert(enemy.x, enemy.y, enemy_run);
                    } else {
                        graphics.rendert(enemy.x, enemy.y, enemy_run_flip);
                    }
                } else {
                    if (enemy.isMovingRight) {
                        graphics.renderTexture(enemyTex, enemy.x, enemy.y);
                    } else {
                        graphics.renderTexture(enemystandflipTex, enemy.x, enemy.y);
                    }
                }
            }
        }

        // 6.2) Vẽ robot nếu chưa "chết"
        if (!enemyRDead) {
            if (!rightStopped)
                graphics.rendert(enemyR, enemyY_run, enemy_run_flip);
            else
                graphics.renderTexture(enemystandflipTex, stopR, enemyY_stop);
        }

        if (!enemyLDead) {
            if (!leftStopped)
                graphics.rendert(enemyL, enemyY_run, enemy_run);
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

        // Vẽ bullet box nếu đang visible
        if (bulletBox.isVisible) {
            graphics.renderTexture(bulletbox, bulletBox.x, bulletBoxY);
        }

        graphics.presentScene();
        SDL_Delay(16);
    }

    // Hiển thị màn hình game over nếu người chơi bị bắn trúng
    if (gameOver) {
        SDL_Delay(1000);  // Dừng 1 giây trước khi thoát
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
    SDL_DestroyTexture(bulletbox);
    graphics.quit();
    return 0;
}
