#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <windows.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define EDGE_MARGIN 5  // The detection range for resizing

int fullscreen = 0;  // Track fullscreen state

void ToggleFullscreen(SDL_Window* window) {
    HWND taskbar = FindWindow("Shell_TrayWnd", NULL); // Get taskbar handle
    fullscreen = !fullscreen;

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        ShowWindow(taskbar, SW_HIDE); // Hide taskbar
    } else {
        SDL_SetWindowFullscreen(window, 0);
        ShowWindow(taskbar, SW_SHOW); // Show taskbar
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Resizable SDL2 Window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Cursor types
    SDL_Cursor* resize_cursor_hor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);  // ↔ Horizontal resize
    SDL_Cursor* resize_cursor_ver = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);  // ↕ Vertical resize
    SDL_Cursor* default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);      // Normal arrow

    int width = WINDOW_WIDTH, height = WINDOW_HEIGHT;
    int left_panel_width = 200;
    int bottom_panel_height = 150;
    int top_panel_height = 50;  // Fixed top panel height
    int dragging_left = 0;
    int dragging_bottom = 0;

    // Main loop
    int running = 1;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                ShowWindow(FindWindow("Shell_TrayWnd", NULL), SW_SHOW);
                running = 0;
            } 
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F11) {
                    ToggleFullscreen(window);
                }
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    width = event.window.data1;
                    height = event.window.data2;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.x >= left_panel_width - EDGE_MARGIN && event.button.x <= left_panel_width + EDGE_MARGIN) {
                    dragging_left = 1;
                }
                else if (event.button.y >= height - bottom_panel_height - EDGE_MARGIN && event.button.y <= height - bottom_panel_height + EDGE_MARGIN &&
                         event.button.x >= left_panel_width) {
                    dragging_bottom = 1;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP) {
                dragging_left = 0;
                dragging_bottom = 0;
            }
            else if (event.type == SDL_MOUSEMOTION) {
                int mouse_x = event.motion.x;
                int mouse_y = event.motion.y;

                // Detect if the mouse is near the edges
                if (mouse_x >= left_panel_width - EDGE_MARGIN && mouse_x <= left_panel_width + EDGE_MARGIN) {
                    SDL_SetCursor(resize_cursor_hor);  // ↔ for left panel resize
                }
                else if (mouse_y >= height - bottom_panel_height - EDGE_MARGIN && mouse_y <= height - bottom_panel_height + EDGE_MARGIN &&
                         mouse_x >= left_panel_width) {
                    SDL_SetCursor(resize_cursor_ver);  // ↕ for bottom panel resize
                } 
                else {
                    SDL_SetCursor(default_cursor);
                }

                // Resize panels
                if (dragging_left) {
                    if (mouse_x > 50 && mouse_x < width - 50) {
                        left_panel_width = mouse_x;
                    }
                }

                if (dragging_bottom) {
                    if (mouse_y < height - 50 && mouse_y > top_panel_height + 50) {
                        bottom_panel_height = height - mouse_y;
                    }
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 32, 42, 255);
        SDL_RenderClear(renderer);

        // Draw main background
        SDL_Rect background = {left_panel_width, top_panel_height, width - left_panel_width, height - bottom_panel_height - top_panel_height};
        SDL_SetRenderDrawColor(renderer, 0, 32, 42, 255);
        SDL_RenderFillRect(renderer, &background);

        // Draw bottom panel (dark gray)
        SDL_Rect bottom_panel = {left_panel_width, height - bottom_panel_height, width - left_panel_width, bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &bottom_panel);

        // Draw left panel (light gray)
        SDL_Rect left_panel = {0, 0, left_panel_width, height};
        SDL_SetRenderDrawColor(renderer,  0, 32, 52, 255);
        SDL_RenderFillRect(renderer, &left_panel);

        // Draw top panel (dark gray)
        SDL_Rect top_panel = {0, 0, width, top_panel_height - 23};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &top_panel);

        // Draw separator lines
        int line_thickness = 5;

        // Bottom panel separator (Original separator)
        SDL_Rect bottom_line = {left_panel_width, height - bottom_panel_height, width - left_panel_width, line_thickness};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White color
        SDL_RenderFillRect(renderer, &bottom_line);

        // Top panel separator (white line at the bottom of the top panel)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, top_panel_height - 23, width, top_panel_height -23);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ShowWindow(FindWindow("Shell_TrayWnd", NULL), SW_SHOW);
    SDL_FreeCursor(resize_cursor_hor);
    SDL_FreeCursor(resize_cursor_ver);
    SDL_FreeCursor(default_cursor);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
