#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <process.h>
#include <math.h> // For smoother rounded corners

// Panel sizing constants
#define MIN_PANEL_WIDTH 200
#define MIN_PANEL_HEIGHT 150
#define EDGE_MARGIN 10
#define MAX_TEXT_LENGTH 10000
#define TEMP_FILE_NAME "temp_code.ocl"
#define SCROLL_SPEED 20
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 30
#define ANIMATION_FRAMES 240 // 4 seconds at 60 FPS
#define CORNER_RADIUS 8 // Reduced for better button fit

typedef struct {
    SDL_Color bg_dark;
    SDL_Color bg_light;
    SDL_Color accent;
    SDL_Color text;
    SDL_Color keyword;
    SDL_Color string;
    SDL_Color comment;
    SDL_Color number;
    SDL_Color hover;
    SDL_Color selection;
    SDL_Color button_border;
} Theme;

typedef struct {
    char text[32];
    int enabled;
    SDL_Rect rect;
    void (*action)(void*, char*);
} MenuItem;

typedef struct {
    SDL_Rect rect;
    char title[16];
    MenuItem items[10];
    int item_count;
    int open;
} DropdownMenu;

typedef struct {
    char text[MAX_TEXT_LENGTH];
    int cursor_pos;
    int selection_start;
    int scroll_y;
    int line_height;
    TTF_Font* font;
    char filename[256];
    int modified;
    int debug_mode;
    int total_lines;
} EditorState;

typedef struct {
    char text[MAX_TEXT_LENGTH];
    int cursor_pos;
} EditHistory;

int fullscreen = 0;
Theme theme;
void* editor_ptr;
SDL_Window* global_window = NULL;
EditHistory history[100];
int history_pos = 0, history_count = 0;

void ToggleFullscreen(SDL_Window* window);
void InitTheme();
void InitMenus(DropdownMenu* file_menu, DropdownMenu* edit_menu, DropdownMenu* view_menu);
void InitEditorState(EditorState* editor, TTF_Font* font);
void PushHistory(EditorState* editor);
void InsertText(EditorState* editor, const char* text);
void DeleteText(EditorState* editor);
char* ExecuteCode(EditorState* editor, int debug, char* console_output);
void EditorOpenFile(void* data, char* console_output);
void SaveFile(void* data, char* console_output);
void SaveAsFile(void* data, char* console_output);
void NewFile(void* data, char* console_output);
void ExitEditor(void* data, char* console_output);
void Cut(void* data, char* console_output);
void Copy(void* data, char* console_output);
void Paste(void* data, char* console_output);
void SelectAll(void* data, char* console_output);
void Undo(EditorState* editor, char* console_output);
void Redo(EditorState* editor, char* console_output);
void ToggleFullscreenAction(void* data, char* console_output);
void RunCode(void* data, char* console_output);
void DebugCode(void* data, char* console_output);
void RenderText(SDL_Renderer* renderer, EditorState* editor, int x, int y, int width, int height, int menu_bar_height);
void RenderRoundedRect(SDL_Renderer* renderer, SDL_Rect* rect, int radius, SDL_Color color, Uint8 alpha);
void RenderDropdownMenu(SDL_Renderer* renderer, DropdownMenu* menu, TTF_Font* font, int mouse_x, int mouse_y);
int CountLines(const char* text);
void RenderAnimation(SDL_Renderer* renderer, TTF_Font* font);
void RenderEditorFadeIn(SDL_Renderer* renderer, TTF_Font* font, EditorState* editor, 
                        DropdownMenu* file_menu, DropdownMenu* edit_menu, DropdownMenu* view_menu,
                        int window_width, int window_height, int left_panel_width, int bottom_panel_height,
                        int menu_bar_height, char* console_output);

void ToggleFullscreen(SDL_Window* window) {
    HWND taskbar = FindWindow("Shell_TrayWnd", NULL);
    fullscreen = !fullscreen;
    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        ShowWindow(taskbar, SW_HIDE);
    } else {
        SDL_SetWindowFullscreen(window, 0);
        ShowWindow(taskbar, SW_SHOW);
    }
}

void InitTheme() {
    theme.bg_dark = (SDL_Color){30, 30, 30, 255};
    theme.bg_light = (SDL_Color){60, 60, 60, 255};
    theme.accent = (SDL_Color){70, 130, 180, 255};
    theme.text = (SDL_Color){240, 240, 240, 255};
    theme.keyword = (SDL_Color){135, 206, 235, 255};
    theme.string = (SDL_Color){144, 238, 144, 255};
    theme.comment = (SDL_Color){150, 150, 150, 255};
    theme.number = (SDL_Color){255, 165, 0, 255};
    theme.hover = (SDL_Color){100, 149, 237, 255};
    theme.selection = (SDL_Color){65, 105, 225, 100};
    theme.button_border = (SDL_Color){40, 40, 40, 255};
}

void InitMenus(DropdownMenu* file_menu, DropdownMenu* edit_menu, DropdownMenu* view_menu) {
    strcpy(file_menu->title, "File");
    file_menu->rect = (SDL_Rect){20, 10, 60, 30};
    file_menu->open = 0;
    file_menu->item_count = 5;
    strcpy(file_menu->items[0].text, "New"); file_menu->items[0].enabled = 1; file_menu->items[0].action = NewFile;
    strcpy(file_menu->items[1].text, "Open..."); file_menu->items[1].enabled = 1; file_menu->items[1].action = EditorOpenFile;
    strcpy(file_menu->items[2].text, "Save"); file_menu->items[2].enabled = 1; file_menu->items[2].action = SaveFile;
    strcpy(file_menu->items[3].text, "Save As..."); file_menu->items[3].enabled = 1; file_menu->items[3].action = SaveAsFile;
    strcpy(file_menu->items[4].text, "Exit"); file_menu->items[4].enabled = 1; file_menu->items[4].action = ExitEditor;

    strcpy(edit_menu->title, "Edit");
    edit_menu->rect = (SDL_Rect){90, 10, 60, 30};
    edit_menu->open = 0;
    edit_menu->item_count = 4;
    strcpy(edit_menu->items[0].text, "Cut"); edit_menu->items[0].enabled = 1; edit_menu->items[0].action = Cut;
    strcpy(edit_menu->items[1].text, "Copy"); edit_menu->items[1].enabled = 1; edit_menu->items[1].action = Copy;
    strcpy(edit_menu->items[2].text, "Paste"); edit_menu->items[2].enabled = 1; edit_menu->items[2].action = Paste;
    strcpy(edit_menu->items[3].text, "Select All"); edit_menu->items[3].enabled = 1; edit_menu->items[3].action = SelectAll;

    strcpy(view_menu->title, "View");
    view_menu->rect = (SDL_Rect){160, 10, 60, 30};
    view_menu->open = 0;
    view_menu->item_count = 1;
    strcpy(view_menu->items[0].text, "Fullscreen"); view_menu->items[0].enabled = 1; view_menu->items[0].action = ToggleFullscreenAction;
}

