#ifndef _GRAPHICS__H
#define _GRAPHICS__H

#include <SDL.h>
#include <SDL_image.h>
#include "defs.h"

#include <vector>

using namespace std;

struct ScrollingBackground
{
        SDL_Texture* texture;
    int scrollingOffset = 0;
    int width, height;
void setTexture(SDL_Texture* _texture);
void scroll(int distance);
};

struct Sprite;

struct Graphics {
    SDL_Renderer *renderer;
	SDL_Window *window;

	void logErrorAndExit(const char* msg, const char* error);
	void init();
	void prepareScene(SDL_Texture * background = nullptr);
    void presentScene();
    SDL_Texture *loadTexture(const char *filename);
    void renderTexture(SDL_Texture *texture, int x, int y);
    void render(const ScrollingBackground& bgr);
    void renderflip (const ScrollingBackground& bgr);
    void rendert(int x, int y, const Sprite& sprite);
    void quit();
};

struct Sprite
 {
    SDL_Texture* texture;
    std::vector<SDL_Rect> clips;
    int currentFrame = 0;
    void init(SDL_Texture* _texture, int frames, const int _clips [][4]);
    void tick();
    const SDL_Rect* getCurrentClip() const;

};


#endif
