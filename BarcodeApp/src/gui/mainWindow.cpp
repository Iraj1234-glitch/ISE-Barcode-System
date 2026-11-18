#define NOMINMAX

#include "../../include/gui/MainWindow.h"

#include "../../include/generation/BarcodeGenerator.h"
#include "../../include/image_scanner/imageScanner.h"
#include "../../include/camera_scanner/CameraScanner.h"

#include <windows.h>
#include <commdlg.h>

#include <algorithm>
#include <ctime>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#pragma comment(lib, "Comdlg32.lib")

using namespace std;

// ======================================================================
// Layout constants (shared by drawing code and click detection)
// ======================================================================

static const int WIN_W = 1200;
static const int WIN_H = 760;

static const int SIDE_COUNT = 5;
static const float SIDE_Y[SIDE_COUNT] = { 140, 195, 250, 305, 360 };
static const int SIDE_PAGE[SIDE_COUNT] =
{
    PAGE_DASHBOARD, PAGE_ORDER, PAGE_GENERATE, PAGE_IMAGE, PAGE_CAMERA
};
static const char* SIDE_LABEL[SIDE_COUNT] =
{
    "Dashboard", "New Order", "Generate Barcode", "Image Scanner", "Camera Scanner"
};

static const UiRect R_BACK = { 990, 700, 180, 45 };

// Dashboard
static const UiRect R_DASH_ORDER = { 260, 225, 300, 55 };
static const UiRect R_DASH_GEN = { 260, 295, 300, 55 };
static const UiRect R_DASH_IMG = { 260, 365, 300, 55 };
static const UiRect R_DASH_CAM = { 260, 435, 300, 55 };

// Generator
static const UiRect R_GEN_INPUT = { 260, 165, 300, 48 };
static const UiRect R_GEN_GO = { 260, 275, 200, 50 };
static const UiRect R_GEN_NEW = { 480, 275, 180, 50 };

// Image scanner
static const UiRect R_IMG_CHOOSE = { 260, 185, 220, 50 };
static const UiRect R_IMG_SCAN = { 260, 350, 220, 50 };
static const UiRect R_IMG_NEW = { 510, 350, 220, 50 };

// Camera scanner
static const UiRect R_CAM_START = { 260, 240, 250, 55 };
static const UiRect R_CAM_CLEAR = { 530, 240, 180, 55 };

// Order page
static const UiRect R_NAME = { 250, 110, 320, 40 };
static const UiRect R_CONTACT = { 600, 110, 280, 40 };
static const UiRect R_CODE = { 250, 195, 200, 40 };
static const UiRect R_QTY = { 480, 195, 90, 40 };
static const UiRect R_SCAN_IMG = { 250, 250, 140, 40 };
static const UiRect R_SCAN_CAM = { 400, 250, 140, 40 };
static const UiRect R_DESC = { 550, 250, 170, 40 };
static const UiRect R_ADD = { 735, 250, 145, 40 };
static const UiRect R_PRODUCT = { 250, 305, 630, 105 };
static const UiRect R_REMOVE = { 250, 645, 180, 40 };
static const UiRect R_UPDATE = { 445, 645, 160, 40 };
static const UiRect R_SUMMARY = { 910, 85, 260, 300 };
static const UiRect R_BILL = { 910, 420, 260, 55 };
static const UiRect R_CLEAR = { 910, 490, 260, 45 };

// Order table
static const float TBL_X = 250;
static const float TBL_Y = 425;
static const float TBL_W = 630;
static const float HDR_H = 26;
static const float ROW_H = 28;
static const int TBL_ROWS = 6;

// Payment page
static const UiRect R_CASH = { 250, 215, 150, 45 };
static const UiRect R_CARD = { 415, 215, 150, 45 };
static const UiRect R_WALLET = { 580, 215, 150, 45 };
static const UiRect R_PAYFIELD = { 250, 310, 320, 40 };
static const UiRect R_CONFIRM = { 250, 430, 230, 50 };
static const UiRect R_BACKORDER = { 500, 430, 180, 50 };

// Bill page
static const UiRect R_RECEIPT = { 250, 25, 600, 660 };
static const UiRect R_SAVEBILL = { 890, 110, 260, 50 };
static const UiRect R_NEWORDER = { 890, 180, 260, 50 };
static const float BILL_ROWS_Y = 231;
static const float BILL_ROW_H = 24;
static const int BILL_ROWS = 12;

// Colours
static const sf::Color COL_TITLE(30, 40, 55);
static const sf::Color COL_TEXT(55, 65, 80);
static const sf::Color COL_MUTED(100, 110, 125);
static const sf::Color COL_GREEN(20, 135, 85);
static const sf::Color COL_RED(190, 55, 55);
static const sf::Color COL_BLUE(60, 100, 170);
static const sf::Color COL_GREY(120, 135, 155);
static const sf::Color COL_DANGER(185, 70, 70);

// ======================================================================
// Small helpers
// ======================================================================

static bool hit(const UiRect& r, float x, float y)
{
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

static string trimText(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static string money(double value)
{
    ostringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

static string fitText(const string& s, size_t maxChars)
{
    if (s.length() <= maxChars)
        return s;

    if (maxChars <= 3)
        return s.substr(0, maxChars);

    return s.substr(0, maxChars - 3) + "...";
}

// Accepts only digits; result must be at least 1
static bool parsePositiveInt(const string& s, int& value)
{
    if (s.empty() || s.length() > 6)
        return false;

    int result = 0;

    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] < '0' || s[i] > '9')
            return false;

        result = result * 10 + (s[i] - '0');
    }

    if (result < 1)
        return false;

    value = result;
    return true;
}