void InitEditorState(EditorState* editor, TTF_Font* font) {
    strcpy(editor->text, "# OCL Editor - Enhanced UI\n# Use Run/Debug Buttons\n\nlet x = 10;\nprint \"Hello, OCL! x = {x}\";\n");
    editor->cursor_pos = 0;
    editor->selection_start = -1;
    editor->scroll_y = 0;
    editor->line_height = TTF_FontHeight(font);
    editor->font = font;
    strcpy(editor->filename, "untitled.ocl");
    editor->modified = 0;
    editor->debug_mode = 0;
    editor->total_lines = CountLines(editor->text);
}

void PushHistory(EditorState* editor) {
    if (history_count < 100) {
        strcpy(history[history_count].text, editor->text);
        history[history_count].cursor_pos = editor->cursor_pos;
        history_count++;
        history_pos = history_count;
    }
}

void InsertText(EditorState* editor, const char* text) {
    PushHistory(editor);
    int text_len = strlen(text);
    int current_len = strlen(editor->text);
    if (current_len + text_len >= MAX_TEXT_LENGTH - 1) return;
    memmove(editor->text + editor->cursor_pos + text_len, editor->text + editor->cursor_pos, current_len - editor->cursor_pos + 1);
    memcpy(editor->text + editor->cursor_pos, text, text_len);
    editor->cursor_pos += text_len;
    editor->modified = 1;
    editor->total_lines = CountLines(editor->text);
}

void DeleteText(EditorState* editor) {
    PushHistory(editor);
    int len = strlen(editor->text);
    if (editor->cursor_pos > 0 && len > 0) {
        memmove(editor->text + editor->cursor_pos - 1, editor->text + editor->cursor_pos, len - editor->cursor_pos + 1);
        editor->cursor_pos--;
        editor->modified = 1;
        editor->total_lines = CountLines(editor->text);
    }
}

int CountLines(const char* text) {
    int lines = 1;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') lines++;
    }
    return lines;
}

char* ExecuteCode(EditorState* editor, int debug, char* console_output) {
    static char output[1024] = "";
    FILE* file = fopen(TEMP_FILE_NAME, "w");
    if (!file) {
        sprintf(output, "> Error: Cannot create temp file\n");
        strncpy(console_output, output, 1024 - 1);
        console_output[1023] = '\0';
        return output;
    }
    fprintf(file, "%s", editor->text);
    fclose(file);

    char cmd[512];
    if (debug) {
        sprintf(cmd, "C:\\Python39\\python.exe C:\\Users\\nayle\\Documents\\Projects\\OCL\\main.py --debug %s > temp_output.txt 2>&1", TEMP_FILE_NAME);
    } else {
        sprintf(cmd, "C:\\Python39\\python.exe C:\\Users\\nayle\\Documents\\Projects\\OCL\\main.py %s > temp_output.txt 2>&1", TEMP_FILE_NAME);
    }

    printf("Executing command: %s\n", cmd);
    int result = system(cmd);
    if (result != 0) {
        sprintf(output, "> Error: Command failed with code %d\n", result);
    }

    FILE* output_file = fopen("temp_output.txt", "r");
    if (!output_file) {
        sprintf(output, "> Error: Failed to read output\n");
        strncpy(console_output, output, 1024 - 1);
        console_output[1023] = '\0';
        return output;
    }

    output[0] = '\0';
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), output_file) != NULL) {
        strncat(output, buffer, sizeof(output) - strlen(output) - 1);
    }
    fclose(output_file);

    strncpy(console_output, output, 1024 - 1);
    console_output[1023] = '\0';
    return output;
}

void SaveFile(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    FILE* file = fopen(editor->filename, "w");
    if (!file) {
        SaveAsFile(editor, console_output);
        return;
    }
    fprintf(file, "%s", editor->text);
    fclose(file);
    editor->modified = 0;
}

void SaveAsFile(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    char filename[MAX_PATH] = "";
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "OCL Files (*.ocl)\0*.ocl\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "ocl";
    if (GetSaveFileName(&ofn)) {
        strcpy(editor->filename, filename);
        SaveFile(editor, console_output);
    }
}

void EditorOpenFile(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    char filename[MAX_PATH] = "";
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "OCL Files (*.ocl)\0*.ocl\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "ocl";
    if (GetOpenFileName(&ofn)) {
        FILE* file = fopen(filename, "r");
        if (file) {
            editor->text[0] = '\0';
            editor->cursor_pos = 0;
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file)) {
                InsertText(editor, buffer);
            }
            editor->cursor_pos = 0;
            editor->scroll_y = 0;
            editor->modified = 0;
            strcpy(editor->filename, filename);
            fclose(file);
        }
    }
}

