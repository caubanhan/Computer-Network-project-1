#include "VideoDisplay.h"
#include <iostream>

// Button Layout Constants
// Adjusted y positions to match the "bottom bar" look
static const int BTN_Y = 720;
static const int BTN_W = 120;
static const int BTN_H = 40;
static const int GAP = 20;

// Calculate X positions to center them nicely
static const SDL_Rect btnSetup = { 50, BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnPlay  = { 50 + BTN_W + GAP, BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnPause = { 50 + 2*(BTN_W + GAP), BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnTd    = { 50 + 3*(BTN_W + GAP), BTN_Y, BTN_W, BTN_H };

VideoDisplay::VideoDisplay() {
    window = nullptr;
    renderer = nullptr;
    texture = nullptr;
    font = nullptr;
    videoW = videoH = 0;
}

VideoDisplay::~VideoDisplay() {
    if (texture) SDL_DestroyTexture(texture);
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

bool VideoDisplay::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    if (TTF_Init() == -1) return false;

    // Load font - ensure arial.ttf is in the build folder
    font = TTF_OpenFont("arial.ttf", 18); // Slightly smaller, cleaner font
    if (!font) std::cerr << "Warning: Failed to load arial.ttf\n";

    window = SDL_CreateWindow("RTSP Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1380, 820, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    return (window && renderer);
}

void VideoDisplay::renderFrame(const std::vector<uint8_t>& rgb, int w, int h) {
    if (!texture || w != videoW || h != videoH) {
        if (texture) SDL_DestroyTexture(texture);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, w, h);
        videoW = w;
        videoH = h;
    }
    SDL_UpdateTexture(texture, NULL, rgb.data(), w * 3);

    // Draw Video in a specific area (leaving space at bottom for UI)
    SDL_Rect dst = { (800 - w) / 2, 50, w, h };
    
    // Draw a black border around video
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect border = { dst.x - 2, dst.y - 2, dst.w + 4, dst.h + 4 };
    SDL_RenderFillRect(renderer, &border);

    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

// Helper: Check if mouse is inside rect
bool isMouseOver(SDL_Rect r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h);
}

void VideoDisplay::drawButton(SDL_Rect rect, std::string text, int mx, int my, bool mouseDown, bool isEnabled) {
    bool hovered = isMouseOver(rect, mx, my);
    bool active = hovered && mouseDown;

    // 1. Pick Background Color based on state
    if (!isEnabled) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Disabled (Gray)
    } else if (active) {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); // Active/Clicked (Darker)
    } else if (hovered) {
        SDL_SetRenderDrawColor(renderer, 220, 230, 255, 255); // Hover (Slight Blue tint)
    } else {
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255); // Normal (White-ish)
    }
    SDL_RenderFillRect(renderer, &rect);

    // 2. Draw Border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Dark Gray Border
    SDL_RenderDrawRect(renderer, &rect);

    // 3. Render Text
    if (font) {
        SDL_Color textColor = isEnabled ? SDL_Color{ 0, 0, 0, 255 } : SDL_Color{ 128, 128, 128, 255 };
        SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), textColor);
        if (surf) {
            SDL_Texture* label = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect textRect = { 
                rect.x + (rect.w - surf->w) / 2, 
                rect.y + (rect.h - surf->h) / 2, 
                surf->w, surf->h 
            };
            SDL_RenderCopy(renderer, label, NULL, &textRect);
            SDL_DestroyTexture(label);
            SDL_FreeSurface(surf);
        }
    }
}

void VideoDisplay::renderUI(int clientState, int seconds, int mx, int my, bool mouseDown) {
    // 0=INIT, 1=READY, 2=PLAYING

    // 1. Draw Bottom Panel Background
    SDL_Rect bottomPanel = { 0, 700, 800, 100 };
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); // Light Gray Panel
    SDL_RenderFillRect(renderer, &bottomPanel);
    
    // Draw a separator line
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderDrawLine(renderer, 0, 600, 800, 600);

    // 2. Determine which buttons are enabled based on RTSP State
    bool canSetup = (clientState == 0);          // Only if INIT
    bool canPlay  = (clientState == 1);          // Only if READY
    bool canPause = (clientState == 2);          // Only if PLAYING
    bool canTd    = (clientState != 0);          // If READY or PLAYING

    // 3. Draw Buttons with Logic
    drawButton(btnSetup, "Setup", mx, my, mouseDown, canSetup);
    drawButton(btnPlay,  "Play",  mx, my, mouseDown, canPlay);
    drawButton(btnPause, "Pause", mx, my, mouseDown, canPause);
    drawButton(btnTd,    "Teardown", mx, my, mouseDown, canTd);

    // Optional: Draw Time label
    if (font) {
        SDL_Color textColor = { 0, 0, 0, 255 };
        SDL_Surface* surf = TTF_RenderText_Blended(font, std::to_string(seconds).c_str(), textColor);
        if (surf) {
            SDL_Texture* label = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_Rect textRect = { 600, BTN_Y + (BTN_H - surf->h) / 2, surf->w, surf->h };
            SDL_RenderCopy(renderer, label, NULL, &textRect);
            SDL_DestroyTexture(label);
            SDL_FreeSurface(surf);
        }
    }

    SDL_RenderPresent(renderer);
}