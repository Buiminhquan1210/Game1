#ifndef _DEFS__H
#define _DEFS__H

const int SCREEN_WIDTH = 1536;
const int SCREEN_HEIGHT = 1024;

#define WINDOW_TITLE "Gamevui!"

#define BACKGROUND_IMG        "background.png"
#define NPC_RUN_IMG           "npc_run.png"
#define NPC_RUN_FLIP_IMG      "npc_run_flipped.png"
#define NPC_STAND_IMG         "npc_stand.png"
#define NPC_STAND_FLIP_IMG    "npc_stand_flipped.png"
#define NPC_GUN_IMG           "npc_gun.png"
#define NPC_GUN_FLIP_IMG      "npc_gun_flipped.png"
#define NPC_JUMP_IMG          "npc_jump.png"
#define NPC_JUMP_FLIP_IMG     "npc_jump_flipped.png"
#define BULLET_IMG            "bullet.png"
#define ENEMY_RUN_FLIP_IMG    "enemy_run_flipped.jpg"
#define ENEMY_FLIP_IMG        "enemychinh_flip.png"
#define LASER_IMG             "laser.png"
#define ENEMY_BULLET_IMG      "enemy_bullet.png"
#define ENEMY_IMG             "enemychinh.png"
#define ENEMY_RUN_IMG         "enemy_run.jpg"


const int NPC_CLIPS[][4] =
{
    { 0, 0, 250, 150},
    { 250, 0, 250, 150},
    {500, 0, 250, 150},
    {750, 0, 250, 150},
    {1000, 0, 250, 150},
    {1250, 0, 250, 150},
    {1500, 0, 250, 150},
    {1750, 0, 250, 150},
};
const int NPC_FRAMES = sizeof(NPC_CLIPS)/sizeof(int)/4;

const int ENEMY_CLIPS[][4] =
{
    {   0,   0, 129, 159},
    {129,    0, 129, 159},
    { 258,   0, 129, 159},
    { 387,   0, 129, 159},
    { 516,   0, 129, 159},
    {645,    0, 129, 159},
    {774, 0, 129, 159},
    {903, 0 , 129, 159},
    {1032, 0, 129, 159},
};
const int ENEMY_FRAMES = sizeof(ENEMY_CLIPS) / (sizeof(int) * 4);

const int enemyFrameW = ENEMY_CLIPS[0][2];
const int enemyFrameH = ENEMY_CLIPS[0][3];

const int NPC_JUMP_CLIPS[][4] = {
    {   0,  0, 250, 150 },
    { 250,  0, 250, 150 }
};
const int NPC_JUMP_FRAMES = sizeof(NPC_JUMP_CLIPS) / sizeof(int) / 4;

#endif