void NewFile(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    if (editor->modified) {
        int answer = MessageBox(NULL, "Save changes?", "New File", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (answer == IDYES) SaveFile(editor, console_output);
        else if (answer == IDCANCEL) return;
    }
    strcpy(editor->text, "# New OCL File\n\nlet x = 10;\nprint \"Hello, OCL! x = {x}\";\n");
    editor->cursor_pos = 0;
    editor->scroll_y = 0;
    editor->modified = 0;
    strcpy(editor->filename, "untitled.ocl");
    editor->total_lines = CountLines(editor->text);
}

void ExitEditor(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    if (editor->modified) {
        int answer = MessageBox(NULL, "Save changes before exiting?", "Exit", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (answer == IDYES) SaveFile(editor, console_output);
        else if (answer == IDCANCEL) return;
    }
    SDL_Event quit_event = {.type = SDL_QUIT};
    SDL_PushEvent(&quit_event);
}

void Cut(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    if (editor->selection_start >= 0) {
        int start = editor->selection_start < editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
        int end = editor->selection_start > editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
        int len = end - start;
        char* selected = malloc(len + 1);
        if (selected) {
            strncpy(selected, editor->text + start, len);
            selected[len] = '\0';
            SDL_SetClipboardText(selected);
            memmove(editor->text + start, editor->text + end, strlen(editor->text) - end + 1);
            editor->cursor_pos = start;
            editor->selection_start = -1;
            editor->modified = 1;
            editor->total_lines = CountLines(editor->text);
            free(selected);
        }
    }
}

void Copy(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    if (editor->selection_start >= 0) {
        int start = editor->selection_start < editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
        int end = editor->selection_start > editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
        int len = end - start;
        char* selected = malloc(len + 1);
        if (selected) {
            strncpy(selected, editor->text + start, len);
            selected[len] = '\0';
            SDL_SetClipboardText(selected);
            free(selected);
        }
    }
}

void Paste(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    char* clipboard = SDL_GetClipboardText();
    if (clipboard) {
        InsertText(editor, clipboard);
        SDL_free(clipboard);
    }
}

void SelectAll(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    editor->selection_start = 0;
    editor->cursor_pos = strlen(editor->text);
}

void Undo(EditorState* editor, char* console_output) {
    if (history_pos > 0) {
        history_pos--;
        strcpy(editor->text, history[history_pos].text);
        editor->cursor_pos = history[history_pos].cursor_pos;
        editor->modified = 1;
        editor->total_lines = CountLines(editor->text);
    }
}

void Redo(EditorState* editor, char* console_output) {
    if (history_pos < history_count - 1) {
        history_pos++;
        strcpy(editor->text, history[history_pos].text);
        editor->cursor_pos = history[history_pos].cursor_pos;
        editor->modified = 1;
        editor->total_lines = CountLines(editor->text);
    }
}

void ToggleFullscreenAction(void* data, char* console_output) { if (global_window) ToggleFullscreen(global_window); }

void RunCode(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    editor->debug_mode = 0;
    ExecuteCode(editor, 0, console_output);
}

void DebugCode(void* data, char* console_output) {
    EditorState* editor = (EditorState*)data;
    editor->debug_mode = 1;
    ExecuteCode(editor, 1, console_output);
}

void RenderText(SDL_Renderer* renderer, EditorState* editor, int x, int y, int width, int height, int menu_bar_height) {
    char line[1024];
    int line_start = 0, line_num = 0, render_y = y - editor->scroll_y;
    int cursor_line = 0, cursor_col = 0;
    for (int i = 0; i < editor->cursor_pos; i++) {
        if (editor->text[i] == '\n') { cursor_line++; cursor_col = 0; } else { cursor_col++; }
    }
    for (int i = 0; i <= strlen(editor->text); i++) {
        if (editor->text[i] == '\n' || editor->text[i] == '\0') {
            int line_len = i - line_start;
            if (line_len > 1023) line_len = 1023;
            strncpy(line, editor->text + line_start, line_len);
            line[line_len] = '\0';

            if (render_y >= menu_bar_height) {
                SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, 255);
                SDL_Rect line_num_bg = {x - 60, render_y, 50, editor->line_height};
                SDL_RenderFillRect(renderer, &line_num_bg);

                char line_num_str[10];
                sprintf(line_num_str, "%3d ", line_num + 1);
                SDL_Surface* line_num_surface = TTF_RenderText_Solid(editor->font, line_num_str, theme.comment);
                if (line_num_surface) {
                    SDL_Texture* line_num_texture = SDL_CreateTextureFromSurface(renderer, line_num_surface);
                    if (line_num_texture) {
                        SDL_Rect line_num_rect = {x - 55, render_y, line_num_surface->w, line_num_surface->h};
                        SDL_RenderCopy(renderer, line_num_texture, NULL, &line_num_rect);
                        SDL_DestroyTexture(line_num_texture);
                    }
                    SDL_FreeSurface(line_num_surface);
                }

                if (editor->selection_start >= 0) {
                    int sel_start = editor->selection_start < editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
                    int sel_end = editor->selection_start > editor->cursor_pos ? editor->selection_start : editor->cursor_pos;
                    if (line_start <= sel_end && i >= sel_start) {
                        int start_x = x + (sel_start > line_start ? sel_start - line_start : 0) * 10;
                        int end_x = x + (sel_end < i ? sel_end - line_start : i - line_start) * 10;
                        SDL_SetRenderDrawColor(renderer, theme.selection.r, theme.selection.g, theme.selection.b, theme.selection.a);
                        SDL_Rect sel_rect = {start_x, render_y, end_x - start_x, editor->line_height};
                        SDL_RenderFillRect(renderer, &sel_rect);
                    }
                }

                SDL_Color color = theme.text;
                if (strstr(line, "let") || strstr(line, "print") || strstr(line, "if") ||
                    strstr(line, "elif") || strstr(line, "else") || strstr(line, "while") ||
                    strstr(line, "define") || strstr(line, "return") || strstr(line, "class") ||
                    strstr(line, "break") || strstr(line, "continue") || strstr(line, "true") ||
                    strstr(line, "false") || strstr(line, "null") || strstr(line, "int") ||
                    strstr(line, "float") || strstr(line, "bool") || strstr(line, "string") ||
                    strstr(line, "ocl")) color = theme.keyword;
                else if (strstr(line, "#")) color = theme.comment;
                else if (strstr(line, "\"")) color = theme.string;
                else if (strspn(line, "0123456789.") == strlen(line)) color = theme.number;

                SDL_Surface* text_surface = TTF_RenderText_Solid(editor->font, line, color);
                if (text_surface) {
                    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
                    if (text_texture) {
                        SDL_Rect text_rect = {x, render_y, text_surface->w, text_surface->h};
                        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
                        SDL_DestroyTexture(text_texture);
                    }
                    SDL_FreeSurface(text_surface);
                }

                if (line_num == cursor_line) {
                    int cursor_x = x + cursor_col * 10;
                    SDL_SetRenderDrawColor(renderer, theme.accent.r, theme.accent.g, theme.accent.b, 255);
                    SDL_Rect cursor_rect = {cursor_x, render_y, 2, editor->line_height};
                    SDL_RenderFillRect(renderer, &cursor_rect);
                }
            }

            render_y += editor->line_height;
            line_start = i + 1;
            line_num++;
            if (render_y > y + height) break;
        }
    }
}

// Improved function to render smoother rounded rectangles with alpha
void RenderRoundedRect(SDL_Renderer* renderer, SDL_Rect* rect, int radius, SDL_Color color, Uint8 alpha) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);

    // Ensure radius doesn't exceed half the width or height
    if (radius > rect->w / 2) radius = rect->w / 2;
    if (radius > rect->h / 2) radius = rect->h / 2;

    // Draw the straight parts
    SDL_Rect top = {rect->x + radius, rect->y, rect->w - 2 * radius, radius};
    SDL_Rect middle = {rect->x, rect->y + radius, rect->w, rect->h - 2 * radius};
    SDL_Rect bottom = {rect->x + radius, rect->y + rect->h - radius, rect->w - 2 * radius, radius};
    SDL_RenderFillRect(renderer, &top);
    SDL_RenderFillRect(renderer, &middle);
    SDL_RenderFillRect(renderer, &bottom);

    // Draw the rounded corners using a smoother curve
    for (int i = 0; i <= radius; i++) {
        int offset = (int)(radius - sqrt(radius * radius - i * i));
        SDL_RenderDrawLine(renderer, rect->x + offset, rect->y + i, rect->x + rect->w - offset - 1, rect->y + i); // Top
        SDL_RenderDrawLine(renderer, rect->x + offset, rect->y + rect->h - i - 1, rect->x + rect->w - offset - 1, rect->y + rect->h - i - 1); // Bottom
    }
}

