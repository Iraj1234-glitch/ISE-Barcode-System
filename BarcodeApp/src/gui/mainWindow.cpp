#include "../../include/gui/MainWindow.h"

#include "../../include/generation/BarcodeGenerator.h"
#include "../../include/image_scanner/imageScanner.h"
#include "../../include/camera_scanner/CameraScanner.h"

#include <windows.h>
#include <commdlg.h>

#include <iostream>
#include <exception>

#pragma comment(lib, "Comdlg32.lib")

using namespace std;

// Opens a Windows file-selection dialog
string openImageDialog()
{
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter =
        "Image Files\0*.bmp;*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
        return filename;

    return "";
}

// Opens a Windows dialog to choose where to save the barcode
string saveImageDialog()
{
    char filename[MAX_PATH] = "barcode.bmp";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "BMP Image\0*.bmp\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = "bmp";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn))
        return filename;

    return "";
}

MainWindow::MainWindow()
{
    window.create(
        sf::VideoMode(1000, 700),
        "ISE Barcode Management System"
    );

    window.setFramerateLimit(60);

    currentPage = 0;

    inputCode = "";
    selectedImage = "";
    resultMessage = "";
    generatedImage = "";

    fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    isSuccess = false;
    previewLoaded = false;
}

void MainWindow::run()
{
    while (window.isOpen())
    {
        handleEvents();
        draw();
    }
}

void MainWindow::handleEvents()
{
    sf::Event event;

    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                handleClick(
                    (float)event.mouseButton.x,
                    (float)event.mouseButton.y
                );
            }
        }

        if (event.type == sf::Event::TextEntered)
        {
            handleTyping(event.text.unicode);
        }
    }
}

void MainWindow::drawText(
    const string& text,
    float x, float y,
    int size,
    sf::Color color
)
{
    if (!fontLoaded)
        return;

    sf::Text displayedText;
    displayedText.setFont(font);
    displayedText.setString(text);
    displayedText.setCharacterSize(size);
    displayedText.setFillColor(color);
    displayedText.setPosition(x, y);

    window.draw(displayedText);
}

void MainWindow::drawButton(
    float x, float y,
    float width, float height,
    const string& text
)
{
    sf::RectangleShape button(sf::Vector2f(width, height));

    button.setPosition(x, y);
    button.setFillColor(sf::Color(30, 150, 105));


    window.draw(button);

    drawText(
        text,
        x + 15,
        y + 12,
        17,
        sf::Color::White
    );
}

void MainWindow::drawSidebar()
{
    sf::RectangleShape sidebar(sf::Vector2f(220, 700));
    sidebar.setFillColor(sf::Color(24, 35, 55));

    window.draw(sidebar);

    drawText("ISE BARCODE", 25, 30, 25, sf::Color::White);
    drawText("MANAGEMENT SYSTEM", 25, 65, 13, sf::Color(180, 195, 215));

    sf::Color normal(190, 200, 215);
    sf::Color selected(70, 190, 140);

    drawText("Dashboard", 30, 150, 19,
        currentPage == 0 ? selected : normal);

    drawText("Generate Barcode", 30, 215, 19,
        currentPage == 1 ? selected : normal);

    drawText("Image Scanner", 30, 280, 19,
        currentPage == 2 ? selected : normal);

    drawText("Camera Scanner", 30, 345, 19,
        currentPage == 3 ? selected : normal);

    drawText("ISE Project", 30, 640, 14, sf::Color(150, 165, 185));
}

void MainWindow::drawDashboard()
{
    drawText("Dashboard", 260, 45, 32, sf::Color(30, 40, 55));

    drawText(
        "Welcome to the Barcode Management System.",
        260, 105, 18, sf::Color(90, 100, 115)
    );

    drawText("Choose an operation to continue:", 260, 165, 18,
        sf::Color(55, 65, 80));

    drawButton(260, 225, 300, 55, "Generate Barcode");
    drawButton(260, 310, 300, 55, "Scan an Image");
    drawButton(260, 395, 300, 55, "Scan Using Camera");

    drawText(
        "Generate and scan six-digit product barcodes.",
        260, 500, 16, sf::Color(100, 110, 125)
    );
}

