#pragma once
#include "include/SDL2/SDL.h"
#include "include/SDL2/SDL_ttf.h"
#include <string>
#include <vector>

class VideoDisplay {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    TTF_Font* font;

    int videoW, videoH;

    // Internal helper to draw a single button with state logic
    void drawButton(SDL_Rect rect, std::string text, int mx, int my, bool mouseDown, bool isEnabled);

public:
    VideoDisplay();
    ~VideoDisplay();

    bool init();
    void renderFrame(const std::vector<uint8_t>& rgb, int w, int h);
    
    // Updated: Now accepts mouse coordinates and click state
    void renderUI(int clientState, int playSeconds, int mx, int my, bool mouseDown);
    
    SDL_Renderer* getRenderer() { return renderer; }
};