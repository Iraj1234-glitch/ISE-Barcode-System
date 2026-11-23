#include "../../include/gui/SeasonManagementWindow.h"
#include <sstream>
#include <iomanip>

namespace {
    const sf::Color NAVY(32, 74, 128);
    const sf::Color GREEN(39, 128, 82);
    const sf::Color GREY(90, 90, 90);
}

SeasonManagementWindow::SeasonManagementWindow(const std::string& seasonFile)
    : seasonManager(seasonFile) {
    window.create(sf::VideoMode(1000, 650), "ISE Manager - Season Management");
    window.setFramerateLimit(60);
    fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    statusSuccess = true;
    activeInput = false;
    editing = false;
    selectedIndex = -1;
    activeField = 0;
    listScroll = 0;
    discountInput = "10";
}

void SeasonManagementWindow::run() {
    while (window.isOpen()) {
        handleEvents();
        draw();
    }
}

void SeasonManagementWindow::setStatus(const std::string& message, bool success) {
    statusMessage = message;
    statusSuccess = success;
}

bool SeasonManagementWindow::hit(float x, float y, float rx, float ry, float rw, float rh) const {
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void SeasonManagementWindow::clearForm() {
    startDateInput.clear();
    endDateInput.clear();
    discountInput = "10";
    activeInput = false;
    editing = false;
    selectedIndex = -1;
    activeField = 0;
}

void SeasonManagementWindow::loadSeasonIntoForm(int index) {
    const Season* season = seasonManager.getSeasonAt(index);
    if (!season) return;
    selectedIndex = index;
    editing = true;
    startDateInput = season->startDate;
    endDateInput = season->endDate;
    std::ostringstream value;
    value << std::fixed << std::setprecision(2) << season->discountPercentage;
    discountInput = value.str();
    activeInput = season->isActive;
    setStatus("Editing selected season. Change fields and save.", true);
}

void SeasonManagementWindow::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            handleClick(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        else if (event.type == sf::Event::MouseWheelScrolled) {
            listScroll -= static_cast<int>(event.mouseWheelScroll.delta);
            if (listScroll < 0) listScroll = 0;
            int maxScroll = seasonManager.getSeasonCount() - 8;
            if (maxScroll < 0) maxScroll = 0;
            if (listScroll > maxScroll) listScroll = maxScroll;
        }
        else if (event.type == sf::Event::TextEntered) handleTyping(event.text.unicode);
    }
}

void SeasonManagementWindow::handleTyping(unsigned int unicode) {
    std::string* target = nullptr;
    int maxLength = 10;
    if (activeField == 1) target = &startDateInput;
    else if (activeField == 2) target = &endDateInput;
    else if (activeField == 3) { target = &discountInput; maxLength = 6; }
    else return;

    if (unicode == 8) {
        if (!target->empty()) target->pop_back();
        return;
    }
    if (unicode < 32 || unicode > 126 || static_cast<int>(target->size()) >= maxLength) return;
    char ch = static_cast<char>(unicode);
    if (activeField == 3) {
        if ((ch < '0' || ch > '9') && ch != '.') return;
        if (ch == '.' && target->find('.') != std::string::npos) return;
    }
    else if ((ch < '0' || ch > '9') && ch != '-') return;
    *target += ch;
}

void SeasonManagementWindow::handleClick(float x, float y) {
    activeField = 0;
    if (hit(x, y, 30, 100, 250, 34)) activeField = 1;
    else if (hit(x, y, 300, 100, 250, 34)) activeField = 2;
    else if (hit(x, y, 570, 100, 150, 34)) activeField = 3;

    for (int row = 0; row < 8; ++row) {
        int index = listScroll + row;
        if (index >= seasonManager.getSeasonCount()) break;
        if (hit(x, y, 30, 260 + row * 34, 920, 30)) {
            loadSeasonIntoForm(index);
            return;
        }
    }

    if (hit(x, y, 750, 100, 190, 34)) {
        activeInput = !activeInput;
        return;
    }
    if (hit(x, y, 30, 160, 180, 38)) {
        double discount = -1.0;
        try { discount = std::stod(discountInput); } catch (...) {}
        std::string error;
        bool ok = editing
            ? seasonManager.editSeason(selectedIndex, startDateInput, endDateInput, discount, activeInput, error)
            : seasonManager.addSeason(startDateInput, endDateInput, discount, activeInput, error);
        setStatus(error, ok);
        if (ok) clearForm();
        return;
    }
    if (hit(x, y, 225, 160, 180, 38)) {
        if (selectedIndex < 0) { setStatus("Select a season first.", false); return; }
        const Season* season = seasonManager.getSeasonAt(selectedIndex);
        if (!season) return;
        std::string error;
        bool ok = season->isActive
            ? seasonManager.deactivateSeason(selectedIndex, error)
            : seasonManager.activateSeason(selectedIndex, error);
        setStatus(error, ok);
        if (ok) loadSeasonIntoForm(selectedIndex);
        return;
    }
    if (hit(x, y, 420, 160, 150, 38)) {
        if (selectedIndex < 0) { setStatus("Select a season first.", false); return; }
        bool ok = seasonManager.deleteSeason(selectedIndex);
        setStatus(ok ? "Season deleted." : "Could not delete season.", ok);
        if (ok) clearForm();
        return;
    }
    if (hit(x, y, 585, 160, 150, 38)) {
        clearForm();
        setStatus("Ready to add a new season.", true);
        return;
    }
    if (hit(x, y, 755, 160, 185, 38)) {
        seasonManager.loadSeasons();
        setStatus("Seasons reloaded from Seasons.txt.", true);
        return;
    }
}