// Accepts digits with at most one decimal point
static bool parseMoney(const string& s, double& value)
{
    if (s.empty())
        return false;

    int dots = 0;
    int digits = 0;

    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == '.')
            dots++;
        else if (s[i] >= '0' && s[i] <= '9')
            digits++;
        else
            return false;
    }

    if (dots > 1 || digits == 0)
        return false;

    try
    {
        value = stod(s);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

// Breaks text into at most maxLines lines of about maxChars characters
static string wrapText(const string& text, size_t maxChars, int maxLines)
{
    string out;
    string line;
    int used = 1;
    bool truncated = false;

    istringstream words(text);
    string word;

    while (words >> word)
    {
        if (!line.empty() && line.size() + 1 + word.size() > maxChars)
        {
            if (used >= maxLines)
            {
                truncated = true;
                break;
            }

            out += line + "\n";
            used++;
            line = word;
        }
        else
        {
            line += (line.empty() ? "" : " ") + word;
        }
    }

    out += line;

    if (truncated)
        out += "...";

    return out;
}

static string modeLabel(const string& mode)
{
    if (mode == "cash")
        return "Cash";

    if (mode == "card")
        return "Card";

    if (mode == "wallet")
        return "Wallet";

    return mode;
}

// Today's date as YYYY-MM-DD and the time as HH:MM:SS
static void currentDateTime(string& date, string& timeText)
{
    time_t now = time(nullptr);
    tm local = {};

#ifdef _MSC_VER
    localtime_s(&local, &now);
#else
    local = *localtime(&now);
#endif

    ostringstream d;
    d << (local.tm_year + 1900) << "-"
      << setw(2) << setfill('0') << (local.tm_mon + 1) << "-"
      << setw(2) << setfill('0') << local.tm_mday;

    ostringstream t;
    t << setw(2) << setfill('0') << local.tm_hour << ":"
      << setw(2) << setfill('0') << local.tm_min << ":"
      << setw(2) << setfill('0') << local.tm_sec;

    date = d.str();
    timeText = t.str();
}

// ======================================================================
// Windows file dialogs
// ======================================================================

// Opens a Windows file-selection dialog
static string openImageDialog(HWND owner)
{
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
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
static string saveImageDialog(HWND owner)
{
    char filename[MAX_PATH] = "barcode.bmp";

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
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

// ======================================================================
// Construction and main loop
// ======================================================================

MainWindow::MainWindow()
    : productManager("Product.txt", "Inventory.txt"), inventoryEnabled(false)
{
    window.create(
        sf::VideoMode(WIN_W, WIN_H),
        "ISE Barcode Management System"
    );

    window.setFramerateLimit(60);

    // Load saved orders (needed for generating the next order ID)
    orderManager.reset(new OrderManager("Order.txt", "OrderDetail.txt"));
    orderManager->loadAll();

    // Load products and, when supplied, the separate stock file.
    productManager.loadProducts();
    inventoryEnabled = productManager.loadInventory();

    currentPage = PAGE_DASHBOARD;
    activeField = FIELD_NONE;

    inputCode = "";
    selectedImage = "";
    resultMessage = "";
    generatedImage = "";

    fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    isSuccess = false;
    previewLoaded = false;

    customerName = "";
    customerContact = "";
    orderCode = "";
    orderQty = "1";
    productFound = false;
    showDescription = false;
    cartCount = 0;
    selectedRow = -1;
    cartScroll = 0;

    payMode = "cash";
    payMerchant = "";
    payTendered = "";

    billItemCount = 0;
    billScroll = 0;
    billTotal = 0.0;
    billReceived = 0.0;
    billChange = 0.0;
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

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            handleScroll(
                event.mouseWheelScroll.delta,
                (float)event.mouseWheelScroll.x,
                (float)event.mouseWheelScroll.y
            );
        }

        if (event.type == sf::Event::TextEntered)
        {
            handleTyping(event.text.unicode);
        }
    }
}

void MainWindow::setMessage(const string& text, bool success)
{
    resultMessage = text;
    isSuccess = success;
}

// ======================================================================
// Drawing helpers
// ======================================================================

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

void MainWindow::drawTextRight(
    const string& text,
    float xRight, float y,
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

    sf::FloatRect bounds = displayedText.getLocalBounds();
    displayedText.setPosition(xRight - bounds.width - bounds.left, y);

    window.draw(displayedText);
}

void MainWindow::drawTextCentered(
    const string& text,
    float xCenter, float y,
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

    sf::FloatRect bounds = displayedText.getLocalBounds();
    displayedText.setPosition(xCenter - bounds.width / 2 - bounds.left, y);

    window.draw(displayedText);
}

void MainWindow::drawButton(
    const UiRect& rect,
    const string& text,
    sf::Color color
)
{
    sf::RectangleShape button(sf::Vector2f(rect.w, rect.h));

    button.setPosition(rect.x, rect.y);
    button.setFillColor(color);

    window.draw(button);

    if (!fontLoaded)
        return;

    sf::Text label;
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(17);
    label.setFillColor(sf::Color::White);

    sf::FloatRect bounds = label.getLocalBounds();

    label.setPosition(
        rect.x + (rect.w - bounds.width) / 2 - bounds.left,
        rect.y + (rect.h - bounds.height) / 2 - bounds.top
    );

    window.draw(label);
}

void MainWindow::drawField(const UiRect& rect, const string& text, bool active)
{
    sf::RectangleShape box(sf::Vector2f(rect.w, rect.h));

    box.setPosition(rect.x, rect.y);
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(active ? 2.0f : 1.0f);
    box.setOutlineColor(
        active ? sf::Color(30, 150, 105) : sf::Color(170, 180, 195)
    );

    window.draw(box);

    string shown = text;

    // Blinking cursor in the active box
    if (active && (caretClock.getElapsedTime().asMilliseconds() % 1000) < 500)
        shown += "|";

    drawText(shown, rect.x + 10, rect.y + (rect.h - 24) / 2, 19, COL_TITLE);
}

void MainWindow::drawMessage(float x, float y, int size)
{
    if (resultMessage.empty())
        return;

    drawText(resultMessage, x, y, size, isSuccess ? COL_GREEN : COL_RED);
}

void MainWindow::drawBackButton()
{
    drawButton(R_BACK, "Back to Dashboard");
}

// ======================================================================
// Sidebar and page drawing
// ======================================================================

void MainWindow::drawSidebar()
{
    sf::RectangleShape sidebar(sf::Vector2f(220, (float)WIN_H));
    sidebar.setFillColor(sf::Color(24, 35, 55));

    window.draw(sidebar);

    drawText("ISE BARCODE", 25, 30, 25, sf::Color::White);
    drawText("MANAGEMENT SYSTEM", 25, 65, 13, sf::Color(180, 195, 215));

    sf::Color normal(190, 200, 215);
    sf::Color selected(70, 190, 140);

    // Payment and bill belong to the "New Order" section
    int highlighted = currentPage;

    if (currentPage == PAGE_PAYMENT || currentPage == PAGE_BILL)
        highlighted = PAGE_ORDER;

    for (int i = 0; i < SIDE_COUNT; i++)
    {
        drawText(
            SIDE_LABEL[i], 30, SIDE_Y[i], 19,
            SIDE_PAGE[i] == highlighted ? selected : normal
        );
    }

    drawText("ISE Project", 30, 700, 14, sf::Color(150, 165, 185));
}

void MainWindow::drawDashboard()
{
    drawText("Dashboard", 260, 45, 32, COL_TITLE);

    drawText(
        "Welcome to the ISE Point of Sale system.",
        260, 100, 18, sf::Color(90, 100, 115)
    );

    // Quick statistics from the saved orders
    double revenue = 0.0;
    int total = orderManager->getOrderCount();

    for (int i = 0; i < total; i++)
    {
        const Order* order = orderManager->getOrderAt(i);

        if (order != nullptr && order->getOrderStatus() == "paid")
            revenue += order->getPaymentAmount();
    }

    drawText(
        "Orders recorded: " + to_string(total) +
        "     Paid revenue: Rs " + money(revenue) +
        "     Products in catalog: " + to_string(productManager.getProductCount()),
        260, 138, 16, COL_TEXT
    );

    drawText("Choose an operation to continue:", 260, 180, 18, COL_TEXT);

    drawButton(R_DASH_ORDER, "New Order (POS)");
    drawButton(R_DASH_GEN, "Generate Barcode", COL_BLUE);
    drawButton(R_DASH_IMG, "Scan an Image", COL_BLUE);
    drawButton(R_DASH_CAM, "Scan Using Camera", COL_BLUE);

    drawText(
        "Record orders, scan six-digit product barcodes and print bills.",
        260, 520, 16, sf::Color(100, 110, 125)
    );

    if (productManager.getProductCount() == 0)
    {
        drawText(
            "Product catalog is empty. Put Product.txt next to the program.",
            260, 555, 16, COL_RED
        );
    }
}

void MainWindow::drawGenerator()
{
    drawText("Generate Barcode", 260, 45, 32, COL_TITLE);

    drawText("Enter a six-digit product code:", 260, 125, 19, COL_TEXT);

    sf::RectangleShape inputBox(sf::Vector2f(R_GEN_INPUT.w, R_GEN_INPUT.h));
    inputBox.setPosition(R_GEN_INPUT.x, R_GEN_INPUT.y);
    inputBox.setFillColor(sf::Color::White);
    inputBox.setOutlineThickness(1);
    inputBox.setOutlineColor(sf::Color(170, 180, 195));

    window.draw(inputBox);

    drawText(inputCode, 275, 175, 21, COL_TITLE);

    drawText(
        "Exactly 6 digits. The first digit cannot be zero.",
        260, 225, 14, sf::Color(110, 120, 135)
    );

    drawButton(R_GEN_GO, "Generate and Save");
    drawButton(R_GEN_NEW, "New Barcode", COL_BLUE);

    if (previewLoaded)
    {
        drawText("Generated Barcode:", 260, 365, 18, sf::Color(40, 50, 65));

        previewSprite.setPosition(260, 405);
        previewSprite.setScale(0.65f, 0.65f);

        window.draw(previewSprite);
    }

    drawMessage(260, 590, 16);
    drawBackButton();
}

void MainWindow::drawImageScanner()
{
    drawText("Image Scanner", 260, 45, 32, COL_TITLE);

    drawText(
        "Choose an image containing a Code 128 barcode.",
        260, 125, 18, sf::Color(75, 85, 100)
    );

    drawButton(R_IMG_CHOOSE, "Choose Image");

    if (!selectedImage.empty())
    {
        drawText("Selected:", 260, 270, 16, sf::Color(50, 60, 75));
        drawText(fitText(selectedImage, 90), 260, 300, 14, COL_MUTED);
    }

    drawButton(R_IMG_SCAN, "Scan Image");
    drawButton(R_IMG_NEW, "New Scan", COL_BLUE);

    drawMessage(260, 445, 18);
    drawBackButton();
}

void MainWindow::drawCameraScanner()
{
    drawText("Camera Scanner", 260, 45, 32, COL_TITLE);

    drawText(
        "Scan a barcode using your laptop camera.",
        260, 130, 18, sf::Color(75, 85, 100)
    );

    drawText(
        "Press the button below to open the camera scanner.",
        260, 175, 16, COL_MUTED
    );

    drawButton(R_CAM_START, "Start / Scan Again");
    drawButton(R_CAM_CLEAR, "Clear Result", COL_BLUE);

    drawMessage(260, 350, 18);
    drawBackButton();
}

// ----------------------------------------------------------------------
// Order page
// ----------------------------------------------------------------------

void MainWindow::drawProductPanel()
{
    sf::RectangleShape panel(sf::Vector2f(R_PRODUCT.w, R_PRODUCT.h));
    panel.setPosition(R_PRODUCT.x, R_PRODUCT.y);
    panel.setFillColor(sf::Color::White);
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color(200, 208, 220));
    window.draw(panel);

    float x = R_PRODUCT.x + 15;
    float y = R_PRODUCT.y + 10;

    if (!productFound)
    {
        if (productManager.getProductCount() == 0)
        {
            drawText("The product catalog is empty (Product.txt not found).",
                x, y + 8, 16, COL_RED);
        }
        else if (orderCode.length() == 6)
        {
            drawText("No product found for code " + orderCode + ".",
                x, y + 8, 17, COL_RED);
        }
        else
        {
            drawText("Type or scan a 6-digit product code to look it up.",
                x, y + 8, 16, COL_MUTED);
        }

        return;
    }

    drawText(fitText(foundProduct.name, 40), x, y, 21, COL_TITLE);

    drawText(
        "Price: Rs " + money(foundProduct.price) +
        "      Barcode: " + foundProduct.barcode +
        "      ID: " + foundProduct.productId,
        x, y + 32, 16, COL_TEXT
    );

    if (showDescription)
    {
        drawText(
            wrapText(foundProduct.description, 78, 2),
            x, y + 58, 15, COL_MUTED
        );
    }
    else
    {
        drawText("Press Description to see the product details.",
            x, y + 62, 14, sf::Color(130, 140, 155));
    }
}

