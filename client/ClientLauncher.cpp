#include "Client.h"
#include "VideoDisplay.h"
#include <iostream>

// Re-define constants to match VideoDisplay for hit testing
static const int BTN_Y = 620;
static const int BTN_W = 120;
static const int BTN_H = 40;
static const int GAP = 20;

static const SDL_Rect btnSetup = { 50, BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnPlay  = { 50 + BTN_W + GAP, BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnPause = { 50 + 2*(BTN_W + GAP), BTN_Y, BTN_W, BTN_H };
static const SDL_Rect btnTd    = { 50 + 3*(BTN_W + GAP), BTN_Y, BTN_W, BTN_H };

bool isClick(int mx, int my, SDL_Rect r) {
    return (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h);
}

int main(int argc, char* argv[]) {
    Client client("127.0.0.1", 8554, 25000, "movie.mjpeg");
    VideoDisplay display;
    
    if (!display.init()) return -1;

    bool quit = false;
    std::vector<uint8_t> rgb; 
    int w = 0, h = 0;

    // Mouse State Variables
    int mx = 0, my = 0;
    bool mouseDown = false;

    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;

            // Track Mouse Motion
            if (e.type == SDL_MOUSEMOTION) {
                mx = e.motion.x;
                my = e.motion.y;
            }
            // Track Mouse Down
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    mouseDown = true;
                    // Logic for actions (On Click)
                    if (isClick(mx, my, btnSetup)) client.setup();
                    else if (isClick(mx, my, btnPlay)) client.play();
                    else if (isClick(mx, my, btnPause)) client.pause();
                    else if (isClick(mx, my, btnTd)) client.teardown();
                }
            }
            // Track Mouse Up
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    mouseDown = false;
                }
            }
        }

        // 1. Draw Main Window Background (Light Gray)
        SDL_SetRenderDrawColor(display.getRenderer(), 230, 230, 230, 255); 
        SDL_RenderClear(display.getRenderer());

        // 2. Render Video (if available)
        if (client.getLatestFrame(rgb, w, h)) {
            display.renderFrame(rgb, w, h);
        } else if (!rgb.empty()) {
            display.renderFrame(rgb, w, h);
        }

        // 3. Render UI (Pass Mouse State!)
        display.renderUI(
            (int)client.getState(), 
            client.getPlaySeconds(),
            mx, my, mouseDown
        );

        SDL_Delay(10);
    }

    client.teardown();
    return 0;
}