void SeasonManagementWindow::drawText(const std::string& text, float x, float y, unsigned int size, sf::Color color) {
    if (!fontLoaded) return;
    sf::Text label;
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(size);
    label.setFillColor(color);
    label.setPosition(x, y);
    window.draw(label);
}

void SeasonManagementWindow::drawButton(float x, float y, float w, float h, const std::string& text, sf::Color color) {
    sf::RectangleShape shape(sf::Vector2f(w, h));
    shape.setPosition(x, y);
    shape.setFillColor(color);
    window.draw(shape);
    if (!fontLoaded) return;
    sf::Text label;
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(15);
    label.setFillColor(sf::Color::White);
    sf::FloatRect bounds = label.getLocalBounds();
    label.setPosition(x + (w - bounds.width) / 2.0f - bounds.left, y + (h - bounds.height) / 2.0f - bounds.top);
    window.draw(label);
}

void SeasonManagementWindow::drawField(float x, float y, float w, float h, const std::string& value, bool active) {
    sf::RectangleShape shape(sf::Vector2f(w, h));
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color::White);
    shape.setOutlineThickness(active ? 2.0f : 1.0f);
    shape.setOutlineColor(active ? sf::Color(40, 110, 200) : sf::Color(160, 160, 160));
    window.draw(shape);
    drawText(value, x + 7, y + 7, 16, sf::Color::Black);
}

void SeasonManagementWindow::draw() {
    window.clear(sf::Color(236, 240, 245));
    sf::RectangleShape header(sf::Vector2f(1000, 50));
    header.setFillColor(NAVY);
    window.draw(header);
    drawText("Manager: Seasonal Discount Management", 24, 12, 22, sf::Color::White);

    drawText("Start date (YYYY-MM-DD)", 30, 72, 14, GREY);
    drawText("End date (YYYY-MM-DD)", 300, 72, 14, GREY);
    drawText("Discount %", 570, 72, 14, GREY);
    drawField(30, 100, 250, 34, startDateInput, activeField == 1);
    drawField(300, 100, 250, 34, endDateInput, activeField == 2);
    drawField(570, 100, 150, 34, discountInput, activeField == 3);
    drawButton(750, 100, 190, 34, activeInput ? "Active: YES" : "Active: NO", activeInput ? GREEN : GREY);

    drawButton(30, 160, 180, 38, editing ? "Save Changes" : "Add Season", GREEN);
    drawButton(225, 160, 180, 38, "Activate / Deactivate", NAVY);
    drawButton(420, 160, 150, 38, "Delete Season", sf::Color(160, 45, 45));
    drawButton(585, 160, 150, 38, "Clear Form", GREY);
    drawButton(755, 160, 185, 38, "Reload Seasons", NAVY);

    drawText("Saved seasons (select a row to edit)", 30, 235, 17, NAVY);
    for (int row = 0; row < 8; ++row) {
        int index = listScroll + row;
        float y = 260.0f + row * 34.0f;
        sf::RectangleShape bg(sf::Vector2f(920, 30));
        bg.setPosition(30, y);
        bg.setFillColor(index == selectedIndex ? sf::Color(186, 214, 245) : sf::Color::White);
        window.draw(bg);
        const Season* season = seasonManager.getSeasonAt(index);
        if (season) {
            std::ostringstream line;
            line << season->startDate << "   to   " << season->endDate << "      "
                 << std::fixed << std::setprecision(2) << season->discountPercentage << "%      "
                 << (season->isActive ? "ACTIVE" : "INACTIVE");
            drawText(line.str(), 42, y + 5, 15, sf::Color::Black);
        }
    }
    sf::Color statusColor = statusSuccess ? sf::Color(20, 110, 50) : sf::Color(160, 30, 30);
    drawText(statusMessage.empty() ? "Dates must use YYYY-MM-DD. Active seasons cannot overlap." : statusMessage,
             30, 590, 16, statusMessage.empty() ? GREY : statusColor);
    window.display();
}