void MainWindow::drawCartTable()
{
    // Header
    sf::RectangleShape header(sf::Vector2f(TBL_W, HDR_H));
    header.setPosition(TBL_X, TBL_Y);
    header.setFillColor(sf::Color(24, 35, 55));
    window.draw(header);

    sf::Color white = sf::Color::White;

    drawText("#", TBL_X + 10, TBL_Y + 4, 15, white);
    drawText("Product", TBL_X + 45, TBL_Y + 4, 15, white);
    drawTextRight("Price", TBL_X + 420, TBL_Y + 4, 15, white);
    drawTextRight("Qty", TBL_X + 495, TBL_Y + 4, 15, white);
    drawTextRight("Subtotal", TBL_X + 615, TBL_Y + 4, 15, white);

    // Rows
    for (int r = 0; r < TBL_ROWS; r++)
    {
        int index = cartScroll + r;
        float y = TBL_Y + HDR_H + r * ROW_H;

        sf::RectangleShape row(sf::Vector2f(TBL_W, ROW_H));
        row.setPosition(TBL_X, y);

        if (index < cartCount && index == selectedRow)
            row.setFillColor(sf::Color(205, 238, 222));
        else if (r % 2 == 0)
            row.setFillColor(sf::Color::White);
        else
            row.setFillColor(sf::Color(240, 243, 247));

        window.draw(row);

        if (index < cartCount)
        {
            const CartItem& item = cart[index];

            drawText(to_string(index + 1), TBL_X + 10, y + 5, 16, COL_TEXT);
            drawText(fitText(item.name, 28), TBL_X + 45, y + 5, 16, COL_TITLE);
            drawTextRight(money(item.price), TBL_X + 420, y + 5, 16, COL_TEXT);
            drawTextRight(to_string(item.quantity), TBL_X + 495, y + 5, 16, COL_TEXT);
            drawTextRight(money(item.price * item.quantity),
                TBL_X + 615, y + 5, 16, COL_TITLE);
        }
        else if (cartCount == 0 && r == 0)
        {
            drawText("No products added yet.", TBL_X + 45, y + 5, 16,
                sf::Color(130, 140, 155));
        }
    }

    float hintY = TBL_Y + HDR_H + TBL_ROWS * ROW_H + 4;

    if (cartCount > TBL_ROWS)
    {
        int last = cartScroll + TBL_ROWS;

        if (last > cartCount)
            last = cartCount;

        drawText(
            "Rows " + to_string(cartScroll + 1) + "-" + to_string(last) +
            " of " + to_string(cartCount) + "  (mouse wheel to scroll)",
            TBL_X, hintY, 13, COL_MUTED
        );
    }
    else
    {
        drawText("Click a row to select it.", TBL_X, hintY, 13, COL_MUTED);
    }
}

