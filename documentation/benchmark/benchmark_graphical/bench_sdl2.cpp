#include <SDL2/SDL.h>
#ifdef SDL_TTF_AVAILABLE
#include <SDL2/SDL_ttf.h>
#endif
#include <chrono>
#include <iostream>
#include <string>

struct Button {
    SDL_Rect bounds;
    std::string text;
    bool isHovered; // Create surface for button

    bool isPressed;

    Button(const std::string& txt, int x, int y, int w, int h)
        : text(txt), bounds{x, y, w, h}, isHovered(false), isPressed(false) {
    }
};

void UpdateButton(Button& button, int mouseX, int mouseY, bool mousePressed) {
    button.isHovered = (mouseX >= button.bounds.x && mouseX <= button.bounds.x + button.bounds.w &&
                        mouseY >= button.bounds.y && mouseY <= button.bounds.y + button.bounds.h);
    button.isPressed = button.isHovered && mousePressed;
}

void DrawButton(SDL_Renderer* renderer, void* font, const Button& button) {
    if (button.isPressed) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    } else if (button.isHovered) {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    }
    SDL_RenderFillRect(renderer, &button.bounds);

    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    SDL_RenderDrawRect(renderer, &button.bounds);

#ifdef SDL_TTF_AVAILABLE
    if (font) {
        TTF_Font* ttfFont = (TTF_Font*)font;
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(ttfFont, button.text.c_str(), textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                int textW = textSurface->w;
                int textH = textSurface->h;

                SDL_Rect textRect = {button.bounds.x + (button.bounds.w - textW) / 2,
                                     button.bounds.y + (button.bounds.h - textH) / 2, textW, textH};

                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
#else
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect textArea = {button.bounds.x + 10, button.bounds.y + 20, button.bounds.w - 20, 20};
    SDL_RenderFillRect(renderer, &textArea);
#endif
}

void DrawText(SDL_Renderer* renderer, void* font, const std::string& text, int x, int y, SDL_Color color) {
#ifdef SDL_TTF_AVAILABLE
    if (!font)
        return;

    TTF_Font* ttfFont = (TTF_Font*)font;
    SDL_Surface* textSurface = TTF_RenderText_Solid(ttfFont, text.c_str(), color);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
#else
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect textRect = {x, y, (int)text.length() * 10, 20};
    SDL_RenderFillRect(renderer, &textRect);
#endif
}

bool IsButtonClicked(const Button& button, int mouseX, int mouseY, bool mouseClicked) {
    return button.isHovered && mouseClicked;
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Initializing SDL2 UI Demo..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

#ifdef SDL_TTF_AVAILABLE
    if (TTF_Init() != 0) {
        std::cout << "TTF_Init Warning: " << TTF_GetError() << std::endl;
        std::cout << "Continuing without font support..." << std::endl;
    }
#else
    std::cout << "SDL_ttf not available - using simple graphics only" << std::endl;
#endif

    SDL_Window* window = SDL_CreateWindow("SDL2 R-Type Menu Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
                                          600, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
#ifdef SDL_TTF_AVAILABLE
        TTF_Quit();
#endif
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
#ifdef SDL_TTF_AVAILABLE
        TTF_Quit();
#endif
        SDL_Quit();
        return 1;
    }

#ifdef SDL_TTF_AVAILABLE
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    TTF_Font* titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 72);

    if (!font) {
        std::cout << "Warning: Could not load font: " << TTF_GetError() << std::endl;
    }
#else
    void* font = nullptr;
    void* titleFont = nullptr;
    std::cout << "TTF not available - using simple rectangles for text" << std::endl;
#endif

    Button playButton("PLAY", 300, 250, 200, 60);
    Button settingsButton("SETTINGS", 300, 330, 200, 60);
    Button exitButton("EXIT", 300, 410, 200, 60);

    bool running = true;
    bool mousePressed = false;
    bool mouseClicked = false;
    int mouseX = 0, mouseY = 0;

    auto initTime = std::chrono::high_resolution_clock::now();
    auto initDuration = std::chrono::duration_cast<std::chrono::milliseconds>(initTime - start);

    while (running) {
        SDL_Event event;
        mouseClicked = false;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousePressed = true;
                    mouseClicked = true;
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousePressed = false;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }
        }

        UpdateButton(playButton, mouseX, mouseY, mousePressed);
        UpdateButton(settingsButton, mouseX, mouseY, mousePressed);
        UpdateButton(exitButton, mouseX, mouseY, mousePressed);

        if (IsButtonClicked(playButton, mouseX, mouseY, mouseClicked)) {
            std::cout << "Play button clicked!" << std::endl;
        }
        if (IsButtonClicked(settingsButton, mouseX, mouseY, mouseClicked)) {
            std::cout << "Settings button clicked!" << std::endl;
        }
        if (IsButtonClicked(exitButton, mouseX, mouseY, mouseClicked)) {
            std::cout << "Exit button clicked!" << std::endl;
            running = false;
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
        SDL_RenderClear(renderer);

        if (titleFont) {
            SDL_Color titleColor = {255, 0, 0, 255};
            DrawText(renderer, titleFont, "R-TYPE", 280, 100, titleColor);
        }

        DrawButton(renderer, font, playButton);
        DrawButton(renderer, font, settingsButton);
        DrawButton(renderer, font, exitButton);

        SDL_RenderPresent(renderer);
    }

#ifdef SDL_TTF_AVAILABLE
    if (font)
        TTF_CloseFont((TTF_Font*)font);
    if (titleFont)
        TTF_CloseFont((TTF_Font*)titleFont);
    TTF_Quit();
#endif
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "\n=== SDL2 UI Demo Results ===" << std::endl;
    std::cout << "Initialization time: " << initDuration.count() << " ms" << std::endl;
    std::cout << "Code complexity: HIGH" << std::endl;
    std::cout << "Lines of code: ~160" << std::endl;
    std::cout << "Features: Manual everything, requires SDL_ttf for text" << std::endl;

    return 0;
}
