#ifndef MENU_HPP
#define MENU_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class Menu {
public:
    Menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* boldFont);
    ~Menu();
    void render();
    bool handleEvents(SDL_Event& event);
    int getElo() const { return elo_; }
    int getThinkTime() const { return thinkTimes_[thinkTimeIndex_]; }

private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    TTF_Font* boldFont_;
    int selectedOption_ = 0;
    int elo_ = 1500;
    int thinkTimeIndex_ = 0;
    bool startGame_ = false;
    static constexpr int thinkTimes_[3] = {0, 3000, 5000};
    void renderButton(const std::string& text, int x, int y, bool selected);
};

#endif // MENU_HPP