void RenderDropdownMenu(SDL_Renderer* renderer, DropdownMenu* menu, TTF_Font* font, int mouse_x, int mouse_y) {
    SDL_Color btn_color = (mouse_x >= 0 && SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &menu->rect)) ? theme.hover : theme.accent;
    RenderRoundedRect(renderer, &menu->rect, CORNER_RADIUS, btn_color, 255);
    SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
    SDL_RenderDrawRect(renderer, &menu->rect);  // Optional: keep border as rectangle

    SDL_Surface* button_text = TTF_RenderText_Solid(font, menu->title, theme.text);
    if (button_text) {
        SDL_Texture* button_texture = SDL_CreateTextureFromSurface(renderer, button_text);
        if (button_texture) {
            SDL_Rect text_rect = {menu->rect.x + (menu->rect.w - button_text->w) / 2, menu->rect.y + 5, button_text->w, button_text->h};
            SDL_RenderCopy(renderer, button_texture, NULL, &text_rect);
            SDL_DestroyTexture(button_texture);
        }
        SDL_FreeSurface(button_text);
    }

    if (menu->open) {
        int dropdown_height = menu->item_count * 35;
        SDL_Rect dropdown_rect = {menu->rect.x, menu->rect.y + menu->rect.h, 150, dropdown_height};
        RenderRoundedRect(renderer, &dropdown_rect, CORNER_RADIUS, theme.bg_light, 255);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &dropdown_rect);  // Optional: keep border as rectangle

        for (int i = 0; i < menu->item_count; i++) {
            menu->items[i].rect = (SDL_Rect){menu->rect.x, menu->rect.y + menu->rect.h + i * 35, 150, 35};
            SDL_Color item_color = (mouse_x >= 0 && SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &menu->items[i].rect)) ? theme.hover : theme.bg_light;
            RenderRoundedRect(renderer, &menu->items[i].rect, CORNER_RADIUS, item_color, 255);
            SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
            SDL_RenderDrawRect(renderer, &menu->items[i].rect);  // Optional: keep border as rectangle

            SDL_Color text_color = menu->items[i].enabled ? theme.text : theme.comment;
            SDL_Surface* item_text = TTF_RenderText_Solid(font, menu->items[i].text, text_color);
            if (item_text) {
                SDL_Texture* item_texture = SDL_CreateTextureFromSurface(renderer, item_text);
                if (item_texture) {
                    SDL_Rect text_rect = {menu->items[i].rect.x + 15, menu->items[i].rect.y + 10, item_text->w, item_text->h};
                    SDL_RenderCopy(renderer, item_texture, NULL, &text_rect);
                    SDL_DestroyTexture(item_texture);
                }
                SDL_FreeSurface(item_text);
            }
        }
    }
}