void MainWindow::drawOrderSummary()
{
    sf::RectangleShape panel(sf::Vector2f(R_SUMMARY.w, R_SUMMARY.h));
    panel.setPosition(R_SUMMARY.x, R_SUMMARY.y);
    panel.setFillColor(sf::Color::White);
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color(200, 208, 220));
    window.draw(panel);

    float x = R_SUMMARY.x + 18;
    float y = R_SUMMARY.y + 15;

    drawText("Order Summary", x, y, 21, COL_TITLE);

    drawText("Customer", x, y + 50, 14, COL_MUTED);
    drawText(
        trimText(customerName).empty() ? "-" : fitText(trimText(customerName), 22),
        x, y + 70, 17, COL_TEXT
    );

    drawText("Products (lines)", x, y + 110, 14, COL_MUTED);
    drawText(to_string(cartCount), x, y + 130, 17, COL_TEXT);

    drawText("Total units", x, y + 165, 14, COL_MUTED);
    drawText(to_string(cartUnits()), x, y + 185, 17, COL_TEXT);

    drawText("Total", x, y + 220, 14, COL_MUTED);
    drawText("Rs " + money(cartTotal()), x, y + 238, 26, COL_GREEN);
}

void MainWindow::drawOrderPage()
{
    drawText("New Order", 250, 30, 32, COL_TITLE);

    // Customer details
    drawText("Customer Name *", 250, 85, 15, COL_MUTED);
    drawField(R_NAME, customerName, activeField == FIELD_NAME);

    drawText("Contact (optional)", 600, 85, 15, COL_MUTED);
    drawField(R_CONTACT, customerContact, activeField == FIELD_CONTACT);

    // Product entry
    drawText("Product Code (6 digits)", 250, 170, 15, COL_MUTED);
    drawField(R_CODE, orderCode, activeField == FIELD_CODE);

    drawText("Quantity", 480, 170, 15, COL_MUTED);
    drawField(R_QTY, orderQty, activeField == FIELD_QTY);

    drawButton(R_SCAN_IMG, "Scan Image", COL_BLUE);
    drawButton(R_SCAN_CAM, "Scan Camera", COL_BLUE);
    drawButton(R_DESC, showDescription ? "Hide Description" : "Description", COL_GREY);
    drawButton(R_ADD, "Add Product");

    drawProductPanel();

    drawCartTable();

    drawButton(R_REMOVE, "Remove Selected", COL_DANGER);
    drawButton(R_UPDATE, "Update Qty", COL_BLUE);

    drawOrderSummary();
    drawButton(R_BILL, "Generate Bill");
    drawButton(R_CLEAR, "Clear Order", COL_DANGER);

    drawMessage(250, 705, 16);
    drawBackButton();
}

// ----------------------------------------------------------------------
// Payment page
// ----------------------------------------------------------------------

void MainWindow::drawPayment()
{
    drawText("Payment", 250, 30, 32, COL_TITLE);

    // Order information
    sf::RectangleShape panel(sf::Vector2f(630, 90));
    panel.setPosition(250, 90);
    panel.setFillColor(sf::Color::White);
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color(200, 208, 220));
    window.draw(panel);

    drawText("Customer: " + fitText(trimText(customerName), 30),
        268, 102, 18, COL_TITLE);
    drawText(
        "Products: " + to_string(cartCount) +
        "     Units: " + to_string(cartUnits()),
        268, 135, 16, COL_TEXT
    );

    drawText("Amount due", 700, 100, 14, COL_MUTED);
    drawTextRight("Rs " + money(cartTotal()), 865, 125, 28, COL_GREEN);

    // Payment method
    drawText("Payment method", 250, 190, 16, COL_TEXT);

    drawButton(R_CASH, "Cash", payMode == "cash" ? COL_GREEN : COL_GREY);
    drawButton(R_CARD, "Card", payMode == "card" ? COL_GREEN : COL_GREY);
    drawButton(R_WALLET, "Wallet", payMode == "wallet" ? COL_GREEN : COL_GREY);

    if (payMode == "cash")
    {
        drawText("Amount received from customer (Rs)", 250, 280, 15, COL_MUTED);
        drawField(R_PAYFIELD, payTendered, activeField == FIELD_TENDERED);

        double received = 0.0;

        if (parseMoney(payTendered, received))
        {
            double change = received - cartTotal();

            if (change >= -0.005)
            {
                drawText("Change to return: Rs " + money(change < 0 ? 0 : change),
                    250, 365, 18, COL_GREEN);
            }
            else
            {
                drawText("Short by Rs " + money(-change), 250, 365, 18, COL_RED);
            }
        }
    }
    else
    {
        drawText("Provider / merchant name (e.g. Visa, JazzCash)",
            250, 280, 15, COL_MUTED);
        drawField(R_PAYFIELD, payMerchant, activeField == FIELD_MERCHANT);

        drawText("The full amount is charged to the " + payMode + ".",
            250, 365, 15, COL_MUTED);
    }

    drawButton(R_CONFIRM, "Confirm Payment");
    drawButton(R_BACKORDER, "Back to Order", COL_GREY);

    drawMessage(250, 505, 17);
    drawBackButton();
}

// ----------------------------------------------------------------------
// Final bill
// ----------------------------------------------------------------------

