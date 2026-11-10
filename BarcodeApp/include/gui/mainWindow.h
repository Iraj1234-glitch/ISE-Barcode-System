#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class MainWindow
{
private:
    sf::RenderWindow window;
    sf::Font font;

    sf::Texture previewTexture;
    sf::Sprite previewSprite;

    int currentPage;

    std::string inputCode;
    std::string selectedImage;
    std::string resultMessage;
    std::string generatedImage;

    bool fontLoaded;
    bool isSuccess;
    bool previewLoaded;

    void handleEvents();
    void draw();

    void drawSidebar();
    void drawDashboard();
    void drawGenerator();
    void drawImageScanner();
    void drawCameraScanner();

    void drawButton(
        float x, float y,
        float width, float height,
        const std::string& text
    );

    void drawText(
        const std::string& text,
        float x, float y,
        int size,
        sf::Color color
    );

    void handleClick(float x, float y);
    void handleTyping(sf::Uint32 character);

    void generateBarcode();
    void scanImage();
    void scanCamera();
    void loadPreview(const std::string& path);

public:
    MainWindow();

    void run();
};