void RenderAnimation(SDL_Renderer* renderer, TTF_Font* font) {
    TTF_Font* anim_font = TTF_OpenFont("C:\\Windows\\Fonts\\consola.ttf", 46);
    if (!anim_font) anim_font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 46);
    if (!anim_font) {
        printf("Failed to load animation font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color text_color = theme.text;
    int window_width, window_height;
    SDL_GetWindowSize(global_window, &window_width, &window_height);

    int frame = 0;
    const int TOTAL_FRAMES = 240;
    float ocl_x = window_width / 2 - 150;  // Reduced spacing
    float ocl_y = window_height / 2;
    float e_x = window_width;
    float e_y = ocl_y;
    float ditor_alpha = 0.0f;
    float e_target_x = 0;

    SDL_Surface* ocl_surface = NULL;
    SDL_Surface* e_surface = NULL;
    SDL_Surface* ditor_surface = NULL;

    printf("Starting animation\n");
    while (frame < TOTAL_FRAMES) {
        SDL_SetRenderDrawColor(renderer, theme.bg_dark.r, theme.bg_dark.g, theme.bg_dark.b, 255);
        SDL_RenderClear(renderer);

        ocl_surface = TTF_RenderText_Solid(anim_font, "OCL", text_color);
        if (!ocl_surface) {
            printf("Failed to render OCL: %s\n", TTF_GetError());
            break;
        }
        SDL_Texture* ocl_texture = SDL_CreateTextureFromSurface(renderer, ocl_surface);
        if (ocl_texture) {
            SDL_Rect ocl_rect = {(int)ocl_x, (int)ocl_y - ocl_surface->h / 2, ocl_surface->w, ocl_surface->h};
            SDL_RenderCopy(renderer, ocl_texture, NULL, &ocl_rect);
            SDL_DestroyTexture(ocl_texture);
        }
        SDL_FreeSurface(ocl_surface);
        ocl_surface = NULL;

        // Calculate ocl_rect here for use in e_target_x
        SDL_Surface* temp_ocl_surface = TTF_RenderText_Solid(anim_font, "OCL", text_color);
        if (temp_ocl_surface) {
            SDL_Rect temp_ocl_rect = {(int)ocl_x, (int)ocl_y - temp_ocl_surface->h / 2, temp_ocl_surface->w, temp_ocl_surface->h};
            if (frame == TOTAL_FRAMES / 4) {
                e_target_x = ocl_x + temp_ocl_rect.w - 10;  // Reduced spacing between OCL and E
                printf("E target set at frame %d\n", frame);
            }
            SDL_FreeSurface(temp_ocl_surface);
        }

        if (frame >= TOTAL_FRAMES / 4) {
            e_surface = TTF_RenderText_Solid(anim_font, "E", text_color);
            if (!e_surface) {
                printf("Failed to render E: %s\n", TTF_GetError());
                break;
            }
            SDL_Texture* e_texture = SDL_CreateTextureFromSurface(renderer, e_surface);
            if (e_texture) {
                if (e_x > e_target_x) {
                    e_x -= (e_x - e_target_x) * 0.025f;
                    if (e_x < e_target_x) e_x = e_target_x;
                }
                SDL_Rect e_rect = {(int)e_x, (int)e_y - e_surface->h / 2, e_surface->w, e_surface->h};
                SDL_RenderCopy(renderer, e_texture, NULL, &e_rect);
                SDL_DestroyTexture(e_texture);
            }
            SDL_FreeSurface(e_surface);
            e_surface = NULL;

            if (frame < TOTAL_FRAMES / 2 && e_x > e_target_x) {
                ocl_x -= 0.5f;
            }
            else if (frame >= TOTAL_FRAMES / 2 && frame < TOTAL_FRAMES * 3 / 4 && e_x <= e_target_x) {
                float original_ocl_x = window_width / 2 - 150;  // Updated to match new starting position
                if (ocl_x < original_ocl_x) {
                    ocl_x += 0.5f;
                    if (ocl_x > original_ocl_x) ocl_x = original_ocl_x;
                }
            }
        }

        if (frame >= TOTAL_FRAMES * 3 / 4 && e_x <= e_target_x) {
            ditor_surface = TTF_RenderText_Solid(anim_font, "ditor", text_color);
            if (!ditor_surface) {
                printf("Failed to render ditor: %s\n", TTF_GetError());
                break;
            }
            ditor_alpha += 0.025f;  // Increased fade speed for visibility
            if (ditor_alpha > 1.0f) ditor_alpha = 1.0f;
            SDL_Texture* ditor_texture = SDL_CreateTextureFromSurface(renderer, ditor_surface);
            if (ditor_texture) {
                SDL_SetTextureAlphaMod(ditor_texture, (Uint8)(ditor_alpha * 255));
                SDL_Rect ditor_rect = {(int)(e_x + 10), (int)e_y - ditor_surface->h / 2, ditor_surface->w, ditor_surface->h};  // Reduced spacing
                SDL_RenderCopy(renderer, ditor_texture, NULL, &ditor_rect);
                SDL_DestroyTexture(ditor_texture);
                printf("Rendering ditor at frame %d, alpha: %.2f\n", frame, ditor_alpha);
            }
            SDL_FreeSurface(ditor_surface);
            ditor_surface = NULL;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
        frame++;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                frame = TOTAL_FRAMES;
            }
        }
    }

    if (ocl_surface) SDL_FreeSurface(ocl_surface);
    if (e_surface) SDL_FreeSurface(e_surface);
    if (ditor_surface) SDL_FreeSurface(ditor_surface);
    TTF_CloseFont(anim_font);
    printf("Animation completed\n");
}