void MainWindow::drawBill()
{
    // Receipt paper
    sf::RectangleShape paper(sf::Vector2f(R_RECEIPT.w, R_RECEIPT.h));
    paper.setPosition(R_RECEIPT.x, R_RECEIPT.y);
    paper.setFillColor(sf::Color::White);
    paper.setOutlineThickness(1);
    paper.setOutlineColor(sf::Color(200, 208, 220));
    window.draw(paper);

    float left = R_RECEIPT.x + 22;
    float right = R_RECEIPT.x + R_RECEIPT.w - 22;
    float center = R_RECEIPT.x + R_RECEIPT.w / 2;

    drawTextCentered("ISE BARCODE STORE", center, 40, 24, COL_TITLE);
    drawTextCentered("Sales Receipt", center, 75, 15, COL_MUTED);

    drawText("Order ID: " + billOrderId, left, 105, 16, COL_TITLE);
    drawText("Date: " + billDate + "     Time: " + billTime, left, 128, 16, COL_TEXT);
    drawText("Customer: " + fitText(billName, 40), left, 151, 16, COL_TEXT);
    drawText("Contact: " + (billContact.empty() ? string("-") : billContact),
        left, 174, 16, COL_TEXT);

    // Column header
    sf::RectangleShape header(sf::Vector2f(R_RECEIPT.w - 30, 26));
    header.setPosition(R_RECEIPT.x + 15, 203);
    header.setFillColor(sf::Color(24, 35, 55));
    window.draw(header);

    drawText("Item", left, 207, 15, sf::Color::White);
    drawTextRight("Price", right - 150, 207, 15, sf::Color::White);
    drawTextRight("Qty", right - 80, 207, 15, sf::Color::White);
    drawTextRight("Total", right, 207, 15, sf::Color::White);

    // Item rows
    for (int r = 0; r < BILL_ROWS; r++)
    {
        int index = billScroll + r;

        if (index >= billItemCount)
            break;

        const CartItem& item = billItems[index];
        float y = BILL_ROWS_Y + r * BILL_ROW_H;

        drawText(fitText(item.name, 30), left, y, 15, COL_TITLE);
        drawTextRight(money(item.price), right - 150, y, 15, COL_TEXT);
        drawTextRight(to_string(item.quantity), right - 80, y, 15, COL_TEXT);
        drawTextRight(money(item.price * item.quantity), right, y, 15, COL_TITLE);
    }

    if (billItemCount > BILL_ROWS)
    {
        drawText(
            "Showing " + to_string(billScroll + 1) + "-" +
            to_string(min(billScroll + BILL_ROWS, billItemCount)) +
            " of " + to_string(billItemCount) + " items (mouse wheel)",
            left, 521, 13, COL_MUTED
        );
    }

    // Divider
    sf::RectangleShape line(sf::Vector2f(R_RECEIPT.w - 44, 2));
    line.setPosition(left, 545);
    line.setFillColor(sf::Color(180, 190, 205));
    window.draw(line);

    // Totals and payment
    drawText("TOTAL", left, 556, 22, COL_TITLE);
    drawTextRight("Rs " + money(billTotal), right, 556, 22, COL_TITLE);

    drawText("Payment: " + modeLabel(billMode), left, 595, 16, COL_TEXT);
    drawTextRight("Status: PAID", right, 595, 16, COL_GREEN);

    if (billMode != "cash")
        drawText("Provider: " + fitText(billMerchant, 35), left, 620, 16, COL_TEXT);
    else
        drawText("Received: Rs " + money(billReceived) +
            "     Change: Rs " + money(billChange), left, 620, 16, COL_TEXT);

    drawText("Thank you for shopping with us!", left, 650, 14, COL_MUTED);

    // Right-hand column
    drawText("Payment successful", 890, 40, 22, COL_GREEN);
    drawText("Order " + billOrderId + " saved.", 890, 72, 15, COL_TEXT);

    drawButton(R_SAVEBILL, "Save Bill as Text File", COL_BLUE);
    drawButton(R_NEWORDER, "Start New Order");

    if (!resultMessage.empty())
    {
        drawText(
            wrapText(resultMessage, 32, 4),
            890, 255, 15, isSuccess ? COL_GREEN : COL_RED
        );
    }

    drawBackButton();
}

void MainWindow::draw()
{
    window.clear(sf::Color(245, 247, 250));

    drawSidebar();

    switch (currentPage)
    {
    case PAGE_DASHBOARD: drawDashboard(); break;
    case PAGE_GENERATE:  drawGenerator(); break;
    case PAGE_IMAGE:     drawImageScanner(); break;
    case PAGE_CAMERA:    drawCameraScanner(); break;
    case PAGE_ORDER:     drawOrderPage(); break;
    case PAGE_PAYMENT:   drawPayment(); break;
    case PAGE_BILL:      drawBill(); break;
    }

    window.display();
}

// ======================================================================
// Input handling
// ======================================================================

void MainWindow::handleClick(float x, float y)
{
    // Back to Dashboard button on all pages except the dashboard
    if (currentPage != PAGE_DASHBOARD && hit(R_BACK, x, y))
    {
        currentPage = PAGE_DASHBOARD;
        activeField = FIELD_NONE;
        setMessage("", false);
        return;
    }

    // Sidebar navigation
    if (x < 220)
    {
        for (int i = 0; i < SIDE_COUNT; i++)
        {
            if (y >= SIDE_Y[i] - 14 && y < SIDE_Y[i] + 41)
            {
                currentPage = SIDE_PAGE[i];
                activeField = FIELD_NONE;
                setMessage("", false);
                return;
            }
        }

        return;
    }

    switch (currentPage)
    {
    case PAGE_DASHBOARD: clickDashboard(x, y); break;
    case PAGE_GENERATE:  clickGenerator(x, y); break;
    case PAGE_IMAGE:     clickImageScanner(x, y); break;
    case PAGE_CAMERA:    clickCameraScanner(x, y); break;
    case PAGE_ORDER:     clickOrder(x, y); break;
    case PAGE_PAYMENT:   clickPayment(x, y); break;
    case PAGE_BILL:      clickBill(x, y); break;
    }
}

void MainWindow::clickDashboard(float x, float y)
{
    if (hit(R_DASH_ORDER, x, y))
        currentPage = PAGE_ORDER;

    else if (hit(R_DASH_GEN, x, y))
        currentPage = PAGE_GENERATE;

    else if (hit(R_DASH_IMG, x, y))
        currentPage = PAGE_IMAGE;

    else if (hit(R_DASH_CAM, x, y))
        currentPage = PAGE_CAMERA;
}

void MainWindow::clickGenerator(float x, float y)
{
    // Generate barcode
    if (hit(R_GEN_GO, x, y))
    {
        generateBarcode();
    }

    // Start a completely new barcode generation
    else if (hit(R_GEN_NEW, x, y))
    {
        inputCode = "";
        generatedImage = "";
        setMessage("", false);
        previewLoaded = false;
    }
}

void MainWindow::clickImageScanner(float x, float y)
{
    // Choose an image
    if (hit(R_IMG_CHOOSE, x, y))
    {
        selectedImage = openImageDialog(window.getSystemHandle());

        if (selectedImage.empty())
            setMessage("No image selected.", false);
        else
            setMessage("Image selected.", false);
    }

    // Scan selected image
    else if (hit(R_IMG_SCAN, x, y))
    {
        scanImage();
    }

    // Start a new image scanning operation
    else if (hit(R_IMG_NEW, x, y))
    {
        selectedImage = "";
        setMessage("", false);
    }
}

void MainWindow::clickCameraScanner(float x, float y)
{
    // Start camera or scan another barcode
    if (hit(R_CAM_START, x, y))
    {
        setMessage("", false);
        scanCamera();
    }

    // Clear previous camera result
    else if (hit(R_CAM_CLEAR, x, y))
    {
        setMessage("", false);
    }
}