void MainWindow::drawGenerator()
{
    drawText("Generate Barcode", 260, 45, 32, sf::Color(30, 40, 55));

    drawText("Enter a six-digit product code:", 260, 125, 19,
        sf::Color(55, 65, 80));

    sf::RectangleShape inputBox(sf::Vector2f(300, 48));
    inputBox.setPosition(260, 165);
    inputBox.setFillColor(sf::Color::White);
    inputBox.setOutlineThickness(1);
    inputBox.setOutlineColor(sf::Color(170, 180, 195));

    window.draw(inputBox);

    drawText(inputCode, 275, 175, 21, sf::Color(30, 40, 55));

    drawText(
        "Exactly 6 digits. The first digit cannot be zero.",
        260, 225, 14, sf::Color(110, 120, 135)
    );

    drawButton(260, 275, 200, 50, "Generate and Save");

    // Button to clear the previous barcode and start again
    drawButton(480, 275, 180, 50, "New Barcode");

    if (previewLoaded)
    {
        drawText("Generated Barcode:", 260, 365, 18,
            sf::Color(40, 50, 65));

        previewSprite.setPosition(260, 405);
        previewSprite.setScale(0.65f, 0.65f);

        window.draw(previewSprite);
    }

    if (!resultMessage.empty())
    {
        drawText(
            resultMessage,
            260, 590, 16,
            isSuccess ? sf::Color(20, 135, 85) : sf::Color(190, 55, 55)
        );
    }

    // Return to the dashboard
    drawButton(760, 620, 180, 45, "Back to Dashboard");
}

void MainWindow::drawImageScanner()
{
    drawText("Image Scanner", 260, 45, 32, sf::Color(30, 40, 55));

    drawText(
        "Choose an image containing a Code 128 barcode.",
        260, 125, 18, sf::Color(75, 85, 100)
    );

    drawButton(260, 185, 220, 50, "Choose Image");

    if (!selectedImage.empty())
    {
        drawText("Selected:", 260, 270, 16, sf::Color(50, 60, 75));
        drawText(selectedImage, 260, 300, 14, sf::Color(100, 110, 125));
    }

    drawButton(260, 350, 220, 50, "Scan Image");

    // Clear the previous image and start a new scan
    drawButton(510, 350, 220, 50, "New Scan");

    if (!resultMessage.empty())
    {
        drawText(
            resultMessage,
            260, 445, 18,
            isSuccess ? sf::Color(20, 135, 85) : sf::Color(190, 55, 55)
        );
    }

    // Return to the dashboard
    drawButton(760, 620, 180, 45, "Back to Dashboard");
}
void MainWindow::drawCameraScanner()
{
    drawText("Camera Scanner", 260, 45, 32, sf::Color(30, 40, 55));

    drawText(
        "Scan a barcode using your laptop camera.",
        260, 130, 18, sf::Color(75, 85, 100)
    );

    drawText(
        "Press the button below to open the camera scanner.",
        260, 175, 16, sf::Color(100, 110, 125)
    );

    drawButton(260, 240, 250, 55, "Start / Scan Again");

    // Clear the previous scanning result
    drawButton(530, 240, 180, 55, "Clear Result");

    if (!resultMessage.empty())
    {
        drawText(
            resultMessage,
            260, 350, 18,
            isSuccess ? sf::Color(20, 135, 85) : sf::Color(190, 55, 55)
        );
    }

    // Return to the dashboard
    drawButton(760, 620, 180, 45, "Back to Dashboard");
}
void MainWindow::draw()
{
    window.clear(sf::Color(245, 247, 250));

    drawSidebar();

    if (currentPage == 0)
        drawDashboard();

    else if (currentPage == 1)
        drawGenerator();

    else if (currentPage == 2)
        drawImageScanner();

    else if (currentPage == 3)
        drawCameraScanner();

    window.display();
}