void RenderEditorFadeIn(SDL_Renderer* renderer, TTF_Font* font, EditorState* editor, 
                        DropdownMenu* file_menu, DropdownMenu* edit_menu, DropdownMenu* view_menu,
                        int window_width, int window_height, int left_panel_width, int bottom_panel_height,
                        int menu_bar_height, char* console_output) {
    const int FADE_FRAMES = 60;
    float alpha = 0.0f;

    printf("Starting editor fade-in\n");
    for (int frame = 0; frame < FADE_FRAMES; frame++) {
        SDL_SetRenderDrawColor(renderer, theme.bg_dark.r, theme.bg_dark.g, theme.bg_dark.b, 255);
        SDL_RenderClear(renderer);

        alpha += 1.0f / FADE_FRAMES;
        if (alpha > 1.0f) alpha = 1.0f;
        Uint8 alpha_value = (Uint8)(alpha * 255);

        SDL_Rect left_panel = {0, menu_bar_height, left_panel_width, window_height - menu_bar_height - bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, alpha_value);
        SDL_RenderFillRect(renderer, &left_panel);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &left_panel);

        SDL_Surface* left_text = TTF_RenderText_Solid(font, "File Explorer", theme.text);
        if (left_text) {
            SDL_Texture* left_texture = SDL_CreateTextureFromSurface(renderer, left_text);
            if (left_texture) {
                SDL_SetTextureAlphaMod(left_texture, alpha_value);
                SDL_Rect left_text_rect = {20, menu_bar_height + 20, left_text->w, left_text->h};
                SDL_RenderCopy(renderer, left_texture, NULL, &left_text_rect);
                SDL_DestroyTexture(left_texture);
            }
            SDL_FreeSurface(left_text);
        }

        SDL_Rect bottom_panel = {0, window_height - bottom_panel_height, window_width, bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, alpha_value);
        SDL_RenderFillRect(renderer, &bottom_panel);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &bottom_panel);

        SDL_Surface* console_surface = TTF_RenderText_Blended_Wrapped(font, console_output, theme.text, bottom_panel.w - 40);
        if (console_surface) {
            SDL_Texture* console_texture = SDL_CreateTextureFromSurface(renderer, console_surface);
            if (console_texture) {
                SDL_SetTextureAlphaMod(console_texture, alpha_value);
                SDL_Rect text_rect = {bottom_panel.x + 20, bottom_panel.y + 20, console_surface->w, console_surface->h};
                SDL_RenderCopy(renderer, console_texture, NULL, &text_rect);
                SDL_DestroyTexture(console_texture);
            }
            SDL_FreeSurface(console_surface);
        }

        SDL_Rect menu_bar = {0, 0, window_width, menu_bar_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, alpha_value);
        SDL_RenderFillRect(renderer, &menu_bar);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &menu_bar);

        SDL_Rect run_button = {window_width - 180, 10, BUTTON_WIDTH, BUTTON_HEIGHT};
        SDL_Color run_color = theme.accent;  // No hover during fade-in
        RenderRoundedRect(renderer, &run_button, CORNER_RADIUS, run_color, alpha_value);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &run_button);

        SDL_Surface* run_text = TTF_RenderText_Solid(font, "Run", theme.text);
        if (run_text) {
            SDL_Texture* run_texture = SDL_CreateTextureFromSurface(renderer, run_text);
            if (run_texture) {
                SDL_SetTextureAlphaMod(run_texture, alpha_value);
                SDL_Rect run_text_rect = {run_button.x + (BUTTON_WIDTH - run_text->w) / 2, run_button.y + 5, run_text->w, run_text->h};
                SDL_RenderCopy(renderer, run_texture, NULL, &run_text_rect);
                SDL_DestroyTexture(run_texture);
            }
            SDL_FreeSurface(run_text);
        }

        SDL_Rect debug_button = {window_width - 90, 10, BUTTON_WIDTH, BUTTON_HEIGHT};
        SDL_Color debug_color = theme.accent;  // No hover during fade-in
        RenderRoundedRect(renderer, &debug_button, CORNER_RADIUS, debug_color, alpha_value);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &debug_button);

        SDL_Surface* debug_text = TTF_RenderText_Solid(font, "Debug", theme.text);
        if (debug_text) {
            SDL_Texture* debug_texture = SDL_CreateTextureFromSurface(renderer, debug_text);
            if (debug_texture) {
                SDL_SetTextureAlphaMod(debug_texture, alpha_value);
                SDL_Rect debug_text_rect = {debug_button.x + (BUTTON_WIDTH - debug_text->w) / 2, debug_button.y + 5, debug_text->w, debug_text->h};
                SDL_RenderCopy(renderer, debug_texture, NULL, &debug_text_rect);
                SDL_DestroyTexture(debug_texture);
            }
            SDL_FreeSurface(debug_text);
        }

        SDL_Rect editor_area = {left_panel_width, menu_bar_height, window_width - left_panel_width, window_height - menu_bar_height - bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_dark.r, theme.bg_dark.g, theme.bg_dark.b, alpha_value);
        SDL_RenderFillRect(renderer, &editor_area);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, alpha_value);
        SDL_RenderDrawRect(renderer, &editor_area);

        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, alpha_value);
        SDL_Rect line_num_bg = {editor_area.x, editor_area.y, 50, editor_area.h};
        SDL_RenderFillRect(renderer, &line_num_bg);

        // Render editor text during fade-in
        RenderText(renderer, editor, left_panel_width + 60, menu_bar_height + 20, editor_area.w - 80, editor_area.h - 40, menu_bar_height);

        RenderDropdownMenu(renderer, file_menu, font, -1, -1);
        RenderDropdownMenu(renderer, edit_menu, font, -1, -1);
        RenderDropdownMenu(renderer, view_menu, font, -1, -1);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                frame = FADE_FRAMES;
            }
        }
        printf("Fade-in frame %d, alpha: %.2f\n", frame, alpha);
    }
    printf("Editor fade-in completed\n");
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        printf("Initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    int window_width = 1000, window_height = 600;  // Reduced height from 800 to 600
    SDL_Window* window = SDL_CreateWindow("OCL Editor - Enhanced OCL Support", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    global_window = window;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("C:\\Windows\\Fonts\\consola.ttf", 14);
    if (!font) font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 14);
    if (!font) {
        printf("Font loading failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    InitTheme();
    printf("Running animation\n");
    RenderAnimation(renderer, font);

    EditorState editor;
    InitEditorState(&editor, font);
    DropdownMenu file_menu, edit_menu, view_menu;
    InitMenus(&file_menu, &edit_menu, &view_menu);

    int left_panel_width = 250, bottom_panel_height = 150, menu_bar_height = 50;  // Adjusted bottom panel height
    char console_output[1024] = "> OCL Editor Enhanced - Ready\n";
    printf("Running editor fade-in\n");
    RenderEditorFadeIn(renderer, font, &editor, &file_menu, &edit_menu, &view_menu,
                       window_width, window_height, left_panel_width, bottom_panel_height,
                       menu_bar_height, console_output);

    SDL_Cursor* resize_cursor_hor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    SDL_Cursor* resize_cursor_ver = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    SDL_Cursor* default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    if (!resize_cursor_hor || !resize_cursor_ver || !default_cursor) {
        printf("Failed to create cursors\n");
    }

    int resizing_left_panel = 0, resizing_bottom_panel = 0;
    int quit = 0;

    SDL_Event event;
    printf("Entering main loop\n");
    while (!quit) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: quit = 1; break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_F11) ToggleFullscreen(window);
                    else if (event.key.keysym.sym == SDLK_BACKSPACE) DeleteText(&editor);
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) InsertText(&editor, "\n");
                    else if (event.key.keysym.sym == SDLK_TAB) InsertText(&editor, "    ");
                    else if (event.key.keysym.sym == SDLK_DELETE) {
                        int len = strlen(editor.text);
                        if (editor.cursor_pos < len) {
                            PushHistory(&editor);
                            memmove(editor.text + editor.cursor_pos, editor.text + editor.cursor_pos + 1, len - editor.cursor_pos);
                            editor.modified = 1;
                            editor.total_lines = CountLines(editor.text);
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & KMOD_CTRL)) Undo(&editor, console_output);
                    else if (event.key.keysym.sym == SDLK_y && (event.key.keysym.mod & KMOD_CTRL)) Redo(&editor, console_output);
                    else if (event.key.keysym.mod & KMOD_SHIFT) {
                        if (editor.selection_start == -1) editor.selection_start = editor.cursor_pos;
                        if (event.key.keysym.sym == SDLK_LEFT && editor.cursor_pos > 0) editor.cursor_pos--;
                        else if (event.key.keysym.sym == SDLK_RIGHT && editor.cursor_pos < strlen(editor.text)) editor.cursor_pos++;
                    } else {
                        editor.selection_start = -1;
                        if (event.key.keysym.sym == SDLK_LEFT && editor.cursor_pos > 0) editor.cursor_pos--;
                        else if (event.key.keysym.sym == SDLK_RIGHT && editor.cursor_pos < strlen(editor.text)) editor.cursor_pos++;
                        else if (event.key.keysym.sym == SDLK_UP) {
                            int current_pos = editor.cursor_pos;
                            int line_start = current_pos;
                            while (line_start > 0 && editor.text[line_start - 1] != '\n') line_start--;
                            int col = current_pos - line_start;
                            if (line_start > 0) {
                                int prev_line_end = line_start - 1;
                                int prev_line_start = prev_line_end;
                                while (prev_line_start > 0 && editor.text[prev_line_start - 1] != '\n') prev_line_start--;
                                int prev_line_len = prev_line_end - prev_line_start;
                                editor.cursor_pos = prev_line_start + (col < prev_line_len ? col : prev_line_len);
                            }
                        }
                        else if (event.key.keysym.sym == SDLK_DOWN) {
                            int current_pos = editor.cursor_pos;
                            int line_start = current_pos;
                            int cursor_line = 0;
                            for (int i = 0; i < current_pos; i++) {
                                if (editor.text[i] == '\n') cursor_line++;
                            }
                            while (line_start > 0 && editor.text[line_start - 1] != '\n') line_start--;
                            int col = current_pos - line_start;
                            int line_end = current_pos;
                            while (line_end < strlen(editor.text) && editor.text[line_end] != '\n') line_end++;
                            if (line_end < strlen(editor.text) && editor.total_lines > cursor_line + 1) {
                                int next_line_start = line_end + 1;
                                int next_line_end = next_line_start;
                                while (next_line_end < strlen(editor.text) && editor.text[next_line_end] != '\n') next_line_end++;
                                int next_line_len = next_line_end - next_line_start;
                                editor.cursor_pos = next_line_start + (col < next_line_len ? col : next_line_len);
                            }
                        }
                        else if (event.key.keysym.sym == SDLK_HOME) {
                            while (editor.cursor_pos > 0 && editor.text[editor.cursor_pos - 1] != '\n') editor.cursor_pos--;
                        }
                        else if (event.key.keysym.sym == SDLK_END) {
                            while (editor.cursor_pos < strlen(editor.text) && editor.text[editor.cursor_pos] != '\n') editor.cursor_pos++;
                        }
                    }
                    break;
                case SDL_TEXTINPUT: InsertText(&editor, event.text.text); break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (abs(mouse_x - left_panel_width) < EDGE_MARGIN && mouse_y > menu_bar_height) resizing_left_panel = 1;
                        else if (mouse_y > window_height - bottom_panel_height - EDGE_MARGIN && mouse_y < window_height - bottom_panel_height + EDGE_MARGIN) resizing_bottom_panel = 1;
                        else if (mouse_y < menu_bar_height) {
                            if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &file_menu.rect)) {
                                file_menu.open = !file_menu.open; edit_menu.open = 0; view_menu.open = 0;
                            } else if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &edit_menu.rect)) {
                                edit_menu.open = !edit_menu.open; file_menu.open = 0; view_menu.open = 0;
                            } else if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &view_menu.rect)) {
                                view_menu.open = !view_menu.open; file_menu.open = 0; edit_menu.open = 0;
                            } else if (mouse_x >= window_width - 180 && mouse_x <= window_width - 100 && mouse_y >= 10 && mouse_y <= 40) {
                                RunCode(&editor, console_output);
                            } else if (mouse_x >= window_width - 90 && mouse_x <= window_width - 10 && mouse_y >= 10 && mouse_y <= 40) {
                                DebugCode(&editor, console_output);
                            }
                        } else if (file_menu.open || edit_menu.open || view_menu.open) {
                            DropdownMenu* active_menu = file_menu.open ? &file_menu : edit_menu.open ? &edit_menu : &view_menu;
                            for (int i = 0; i < active_menu->item_count; i++) {
                                if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &active_menu->items[i].rect) && active_menu->items[i].enabled) {
                                    active_menu->items[i].action(&editor, console_output);
                                    active_menu->open = 0;
                                    break;
                                }
                            }
                            if (!SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &active_menu->rect)) active_menu->open = 0;
                        } else if (mouse_x >= left_panel_width && mouse_x <= window_width &&
                                   mouse_y >= menu_bar_height && mouse_y <= window_height - bottom_panel_height) {
                            int editor_x = left_panel_width + 60;
                            int editor_y = menu_bar_height + 20;
                            int rel_x = mouse_x - editor_x;
                            int rel_y = mouse_y - editor_y + editor.scroll_y;
                            int line = rel_y / editor.line_height;
                            int pos = 0, line_count = 0;
                            for (int i = 0; i < strlen(editor.text) && line_count <= line; i++) {
                                if (editor.text[i] == '\n') line_count++;
                                if (line_count == line) {
                                    pos = i + 1;
                                    break;
                                }
                            }
                            while (pos < strlen(editor.text) && editor.text[pos] != '\n' &&
                                   (pos - (pos - strlen(&editor.text[pos]))) * 10 < rel_x) pos++;
                            editor.cursor_pos = pos;
                            editor.selection_start = -1;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        resizing_left_panel = 0;
                        resizing_bottom_panel = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (resizing_left_panel) {
                        left_panel_width = event.motion.x;
                        if (left_panel_width < MIN_PANEL_WIDTH) left_panel_width = MIN_PANEL_WIDTH;
                        if (left_panel_width > window_width - MIN_PANEL_WIDTH) left_panel_width = window_width - MIN_PANEL_WIDTH;
                    } else if (resizing_bottom_panel) {
                        bottom_panel_height = window_height - event.motion.y;
                        if (bottom_panel_height < MIN_PANEL_HEIGHT) bottom_panel_height = MIN_PANEL_HEIGHT;
                        if (bottom_panel_height > window_height - menu_bar_height - MIN_PANEL_HEIGHT) bottom_panel_height = window_height - menu_bar_height - MIN_PANEL_HEIGHT;
                    } else {
                        if (abs(mouse_x - left_panel_width) < EDGE_MARGIN && mouse_y > menu_bar_height) SDL_SetCursor(resize_cursor_hor);
                        else if (mouse_y > window_height - bottom_panel_height - EDGE_MARGIN && mouse_y < window_height - bottom_panel_height + EDGE_MARGIN) SDL_SetCursor(resize_cursor_ver);
                        else SDL_SetCursor(default_cursor);
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    editor.scroll_y -= event.wheel.y * SCROLL_SPEED;
                    if (editor.scroll_y < 0) editor.scroll_y = 0;
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        window_width = event.window.data1;
                        window_height = event.window.data2;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, theme.bg_dark.r, theme.bg_dark.g, theme.bg_dark.b, 255);
        SDL_RenderClear(renderer);

        SDL_Rect left_panel = {0, menu_bar_height, left_panel_width, window_height - menu_bar_height - bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, 255);
        SDL_RenderFillRect(renderer, &left_panel);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &left_panel);

        SDL_Surface* left_text = TTF_RenderText_Solid(font, "File Explorer", theme.text);
        if (left_text) {
            SDL_Texture* left_texture = SDL_CreateTextureFromSurface(renderer, left_text);
            if (left_texture) {
                SDL_Rect left_text_rect = {20, menu_bar_height + 20, left_text->w, left_text->h};
                SDL_RenderCopy(renderer, left_texture, NULL, &left_text_rect);
                SDL_DestroyTexture(left_texture);
            }
            SDL_FreeSurface(left_text);
        }

        SDL_Rect bottom_panel = {0, window_height - bottom_panel_height, window_width, bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, 255);
        SDL_RenderFillRect(renderer, &bottom_panel);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &bottom_panel);

        SDL_Surface* console_surface = TTF_RenderText_Blended_Wrapped(font, console_output, theme.text, bottom_panel.w - 40);
        if (console_surface) {
            SDL_Texture* console_texture = SDL_CreateTextureFromSurface(renderer, console_surface);
            if (console_texture) {
                SDL_Rect text_rect = {bottom_panel.x + 20, bottom_panel.y + 20, console_surface->w, console_surface->h};
                SDL_RenderCopy(renderer, console_texture, NULL, &text_rect);
                SDL_DestroyTexture(console_texture);
            }
            SDL_FreeSurface(console_surface);
        }

        SDL_Rect menu_bar = {0, 0, window_width, menu_bar_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_light.r, theme.bg_light.g, theme.bg_light.b, 255);
        SDL_RenderFillRect(renderer, &menu_bar);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &menu_bar);

        SDL_Rect run_button = {window_width - 180, 10, BUTTON_WIDTH, BUTTON_HEIGHT};
        SDL_Color run_color = SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &run_button) ? theme.hover : theme.accent;
        RenderRoundedRect(renderer, &run_button, CORNER_RADIUS, run_color, 255);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &run_button);

        SDL_Surface* run_text = TTF_RenderText_Solid(font, "Run", theme.text);
        if (run_text) {
            SDL_Texture* run_texture = SDL_CreateTextureFromSurface(renderer, run_text);
            if (run_texture) {
                SDL_Rect run_text_rect = {run_button.x + (BUTTON_WIDTH - run_text->w) / 2, run_button.y + 5, run_text->w, run_text->h};
                SDL_RenderCopy(renderer, run_texture, NULL, &run_text_rect);
                SDL_DestroyTexture(run_texture);
            }
            SDL_FreeSurface(run_text);
        }

        SDL_Rect debug_button = {window_width - 90, 10, BUTTON_WIDTH, BUTTON_HEIGHT};
        SDL_Color debug_color = SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &debug_button) ? theme.hover : theme.accent;
        RenderRoundedRect(renderer, &debug_button, CORNER_RADIUS, debug_color, 255);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &debug_button);

        SDL_Surface* debug_text = TTF_RenderText_Solid(font, "Debug", theme.text);
        if (debug_text) {
            SDL_Texture* debug_texture = SDL_CreateTextureFromSurface(renderer, debug_text);
            if (debug_texture) {
                SDL_Rect debug_text_rect = {debug_button.x + (BUTTON_WIDTH - debug_text->w) / 2, debug_button.y + 5, debug_text->w, debug_text->h};
                SDL_RenderCopy(renderer, debug_texture, NULL, &debug_text_rect);
                SDL_DestroyTexture(debug_texture);
            }
            SDL_FreeSurface(debug_text);
        }

        SDL_Rect editor_area = {left_panel_width, menu_bar_height, window_width - left_panel_width, window_height - menu_bar_height - bottom_panel_height};
        SDL_SetRenderDrawColor(renderer, theme.bg_dark.r, theme.bg_dark.g, theme.bg_dark.b, 255);
        SDL_RenderFillRect(renderer, &editor_area);
        SDL_SetRenderDrawColor(renderer, theme.button_border.r, theme.button_border.g, theme.button_border.b, 255);
        SDL_RenderDrawRect(renderer, &editor_area);

        RenderText(renderer, &editor, left_panel_width + 60, menu_bar_height + 20, editor_area.w - 80, editor_area.h - 40, menu_bar_height);

        RenderDropdownMenu(renderer, &file_menu, font, mouse_x, mouse_y);
        RenderDropdownMenu(renderer, &edit_menu, font, mouse_x, mouse_y);
        RenderDropdownMenu(renderer, &view_menu, font, mouse_x, mouse_y);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (resize_cursor_hor) SDL_FreeCursor(resize_cursor_hor);
    if (resize_cursor_ver) SDL_FreeCursor(resize_cursor_ver);
    if (default_cursor) SDL_FreeCursor(default_cursor);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    printf("Program exiting\n");
    return 0;
}