void MainWindow::clickOrder(float x, float y)
{
    activeField = FIELD_NONE;

    if (hit(R_NAME, x, y))
        activeField = FIELD_NAME;

    else if (hit(R_CONTACT, x, y))
        activeField = FIELD_CONTACT;

    else if (hit(R_CODE, x, y))
        activeField = FIELD_CODE;

    else if (hit(R_QTY, x, y))
        activeField = FIELD_QTY;

    else if (hit(R_SCAN_IMG, x, y))
        scanImageIntoOrder();

    else if (hit(R_SCAN_CAM, x, y))
        scanCameraIntoOrder();

    else if (hit(R_DESC, x, y))
    {
        if (!productFound)
            setMessage("Look up a product first (type or scan its code).", false);
        else
        {
            showDescription = !showDescription;
            setMessage("", false);
        }
    }

    else if (hit(R_ADD, x, y))
        addProductToCart();

    else if (hit(R_REMOVE, x, y))
        removeSelectedProduct();

    else if (hit(R_UPDATE, x, y))
        updateSelectedQuantity();

    else if (hit(R_CLEAR, x, y))
    {
        clearOrderForm();
        setMessage("Order cleared.", true);
    }

    else if (hit(R_BILL, x, y))
        goToPayment();

    // Select a row of the order table
    else if (x >= TBL_X && x <= TBL_X + TBL_W &&
        y >= TBL_Y + HDR_H && y < TBL_Y + HDR_H + TBL_ROWS * ROW_H)
    {
        int row = cartScroll + (int)((y - TBL_Y - HDR_H) / ROW_H);

        if (row >= 0 && row < cartCount)
        {
            selectedRow = row;
            orderQty = to_string(cart[row].quantity);
            setMessage("", false);
        }
    }
}

void MainWindow::clickPayment(float x, float y)
{
    activeField = FIELD_NONE;

    if (hit(R_CASH, x, y))
    {
        payMode = "cash";
        activeField = FIELD_TENDERED;
        setMessage("", false);
    }

    else if (hit(R_CARD, x, y))
    {
        payMode = "card";
        activeField = FIELD_MERCHANT;
        setMessage("", false);
    }

    else if (hit(R_WALLET, x, y))
    {
        payMode = "wallet";
        activeField = FIELD_MERCHANT;
        setMessage("", false);
    }

    else if (hit(R_PAYFIELD, x, y))
        activeField = (payMode == "cash") ? FIELD_TENDERED : FIELD_MERCHANT;

    else if (hit(R_CONFIRM, x, y))
        confirmPayment();

    else if (hit(R_BACKORDER, x, y))
    {
        currentPage = PAGE_ORDER;
        setMessage("", false);
    }
}

void MainWindow::clickBill(float x, float y)
{
    if (hit(R_SAVEBILL, x, y))
        saveBillToFile();

    else if (hit(R_NEWORDER, x, y))
    {
        clearOrderForm();
        currentPage = PAGE_ORDER;
        activeField = FIELD_NAME;
        setMessage("", false);
    }
}

void MainWindow::handleScroll(float delta, float x, float y)
{
    int step = (delta > 0) ? -1 : 1;

    if (currentPage == PAGE_ORDER &&
        x >= TBL_X && x <= TBL_X + TBL_W &&
        y >= TBL_Y && y <= TBL_Y + HDR_H + TBL_ROWS * ROW_H)
    {
        cartScroll += step;
        clampCartScroll();
    }

    else if (currentPage == PAGE_BILL &&
        x >= R_RECEIPT.x && x <= R_RECEIPT.x + R_RECEIPT.w &&
        y >= BILL_ROWS_Y && y <= BILL_ROWS_Y + BILL_ROWS * BILL_ROW_H)
    {
        billScroll += step;
        clampBillScroll();
    }
}

void MainWindow::clampCartScroll()
{
    int maxScroll = cartCount - TBL_ROWS;

    if (maxScroll < 0)
        maxScroll = 0;

    if (cartScroll > maxScroll)
        cartScroll = maxScroll;

    if (cartScroll < 0)
        cartScroll = 0;
}

void MainWindow::clampBillScroll()
{
    int maxScroll = billItemCount - BILL_ROWS;

    if (maxScroll < 0)
        maxScroll = 0;

    if (billScroll > maxScroll)
        billScroll = maxScroll;

    if (billScroll < 0)
        billScroll = 0;
}

// Text box that is currently receiving typed characters
string* MainWindow::activeText()
{
    switch (activeField)
    {
    case FIELD_NAME:     return &customerName;
    case FIELD_CONTACT:  return &customerContact;
    case FIELD_CODE:     return &orderCode;
    case FIELD_QTY:      return &orderQty;
    case FIELD_MERCHANT: return &payMerchant;
    case FIELD_TENDERED: return &payTendered;
    default:             return nullptr;
    }
}

// Tab key: move to the next box on the order page
void MainWindow::cycleField()
{
    if (currentPage != PAGE_ORDER)
        return;

    if (activeField == FIELD_NAME)
        activeField = FIELD_CONTACT;
    else if (activeField == FIELD_CONTACT)
        activeField = FIELD_CODE;
    else if (activeField == FIELD_CODE)
        activeField = FIELD_QTY;
    else
        activeField = FIELD_NAME;
}

void MainWindow::handleTyping(sf::Uint32 character)
{
    // Generator page: the original six-digit input
    if (currentPage == PAGE_GENERATE)
    {
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

        return;
    }

    if (currentPage != PAGE_ORDER && currentPage != PAGE_PAYMENT)
        return;

    // Tab
    if (character == 9)
    {
        cycleField();
        return;
    }

    // Enter
    if (character == 13 || character == 10)
    {
        if (currentPage == PAGE_PAYMENT)
        {
            confirmPayment();
        }
        else if (activeField == FIELD_NAME)
        {
            activeField = FIELD_CONTACT;
        }
        else if (activeField == FIELD_CONTACT)
        {
            activeField = FIELD_CODE;
        }
        else if (activeField == FIELD_CODE || activeField == FIELD_QTY)
        {
            addProductToCart();
        }

        return;
    }

    string* target = activeText();

    if (target == nullptr)
        return;

    // Backspace
    if (character == 8)
    {
        if (!target->empty())
            target->pop_back();

        if (activeField == FIELD_CODE)
            refreshProductLookup();

        return;
    }

    if (character < 32 || character > 126)
        return;

    char c = (char)character;
    bool isDigit = (c >= '0' && c <= '9');

    // Characters that would break the space/quote based data files
    bool unsafe = (c == ',' || c == '"' || c == '#');

    switch (activeField)
    {
    case FIELD_NAME:
        if (!unsafe && target->length() < 30)
            *target += c;
        break;

    case FIELD_CONTACT:
        if ((isDigit || c == '+' || c == '-') && target->length() < 16)
            *target += c;
        break;

    case FIELD_CODE:
        if (isDigit && target->length() < 6)
        {
            *target += c;
            refreshProductLookup();
        }
        break;

    case FIELD_QTY:
        if (isDigit && target->length() < 3)
            *target += c;
        break;

    case FIELD_MERCHANT:
        if (!unsafe && target->length() < 25)
            *target += c;
        break;

    case FIELD_TENDERED:
        if ((isDigit || c == '.') && target->length() < 10)
            *target += c;
        break;

    default:
        break;
    }
}