void MainWindow::handleClick(float x, float y)
{
    // Back to Dashboard button on all working pages
    if (currentPage != 0 &&
        x >= 760 && x <= 940 &&
        y >= 620 && y <= 665)
    {
        currentPage = 0;
        resultMessage = "";
        return;
    }

    // Sidebar navigation
    if (x < 220)
    {
        if (y >= 130 && y < 195)
            currentPage = 0;

        else if (y >= 195 && y < 260)
            currentPage = 1;

        else if (y >= 260 && y < 325)
            currentPage = 2;

        else if (y >= 325 && y < 390)
            currentPage = 3;

        resultMessage = "";
        return;
    }

    // Dashboard buttons
    if (currentPage == 0)
    {
        if (y >= 225 && y <= 280)
            currentPage = 1;

        else if (y >= 310 && y <= 365)
            currentPage = 2;

        else if (y >= 395 && y <= 450)
            currentPage = 3;
    }

    // Generate barcode page
    else if (currentPage == 1)
    {
        // Generate barcode
        if (x >= 260 && x <= 460 &&
            y >= 275 && y <= 325)
        {
            generateBarcode();
        }

        // Start a completely new barcode generation
        else if (x >= 480 && x <= 660 &&
            y >= 275 && y <= 325)
        {
            inputCode = "";
            generatedImage = "";
            resultMessage = "";
            isSuccess = false;
            previewLoaded = false;
        }
    }

    // Image scanner page
    else if (currentPage == 2)
    {
        // Choose an image
        if (x >= 260 && x <= 480 &&
            y >= 185 && y <= 235)
        {
            selectedImage = openImageDialog();

            if (selectedImage.empty())
            {
                resultMessage = "No image selected.";
                isSuccess = false;
            }
            else
            {
                resultMessage = "Image selected.";
                isSuccess = false;
            }
        }

        // Scan selected image
        else if (x >= 260 && x <= 480 &&
            y >= 350 && y <= 400)
        {
            scanImage();
        }

        // Start a new image scanning operation
        else if (x >= 510 && x <= 730 &&
            y >= 350 && y <= 400)
        {
            selectedImage = "";
            resultMessage = "";
            isSuccess = false;
        }
    }

    // Camera scanner page
    else if (currentPage == 3)
    {
        // Start camera or scan another barcode
        if (x >= 260 && x <= 510 &&
            y >= 240 && y <= 295)
        {
            resultMessage = "";
            isSuccess = false;
            scanCamera();
        }

        // Clear previous camera result
        else if (x >= 530 && x <= 710 &&
            y >= 240 && y <= 295)
        {
            resultMessage = "";
            isSuccess = false;
        }
    }
}

void MainWindow::handleTyping(sf::Uint32 character)
{
    if (currentPage != 1)
        return;

    if (character == 8)
    {
        if (!inputCode.empty())
            inputCode.pop_back();
    }

    else if (character >= '0' && character <= '9')
    {
        if (inputCode.length() < 6)
            inputCode += (char)character;
    }
}

void MainWindow::loadPreview(const string& path)
{
    if (previewTexture.loadFromFile(path))
    {
        previewSprite.setTexture(previewTexture);
        previewLoaded = true;
    }
    else
    {
        previewLoaded = false;
    }
}

void MainWindow::generateBarcode()
{
    if (inputCode.length() != 6)
    {
        resultMessage = "Error: Enter exactly six digits.";
        isSuccess = false;
        return;
    }

    if (inputCode[0] == '0')
    {
        resultMessage = "Error: The first digit cannot be zero.";
        isSuccess = false;
        return;
    }

    string outputPath = saveImageDialog();

    if (outputPath.empty())
    {
        resultMessage = "Save cancelled.";
        isSuccess = false;
        return;
    }

    BarcodeGenerator generator;

    if (generator.generateBarcode(inputCode, outputPath))
    {
        generatedImage = outputPath;
        loadPreview(generatedImage);

        resultMessage = "Barcode generated and saved successfully.";
        isSuccess = true;
    }
    else
    {
        resultMessage = "Barcode generation failed.";
        isSuccess = false;
    }
}

void MainWindow::scanImage()
{
    if (selectedImage.empty())
    {
        resultMessage = "Please choose an image first.";
        isSuccess = false;
        return;
    }

    string code;
    string error;

    if (readBarcodeFromImage(selectedImage, code, error))
    {
        resultMessage = "Scanned code: " + code;
        isSuccess = true;
    }
    else
    {
        resultMessage = "Scan failed: " + error;
        isSuccess = false;
    }
}

void MainWindow::scanCamera()
{
    try
    {
        CameraScanner scanner;

        resultMessage = "Opening camera scanner...";
        isSuccess = false;

        // The camera module opens and controls its own OpenCV window.
        string code = scanner.scanUsingCamera();

        if (code.empty())
        {
            resultMessage = "No valid barcode was scanned.";
            isSuccess = false;
        }
        else
        {
            resultMessage = "Scanned code: " + code;
            isSuccess = true;
        }
    }
    catch (const exception& e)
    {
        resultMessage = "Camera error: " + string(e.what());
        isSuccess = false;
    }
    catch (...)
    {
        resultMessage = "An unknown camera error occurred.";
        isSuccess = false;
    }
}