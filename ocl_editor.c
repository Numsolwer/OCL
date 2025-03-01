#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define EDGE_MARGIN 5  // The detection range for resizing

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
    int dragging_left = 0;
    int dragging_bottom = 0;

    // Main loop
    int running = 1;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
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
                    if (mouse_y < height - 50 && mouse_y > 50) {
                        bottom_panel_height = height - mouse_y;
                    }
                }
            }
        }

        // Set background color (navy blue), adjusting size
        SDL_SetRenderDrawColor(renderer, 0, 32, 42, 255);
        SDL_Rect background = {left_panel_width, 0, width - left_panel_width, height - bottom_panel_height};
        SDL_RenderFillRect(renderer, &background);

        // Draw bottom panel (dark gray)
        SDL_Rect bottom_panel = {left_panel_width, height - bottom_panel_height, width - left_panel_width, bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &bottom_panel);

        // Draw left panel (light gray)
        SDL_Rect left_panel = {0, 0, left_panel_width, height};
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &left_panel);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_FreeCursor(resize_cursor_hor);
    SDL_FreeCursor(resize_cursor_ver);
    SDL_FreeCursor(default_cursor);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