// ======================================================================
// Generator and scanner operations (existing features)
// ======================================================================

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
        setMessage("Error: Enter exactly six digits.", false);
        return;
    }

    if (inputCode[0] == '0')
    {
        setMessage("Error: The first digit cannot be zero.", false);
        return;
    }

    // Automatically create the barcodes folder
    BOOL folderCreated = CreateDirectoryA("barcodes", NULL);

    if (!folderCreated && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        setMessage("Could not create the barcodes folder.", false);
        return;
    }

    // Automatically use the barcode number as the filename
    string outputPath = "barcodes\\" + inputCode + ".bmp";

    BarcodeGenerator generator;

    if (generator.generateBarcode(inputCode, outputPath))
    {
        generatedImage = outputPath;
        loadPreview(generatedImage);

        setMessage("Barcode saved automatically as " + outputPath, true);
    }
    else
    {
        setMessage("Barcode generation failed.", false);
    }
}


void MainWindow::scanImage()
{
    if (selectedImage.empty())
    {
        setMessage("Please choose an image first.", false);
        return;
    }

    string code;
    string error;

    if (readBarcodeFromImage(selectedImage, code, error))
        setMessage("Scanned code: " + code, true);
    else
        setMessage("Scan failed: " + error, false);
}

// Runs the camera module; returns true and the code on success
bool MainWindow::runCameraScan(string& code, string& error)
{
    code = "";
    error = "";

    try
    {
        CameraScanner scanner;

        // The camera module opens and controls its own OpenCV window.
        code = scanner.scanUsingCamera();

        if (code.empty())
        {
            error = "No valid barcode was scanned.";
            return false;
        }

        return true;
    }
    catch (const exception& e)
    {
        error = "Camera error: " + string(e.what());
    }
    catch (...)
    {
        error = "An unknown camera error occurred.";
    }

    return false;
}

void MainWindow::scanCamera()
{
    string code;
    string error;

    if (runCameraScan(code, error))
        setMessage("Scanned code: " + code, true);
    else
        setMessage(error, false);
}

// ======================================================================
// Order operations
// ======================================================================

double MainWindow::cartTotal() const
{
    double total = 0.0;

    for (int i = 0; i < cartCount; i++)
        total += cart[i].price * cart[i].quantity;

    return total;
}

int MainWindow::cartUnits() const
{
    int units = 0;

    for (int i = 0; i < cartCount; i++)
        units += cart[i].quantity;

    return units;
}

// Looks up the product for the 6-digit code in the code box
void MainWindow::refreshProductLookup()
{
    productFound = false;
    showDescription = false;

    if (orderCode.length() != 6)
        return;

    const Product* product = productManager.findProductByBarcode(orderCode);

    if (product != nullptr)
    {
        foundProduct.productId = product->getProductId();
        foundProduct.barcode = product->getBarcode();
        foundProduct.name = product->getProductName();
        foundProduct.description = product->getDescription();
        foundProduct.price = product->getPrice();
        productFound = true;
    }
}

// Scan a barcode image and put the code into the order form
void MainWindow::scanImageIntoOrder()
{
    string path = openImageDialog(window.getSystemHandle());

    if (path.empty())
    {
        setMessage("No image selected.", false);
        return;
    }

    string code;
    string error;

    if (!readBarcodeFromImage(path, code, error))
    {
        setMessage("Scan failed: " + error, false);
        return;
    }

    orderCode = code;
    refreshProductLookup();

    if (productFound)
    {
        activeField = FIELD_QTY;
        setMessage("Scanned " + code + ". Check the quantity and press Add Product.", true);
    }
    else
    {
        setMessage("Scanned code " + code + " but no product matches it.", false);
    }
}

// Scan with the camera and put the code into the order form
void MainWindow::scanCameraIntoOrder()
{
    string code;
    string error;

    if (!runCameraScan(code, error))
    {
        setMessage(error, false);
        return;
    }

    orderCode = code;
    refreshProductLookup();

    if (productFound)
    {
        activeField = FIELD_QTY;
        setMessage("Scanned " + code + ". Check the quantity and press Add Product.", true);
    }
    else
    {
        setMessage("Scanned code " + code + " but no product matches it.", false);
    }
}

void MainWindow::addProductToCart()
{
    if (!productFound)
    {
        setMessage("Enter or scan a valid product code first.", false);
        return;
    }

    int quantity = 0;

    if (!parsePositiveInt(orderQty, quantity))
    {
        setMessage("Enter a quantity of 1 or more.", false);
        return;
    }

    // When an Inventory.txt file is present, prevent adding more than available stock.
    int existing = -1;

    for (int i = 0; i < cartCount; i++)
    {
        if (cart[i].barcode == foundProduct.barcode)
        {
            existing = i;
            break;
        }
    }

    if (inventoryEnabled)
    {
        int alreadyInCart = (existing == -1) ? 0 : cart[existing].quantity;
        int available = productManager.getStockByBarcode(foundProduct.barcode);
        if (available < alreadyInCart + quantity)
        {
            setMessage("Insufficient stock. Available: " + to_string(available) +
                ", already in order: " + to_string(alreadyInCart) + ".", false);
            return;
        }
    }

    if (existing != -1)
    {
        if (cart[existing].quantity + quantity > 999)
        {
            setMessage("A single line cannot exceed 999 units.", false);
            return;
        }

        cart[existing].quantity += quantity;
        selectedRow = existing;
    }
    else
    {
        if (cartCount >= MAX_DETAILS_PER_ORDER)
        {
            setMessage("The order is full (maximum " +
                to_string(MAX_DETAILS_PER_ORDER) + " different products).", false);
            return;
        }

        cart[cartCount].productId = foundProduct.productId;
        cart[cartCount].barcode = foundProduct.barcode;
        cart[cartCount].name = foundProduct.name;
        cart[cartCount].price = foundProduct.price;
        cart[cartCount].quantity = quantity;
        selectedRow = cartCount;
        cartCount++;
    }

    setMessage("Added " + to_string(quantity) + " x " + foundProduct.name + ".", true);

    // Get ready for the next product
    orderCode = "";
    orderQty = "1";
    productFound = false;
    showDescription = false;
    activeField = FIELD_CODE;

    // Show the line that was just added
    if (selectedRow >= cartScroll + TBL_ROWS)
        cartScroll = selectedRow - TBL_ROWS + 1;

    clampCartScroll();
}

void MainWindow::removeSelectedProduct()
{
    if (selectedRow < 0 || selectedRow >= cartCount)
    {
        setMessage("Click a product in the order first.", false);
        return;
    }

    string name = cart[selectedRow].name;

    for (int i = selectedRow; i < cartCount - 1; i++)
        cart[i] = cart[i + 1];

    cartCount--;
    selectedRow = -1;
    clampCartScroll();

    setMessage("Removed " + name + " from the order.", true);
}

