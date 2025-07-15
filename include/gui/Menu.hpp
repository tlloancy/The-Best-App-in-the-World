#ifndef MENU_HPP
#define MENU_HPP

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class Menu {
public:
    Menu(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* boldFont, TTF_Font* smallFont);
    ~Menu();
    void render();
    bool handleEvents(SDL_Event& event);
    int getElo() const { return elo_; }
    int getThinkTime() const { return thinkTimes_[thinkTimeIndex_]; }
private:
    void renderButton(const std::string& text, int x, int y, bool selected, bool isSlider = false);
    void renderSlider(int x, int y, int width, float value, float maxValue, const std::string& label);
    bool isPointInRect(int x, int y, int rectX, int rectY, int rectW, int rectH) const;

    SDL_Renderer* renderer_;
    TTF_Font* font_;
    TTF_Font* boldFont_;
    TTF_Font* smallFont_;
    int selectedOption_ = 0;
    int elo_ = 1500;
    int thinkTimeIndex_ = 0;
    bool startGame_ = false;
    bool isDraggingEloSlider_ = false;
    static constexpr int thinkTimes_[3] = {0, 3000, 5000};
    static constexpr int ELO_SLIDER_X = 400;
    static constexpr int ELO_SLIDER_Y = 350;
    static constexpr int ELO_SLIDER_WIDTH = 200;
    static constexpr int ELO_SLIDER_HEIGHT = 20;
};

#endif // MENU_HPP
