#include "../../include/gui/Menu.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <iostream>
#include <cmath>

#define AI_DEBUG
#ifdef AI_DEBUG
#define AI_LOG(msg) std::cerr << "[AI_DEBUG] " << msg << std::endl
#else
#define AI_LOG(msg)
#endif

Menu::Menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* boldFont) : renderer_(renderer), font_(font), boldFont_(boldFont) {
    AI_LOG("Menu initialized");
}

Menu::~Menu() {
    AI_LOG("Menu destructor called");
}

void Menu::renderButton(const std::string& text, int x, int y, bool selected) {
    SDL_Color buttonColor = selected ? SDL_Color{255, 255, 0, 255} : SDL_Color{59, 130, 246, 255}; // Jaune si sélectionné, bleu #3B82F6 sinon
    SDL_FRect buttonRect = {static_cast<float>(x), static_cast<float>(y), 200.0f, 50.0f};
    SDL_SetRenderDrawColor(renderer_, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer_, &buttonRect);
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font_, text.c_str(), text.length(), textColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (texture) {
            SDL_FRect textRect = {static_cast<float>(x + 10), static_cast<float>(y + 10), static_cast<float>(surface->w), static_cast<float>(surface->h)};
            SDL_RenderTexture(renderer_, texture, NULL, &textRect);
            SDL_DestroyTexture(texture);
        } else {
            AI_LOG("Failed to create texture for menu button: " + std::string(SDL_GetError()));
        }
        SDL_DestroySurface(surface);
    } else {
        AI_LOG("Failed to render menu button text: " + std::string(SDL_GetError()));
    }
}

void Menu::render() {
    SDL_SetRenderDrawColor(renderer_, 30, 64, 138, 255); // #1E3A8A
    SDL_RenderClear(renderer_);
    Uint32 time = SDL_GetTicks();
    float t = time / 1000.0f;
    SDL_Color neonColor = {255, 255, 255, static_cast<Uint8>(180 + 75 * std::sin(t * 2))};
    std::string title = "Cosmic Chess";
    SDL_Surface* titleSurface = TTF_RenderText_Blended(boldFont_, title.c_str(), title.length(), neonColor);
    if (titleSurface) {
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer_, titleSurface);
        if (titleTexture) {
            SDL_SetTextureAlphaMod(titleTexture, neonColor.a);
            SDL_FRect titleRect = {400.0f, 100.0f, static_cast<float>(titleSurface->w), static_cast<float>(titleSurface->h)};
            SDL_RenderTexture(renderer_, titleTexture, NULL, &titleRect);
            SDL_DestroyTexture(titleTexture);
        } else {
            AI_LOG("Failed to create texture for menu title: " + std::string(SDL_GetError()));
        }
        SDL_DestroySurface(titleSurface);
    } else {
        AI_LOG("Failed to render menu title text: " + std::string(SDL_GetError()));
    }
    renderButton("Start Game", 400, 250, selectedOption_ == 0);
    renderButton("Elo: " + std::to_string(elo_), 400, 350, selectedOption_ == 1);
    std::string thinkTimeStr = thinkTimes_[thinkTimeIndex_] == 0 ? "Instant" : std::to_string(thinkTimes_[thinkTimeIndex_] / 1000) + "s";
    renderButton("Think Time: " + thinkTimeStr, 400, 450, selectedOption_ == 2);
    SDL_RenderPresent(renderer_);
}

bool Menu::handleEvents(SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (event.key.key == SDLK_UP) {
            selectedOption_ = (selectedOption_ > 0) ? selectedOption_ - 1 : 2;
            AI_LOG("Menu: Selected option " + std::to_string(selectedOption_));
        } else if (event.key.key == SDLK_DOWN) {
            selectedOption_ = (selectedOption_ < 2) ? selectedOption_ + 1 : 0;
            AI_LOG("Menu: Selected option " + std::to_string(selectedOption_));
        } else if (event.key.key == SDLK_LEFT && selectedOption_ == 1) {
            elo_ = std::max(0, elo_ - 100);
            AI_LOG("Menu: Elo set to " + std::to_string(elo_));
        } else if (event.key.key == SDLK_RIGHT && selectedOption_ == 1) {
            elo_ = std::min(10000, elo_ + 100);
            AI_LOG("Menu: Elo set to " + std::to_string(elo_));
        } else if (event.key.key == SDLK_LEFT && selectedOption_ == 2) {
            thinkTimeIndex_ = (thinkTimeIndex_ > 0) ? thinkTimeIndex_ - 1 : 2;
            AI_LOG("Menu: Think time set to " + std::to_string(thinkTimes_[thinkTimeIndex_]) + "ms");
        } else if (event.key.key == SDLK_RIGHT && selectedOption_ == 2) {
            thinkTimeIndex_ = (thinkTimeIndex_ < 2) ? thinkTimeIndex_ + 1 : 0;
            AI_LOG("Menu: Think time set to " + std::to_string(thinkTimes_[thinkTimeIndex_]) + "ms");
        } else if (event.key.key == SDLK_RETURN) {
            if (selectedOption_ == 0) {
                startGame_ = true;
                AI_LOG("Menu: Start game selected");
                return true;
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
        int x = event.button.x;
        int y = event.button.y;
        if (x >= 400 && x <= 600) {
            if (y >= 250 && y <= 300) {
                selectedOption_ = 0;
                startGame_ = true;
                AI_LOG("Menu: Start game selected via mouse click");
                return true;
            } else if (y >= 350 && y <= 400) {
                selectedOption_ = 1;
                elo_ = std::min(10000, elo_ + 100);
                AI_LOG("Menu: Elo set to " + std::to_string(elo_) + " via mouse click");
            } else if (y >= 450 && y <= 500) {
                selectedOption_ = 2;
                thinkTimeIndex_ = (thinkTimeIndex_ < 2) ? thinkTimeIndex_ + 1 : 0;
                AI_LOG("Menu: Think time set to " + std::to_string(thinkTimes_[thinkTimeIndex_]) + "ms via mouse click");
            }
        }
    }
    return false;
}