void MainWindow::updateSelectedQuantity()
{
    if (selectedRow < 0 || selectedRow >= cartCount)
    {
        setMessage("Click a product in the order first.", false);
        return;
    }

    int quantity = 0;

    if (!parsePositiveInt(orderQty, quantity) || quantity > 999)
    {
        setMessage("Enter a quantity between 1 and 999.", false);
        return;
    }

    if (inventoryEnabled)
    {
        int available = productManager.getStockByBarcode(cart[selectedRow].barcode);
        if (quantity > available)
        {
            setMessage("Insufficient stock. Available: " + to_string(available) + ".", false);
            return;
        }
    }

    cart[selectedRow].quantity = quantity;
    setMessage("Quantity updated.", true);
}

// Clears customer details, the product entry and the cart
void MainWindow::clearOrderForm()
{
    customerName = "";
    customerContact = "";
    orderCode = "";
    orderQty = "1";

    productFound = false;
    showDescription = false;

    cartCount = 0;
    selectedRow = -1;
    cartScroll = 0;

    activeField = FIELD_NONE;
}

void MainWindow::goToPayment()
{
    if (trimText(customerName).empty())
    {
        setMessage("Enter the customer name before generating the bill.", false);
        activeField = FIELD_NAME;
        return;
    }

    if (cartCount == 0)
    {
        setMessage("Add at least one product to the order first.", false);
        return;
    }

    payMode = "cash";
    payMerchant = "";
    payTendered = "";

    currentPage = PAGE_PAYMENT;
    activeField = FIELD_TENDERED;
    setMessage("", false);
}

void MainWindow::confirmPayment()
{
    if (inventoryEnabled)
    {
        for (int i = 0; i < cartCount; i++)
        {
            int available = productManager.getStockByBarcode(cart[i].barcode);
            if (available < cart[i].quantity)
            {
                setMessage("Insufficient stock for " + cart[i].name +
                    ". Available: " + to_string(available) + ".", false);
                return;
            }
        }
    }

    double total = cartTotal();
    double received = total;
    double change = 0.0;
    string merchant = "None";

    if (payMode == "cash")
    {
        if (!parseMoney(payTendered, received))
        {
            setMessage("Enter the amount received from the customer.", false);
            return;
        }

        if (received + 0.005 < total)
        {
            setMessage("Amount received (Rs " + money(received) +
                ") is less than the total (Rs " + money(total) + ").", false);
            return;
        }

        change = received - total;
    }
    else
    {
        merchant = trimText(payMerchant);

        if (merchant.empty())
        {
            setMessage("Enter the card or wallet provider name.", false);
            return;
        }
    }

    // Create and save the order through the order module
    string date;
    string timeText;
    currentDateTime(date, timeText);

    string orderId = orderManager->generateOrderId(date);

    bool created = orderManager->createOrder(
        orderId, date, timeText,
        trimText(customerName), trimText(customerContact),
        "in-progress", total, payMode, merchant
    );

    if (!created)
    {
        setMessage("The order could not be saved (storage full or invalid data).", false);
        return;
    }

    bool detailsSaved = true;

    for (int i = 0; i < cartCount; i++)
    {
        if (!orderManager->addProductToOrder(
            orderId, cart[i].productId, cart[i].barcode, cart[i].quantity))
        {
            detailsSaved = false;
            break;
        }
    }

    if (!detailsSaved)
    {
        orderManager->updateOrderStatus(orderId, "on-hold");
        setMessage(
            "Could not save all product lines. Order placed on hold.",
            false
        );
        return;
    }

    // Deduct stock only when a real Inventory.txt was loaded.
    // The ProductManager validates the full order before changing any quantities.
    bool stockDeducted = false;
    if (inventoryEnabled)
    {
        Order* savedOrder = orderManager->findOrder(orderId);
        if (savedOrder == nullptr || !productManager.processOrderDeduction(*savedOrder))
        {
            orderManager->updateOrderStatus(orderId, "on-hold");

            setMessage(
                "Stock could not be deducted. Order placed on hold.",
                false
            );
            return;
        }
        stockDeducted = true;
    }

    // An order is marked paid only after its complete product list is saved
    // and (when inventory is enabled) its stock has been deducted.
    if (!orderManager->updateOrderStatus(orderId, "paid"))
    {
        if (stockDeducted)
        {
            Order* savedOrder = orderManager->findOrder(orderId);
            if (savedOrder != nullptr)
                productManager.restoreOrderStock(*savedOrder);
        }
        orderManager->updateOrderStatus(orderId, "on-hold");

        setMessage(
            "The order could not be finalized as paid. Order placed on hold.",
            false
        );
        return;
    }

    // Keep a copy of everything the bill needs
    billOrderId = orderId;
    billDate = date;
    billTime = timeText;
    billName = trimText(customerName);
    billContact = trimText(customerContact);
    billMode = payMode;
    billMerchant = merchant;
    billTotal = total;
    billReceived = received;
    billChange = change;
    billItemCount = cartCount;
    billScroll = 0;

    for (int i = 0; i < cartCount; i++)
        billItems[i] = cart[i];

    // The order is finished, so the form is cleared for the next customer
    clearOrderForm();

    currentPage = PAGE_BILL;
    activeField = FIELD_NONE;

    setMessage("", false);
}

string MainWindow::buildBillText() const
{
    ostringstream out;
    const string rule = "------------------------------------------------------\n";

    out << "ISE BARCODE STORE\n";
    out << "Sales Receipt\n";
    out << rule;
    out << "Order ID : " << billOrderId << "\n";
    out << "Date     : " << billDate << "   Time: " << billTime << "\n";
    out << "Customer : " << billName << "\n";
    out << "Contact  : " << (billContact.empty() ? string("-") : billContact) << "\n";
    out << rule;

    out << left << setw(26) << "Item"
        << right << setw(10) << "Price"
        << setw(6) << "Qty"
        << setw(12) << "Total" << "\n";

    for (int i = 0; i < billItemCount; i++)
    {
        out << left << setw(26) << fitText(billItems[i].name, 25)
            << right << setw(10) << money(billItems[i].price)
            << setw(6) << billItems[i].quantity
            << setw(12) << money(billItems[i].price * billItems[i].quantity)
            << "\n";
    }

    out << rule;
    out << "TOTAL    : Rs " << money(billTotal) << "\n";
    out << "Payment  : " << modeLabel(billMode) << "\n";

    if (billMode == "cash")
    {
        out << "Received : Rs " << money(billReceived) << "\n";
        out << "Change   : Rs " << money(billChange) << "\n";
    }
    else
    {
        out << "Provider : " << billMerchant << "\n";
    }

    out << "Status   : PAID\n";
    out << rule;
    out << "Thank you for shopping with us!\n";

    return out.str();
}

void MainWindow::saveBillToFile()
{
    string path = "Bill_" + billOrderId + ".txt";

    ofstream file(path.c_str());

    if (!file.is_open())
    {
        setMessage("Could not create " + path + ".", false);
        return;
    }

    file << buildBillText();
    file.close();

    setMessage("Bill saved as " + path + " in the program folder.", true);
}
