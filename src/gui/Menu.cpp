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

Menu::Menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* boldFont, TTF_Font* smallFont) : renderer_(renderer), font_(font), boldFont_(boldFont), smallFont_(smallFont) {
    AI_LOG("Menu initialized");
}

Menu::~Menu() {
    AI_LOG("Menu destructor called");
}

void Menu::renderButton(const std::string& text, int x, int y, bool selected, bool isSlider) {
    SDL_Color buttonColor = selected && !isSlider ? SDL_Color{100, 200, 255, 255} : SDL_Color{59, 130, 246, 255}; // #64C8FF for selected, #3B82F6 for normal
    SDL_FRect buttonRect = {static_cast<float>(x), static_cast<float>(y), isSlider ? static_cast<float>(ELO_SLIDER_WIDTH) : 200.0f, isSlider ? static_cast<float>(ELO_SLIDER_HEIGHT) : 50.0f};
    SDL_SetRenderDrawColor(renderer_, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer_, &buttonRect);
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font_, text.c_str(), text.length(), textColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (texture) {
            SDL_FRect textRect = {static_cast<float>(x + 10), static_cast<float>(y + (isSlider ? 5 : 10)), static_cast<float>(surface->w), static_cast<float>(surface->h)};
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

void Menu::renderSlider(int x, int y, int width, float value, float maxValue, const std::string& label) {
    SDL_SetRenderDrawColor(renderer_, 75, 85, 100, 255); // #4B5564 background
    SDL_FRect sliderRect = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(ELO_SLIDER_HEIGHT)};
    SDL_RenderFillRect(renderer_, &sliderRect);
    float sliderPos = (value / maxValue) * width;
    SDL_SetRenderDrawColor(renderer_, 100, 200, 255, 255); // #64C8FF handle
    SDL_FRect handleRect = {static_cast<float>(x + sliderPos - 5), static_cast<float>(y - 5), 10.0f, static_cast<float>(ELO_SLIDER_HEIGHT + 10)};
    SDL_RenderFillRect(renderer_, &handleRect);
    std::string labelText = label + ": " + std::to_string(static_cast<int>(value));
    SDL_Surface* labelSurface = TTF_RenderText_Blended(smallFont_, labelText.c_str(), labelText.length(), SDL_Color{255, 255, 255, 255});
    if (labelSurface) {
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer_, labelSurface);
        if (labelTexture) {
            SDL_FRect labelRect = {static_cast<float>(x), static_cast<float>(y - 25), static_cast<float>(labelSurface->w), static_cast<float>(labelSurface->h)};
            SDL_RenderTexture(renderer_, labelTexture, NULL, &labelRect);
            SDL_DestroyTexture(labelTexture);
        } else {
            AI_LOG("Failed to create texture for slider label: " + std::string(SDL_GetError()));
        }
        SDL_DestroySurface(labelSurface);
    } else {
        AI_LOG("Failed to render slider label text: " + std::string(SDL_GetError()));
    }
}

bool Menu::isPointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH) const {
    return x >= rectX && x <= rectX + rectW && y >= rectY && y <= rectY + rectH;
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
    renderSlider(ELO_SLIDER_X, ELO_SLIDER_Y, ELO_SLIDER_WIDTH, static_cast<float>(elo_), 10000.0f, "Elo");
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
        if (isPointInRect(x, y, 400, 250, 200, 50)) {
            selectedOption_ = 0;
            startGame_ = true;
            AI_LOG("Menu: Start game selected via mouse click");
            return true;
        } else if (isPointInRect(x, y, ELO_SLIDER_X, ELO_SLIDER_Y - 5, ELO_SLIDER_WIDTH, ELO_SLIDER_HEIGHT + 10)) {
            selectedOption_ = 1;
            isDraggingEloSlider_ = true;
            float sliderPos = static_cast<float>(x - ELO_SLIDER_X) / ELO_SLIDER_WIDTH;
            elo_ = static_cast<int>(sliderPos * 10000);
            elo_ = std::max(0, std::min(10000, elo_));
            AI_LOG("Menu: Elo set to " + std::to_string(elo_) + " via slider click");
        } else if (isPointInRect(x, y, 400, 450, 200, 50)) {
            selectedOption_ = 2;
            thinkTimeIndex_ = (thinkTimeIndex_ < 2) ? thinkTimeIndex_ + 1 : 0;
            AI_LOG("Menu: Think time set to " + std::to_string(thinkTimes_[thinkTimeIndex_]) + "ms via mouse click");
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION && isDraggingEloSlider_) {
        int x = event.motion.x;
        float sliderPos = static_cast<float>(x - ELO_SLIDER_X) / ELO_SLIDER_WIDTH;
        sliderPos = std::max(0.0f, std::min(1.0f, sliderPos));
        elo_ = static_cast<int>(sliderPos * 10000);
        elo_ = std::max(0, std::min(10000, elo_));
        AI_LOG("Menu: Elo set to " + std::to_string(elo_) + " via slider drag");
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_LEFT) {
        isDraggingEloSlider_ = false;
    }
    return false;
}
