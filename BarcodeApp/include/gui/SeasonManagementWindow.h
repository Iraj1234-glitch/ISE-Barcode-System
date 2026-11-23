#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "../billing/SeasonManager.h"

class SeasonManagementWindow {
private:
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;
    SeasonManager seasonManager;

    std::string startDateInput;
    std::string endDateInput;
    std::string discountInput;
    std::string statusMessage;
    bool statusSuccess;
    bool activeInput;
    bool editing;
    int selectedIndex;
    int activeField;
    int listScroll;

    void handleEvents();
    void handleClick(float x, float y);
    void handleTyping(unsigned int unicode);
    void draw();
    void drawText(const std::string& text, float x, float y, unsigned int size, sf::Color color);
    void drawButton(float x, float y, float w, float h, const std::string& text, sf::Color color);
    void drawField(float x, float y, float w, float h, const std::string& text, bool active);
    bool hit(float x, float y, float rx, float ry, float rw, float rh) const;
    void setStatus(const std::string& message, bool success);
    void clearForm();
    void loadSeasonIntoForm(int index);

public:
    explicit SeasonManagementWindow(const std::string& seasonFile = "Seasons.txt");
    void run();
};
