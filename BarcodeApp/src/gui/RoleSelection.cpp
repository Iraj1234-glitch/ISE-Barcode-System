#include <SFML/Graphics.hpp>
#include "Role.h"

class RoleSelection {
public:
    RoleSelection(sf::RenderWindow& win)
        : window(win),
          font(),
          title(),
          cashierBtn(),
          managerBtn(),
          passwordBox(),
          passwordPrompt(),
          enteredPassword("")
    {
        if (!font.loadFromFile("assets/arial.ttf")) {
            // fallback – the app will still run, text just uses default font
        }

        title.setFont(font);
        title.setString("Select Role");
        title.setCharacterSize(36);
        title.setFillColor(sf::Color::White);
        centerOrigin(title);
        title.setPosition(window.getSize().x / 2.f, 80.f);

        // ---------------- Cashier button ----------------
        cashierBtn.setSize({200.f, 50.f});
        cashierBtn.setFillColor(sf::Color(70, 130, 180));
        cashierBtn.setPosition(window.getSize().x / 2.f - 110.f, 180.f);

        cashierLabel.setFont(font);
        cashierLabel.setString("Cashier");
        cashierLabel.setCharacterSize(24);
        cashierLabel.setFillColor(sf::Color::White);
        centerOrigin(cashierLabel);
        cashierLabel.setPosition(cashierBtn.getPosition().x + 100.f,
                                 cashierBtn.getPosition().y + 25.f);

        // ---------------- Manager button ----------------
        managerBtn.setSize({200.f, 50.f});
        managerBtn.setFillColor(sf::Color(100, 149, 237));
        managerBtn.setPosition(window.getSize().x / 2.f - 110.f, 260.f);

        managerLabel.setFont(font);
        managerLabel.setString("Manager");
        managerLabel.setCharacterSize(24);
        managerLabel.setFillColor(sf::Color::White);
        centerOrigin(managerLabel);
        managerLabel.setPosition(managerBtn.getPosition().x + 100.f,
                                 managerBtn.getPosition().y + 25.f);

        // --------- Password overlay (hidden initially) ----------
        passwordBox.setSize({300.f, 50.f});
        passwordBox.setFillColor(sf::Color(30, 30, 30));
        passwordBox.setOutlineThickness(2.f);
        passwordBox.setOutlineColor(sf::Color::White);
        passwordBox.setPosition(window.getSize().x / 2.f - 150.f, 340.f);

        passwordPrompt.setFont(font);
        passwordPrompt.setString("Enter manager password:");
        passwordPrompt.setCharacterSize(20);
        passwordPrompt.setFillColor(sf::Color::White);
        centerOrigin(passwordPrompt);
        passwordPrompt.setPosition(window.getSize().x / 2.f,
                                   passwordBox.getPosition().y - 30.f);
    }

    // Returns true when a role has been selected (and password verified for manager)
    bool run() {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window.close();
                return false;
            }
            if (ev.type == sf::Event::MouseButtonPressed) {
                if (ev.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mouse(ev.mouseButton.x, ev.mouseButton.y);
                    if (cashierBtn.getGlobalBounds().contains(mouse)) {
                        g_currentRole = UserRole::Cashier;
                        return true;
                    }
                    if (managerBtn.getGlobalBounds().contains(mouse)) {
                        // show password overlay
                        waitingForPassword = true;
                    }
                }
            }
            if (waitingForPassword) {
                handlePasswordEvent(ev);
            }
        }

        // ---- draw ----
        window.clear(sf::Color(20, 20, 30));
        window.draw(title);
        window.draw(cashierBtn);
        window.draw(cashierLabel);
        window.draw(managerBtn);
        window.draw(managerLabel);
        if (waitingForPassword) drawPasswordOverlay();
        window.display();

        return false; // continue looping
    }

private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::Text title, cashierLabel, managerLabel, passwordPrompt, passwordMask;
    sf::RectangleShape cashierBtn, managerBtn, passwordBox;
    bool waitingForPassword = false;
    std::string enteredPassword;
    const std::string managerPassword = "manager123"; // <-- change as needed

    void centerOrigin(sf::Text& txt) {
        sf::FloatRect bounds = txt.getLocalBounds();
        txt.setOrigin(bounds.left + bounds.width / 2.f,
                      bounds.top + bounds.height / 2.f);
    }
    void centerOrigin(sf::RectangleShape& shape) {
        sf::FloatRect b = shape.getLocalBounds();
        shape.setOrigin(b.width / 2.f, b.height / 2.f);
    }

    void handlePasswordEvent(const sf::Event& ev) {
        if (ev.type == sf::Event::TextEntered) {
            if (ev.text.unicode == '\b') { // backspace
                if (!enteredPassword.empty())
                    enteredPassword.pop_back();
            } else if (ev.text.unicode < 128 && ev.text.unicode >= 32) {
                enteredPassword.push_back(static_cast<char>(ev.text.unicode));
            }
            updateMask();
        } else if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Enter) {
            if (enteredPassword == managerPassword) {
                g_currentRole = UserRole::Manager;
                waitingForPassword = false;
            } else {
                // flash red to indicate failure
                passwordBox.setOutlineColor(sf::Color::Red);
                // keep waiting – clear input
                enteredPassword.clear();
                updateMask();
            }
        }
    }

    void updateMask() {
        passwordMask.setFont(font);
        passwordMask.setString(std::string(enteredPassword.size(), '*'));
        passwordMask.setCharacterSize(24);
        passwordMask.setFillColor(sf::Color::White);
        centerOrigin(passwordMask);
        passwordMask.setPosition(passwordBox.getPosition().x + passwordBox.getSize().x / 2.f,
                                 passwordBox.getPosition().y + passwordBox.getSize().y / 2.f);
    }

    void drawPasswordOverlay() {
        window.draw(passwordBox);
        window.draw(passwordPrompt);
        window.draw(passwordMask);
    }
};
