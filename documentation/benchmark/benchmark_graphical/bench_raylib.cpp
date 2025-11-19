#include <raylib.h>
#include <chrono>
#include <iostream>
#include <cstring>

struct Button {
    Rectangle bounds;
    char text[32];
    bool isHovered;
    bool isPressed;
    
    Button() : isHovered(false), isPressed(false) {
        bounds = {0, 0, 0, 0};
        strcpy(text, "");
    }
    
    Button(const char* txt, float x, float y, float w, float h) 
        : isHovered(false), isPressed(false) {
        bounds = {x, y, w, h};
        strcpy(text, txt);
    }
};

void UpdateButton(Button& btn, Vector2 mousePos, bool pressed) {
    btn.isHovered = CheckCollisionPointRec(mousePos, btn.bounds);
    btn.isPressed = btn.isHovered && pressed;
}

void DrawButton(const Button& btn) {
    Color col = LIGHTGRAY;
    Color txtCol = DARKGRAY;
    if (btn.isPressed) {
        col = GRAY;
        txtCol = WHITE;
    } else if (btn.isHovered) {
        col = WHITE;
        txtCol = BLACK;
    }
    DrawRectangleRec(btn.bounds, col);
    DrawRectangleLinesEx(btn.bounds, 2, DARKGRAY);
    int w = MeasureText(btn.text, 24);
    float tx = btn.bounds.x + (btn.bounds.width - w) / 2;
    float ty = btn.bounds.y + (btn.bounds.height - 24) / 2;
    DrawText(btn.text, (int)tx, (int)ty, 24, txtCol);
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    InitWindow(800, 600, "Raylib R-Type Menu Demo");
    SetTargetFPS(60);
    
    Button playButton("PLAY", 300, 250, 200, 60);
    Button settingsButton("SETTINGS", 300, 330, 200, 60);
    Button exitButton("EXIT", 300, 410, 200, 60);
    
    auto initTime = std::chrono::high_resolution_clock::now();
    auto initDuration = std::chrono::duration_cast<std::chrono::milliseconds>(initTime - start);
    
    std::cout << "Raylib UI Demo initialized" << std::endl;
    
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        bool mousePressed = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        
        UpdateButton(playButton, mousePos, mousePressed);
        UpdateButton(settingsButton, mousePos, mousePressed);
        UpdateButton(exitButton, mousePos, mousePressed);
        
        BeginDrawing();
        ClearBackground({20, 20, 40, 255});
        
        const char* title = "R-TYPE";
        int titleWidth = MeasureText(title, 72);
        DrawText(title, (800 - titleWidth) / 2, 100, 72, RED);
        
        DrawButton(playButton);
        DrawButton(settingsButton);
        DrawButton(exitButton);
        
        EndDrawing();
    }
    
    CloseWindow();
    
    std::cout << "\n=== Raylib UI Demo Results ===" << std::endl;
    std::cout << "Initialization time: " << initDuration.count() << " ms" << std::endl;
    std::cout << "Code complexity: MEDIUM" << std::endl;
    std::cout << "Lines of code: ~100" << std::endl;
    std::cout << "Features: Struct-based buttons, collision detection" << std::endl;
    
    return 0;
}
