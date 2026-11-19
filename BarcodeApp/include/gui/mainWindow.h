#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

#include "ProductCatalog.h"
#include "../product/ProductManager.h"
#include "../order/OrderManager.h"

// Simple rectangle used for drawing and click detection
struct UiRect
{
    float x, y, w, h;
};

// One line of the order that is currently being built
struct CartItem
{
    std::string productId;
    std::string barcode;
    std::string name;
    double price = 0.0;
    int quantity = 0;
    double tax = 0.0;
    double discount = 0.0;
};

enum PageId
{
    PAGE_DASHBOARD = 0,
    PAGE_GENERATE = 1,
    PAGE_IMAGE = 2,
    PAGE_CAMERA = 3,
    PAGE_ORDER = 4,
    PAGE_PAYMENT = 5,
    PAGE_BILL = 6
};

// Text box that currently receives the keyboard input
enum FieldId
{
    FIELD_NONE = 0,
    FIELD_NAME,
    FIELD_CONTACT,
    FIELD_CODE,
    FIELD_QTY,
    FIELD_MERCHANT,
    FIELD_TENDERED
};

class MainWindow
{
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock caretClock;

    sf::Texture previewTexture;
    sf::Sprite previewSprite;

    // Modules
    std::unique_ptr<OrderManager> orderManager;
    ProductManager productManager;
    bool inventoryEnabled;

    int currentPage;
    FieldId activeField;

    // Generator / scanner pages
    std::string inputCode;
    std::string selectedImage;
    std::string resultMessage;
    std::string generatedImage;

    bool fontLoaded;
    bool isSuccess;
    bool previewLoaded;

    // Order page
    std::string customerName;
    std::string customerContact;
    std::string orderCode;
    std::string orderQty;

    bool productFound;
    bool showDescription;
    ProductInfo foundProduct;

    CartItem cart[MAX_DETAILS_PER_ORDER];
    int cartCount;
    int selectedRow;
    int cartScroll;

    // Payment page
    std::string payMode;       // "cash", "card" or "wallet"
    std::string payMerchant;
    std::string payTendered;
    bool walletMenuOpen;       // wallet dropdown is expanded

    // Final bill (a copy, so a new order can be started)
    CartItem billItems[MAX_DETAILS_PER_ORDER];
    int billItemCount;
    int billScroll;
    std::string billOrderId;
    std::string billDate;
    std::string billTime;
    std::string billName;
    std::string billContact;
    std::string billMode;
    std::string billMerchant;
    double billSubtotal;
    double billTax;
    double billDiscount;
    double billTotal;
    double billReceived;
    double billChange;

    // Events
    void handleEvents();
    void handleClick(float x, float y);
    void handleTyping(sf::Uint32 character);
    void handleScroll(float delta, float x, float y);

    void clickDashboard(float x, float y);
    void clickGenerator(float x, float y);
    void clickImageScanner(float x, float y);
    void clickCameraScanner(float x, float y);
    void clickOrder(float x, float y);
    void clickPayment(float x, float y);
    void clickBill(float x, float y);

    // Drawing helpers
    void draw();

    void drawText(
        const std::string& text,
        float x, float y,
        int size,
        sf::Color color
    );

    void drawTextRight(
        const std::string& text,
        float xRight, float y,
        int size,
        sf::Color color
    );

    void drawTextCentered(
        const std::string& text,
        float xCenter, float y,
        int size,
        sf::Color color
    );

    void drawButton(
        const UiRect& rect,
        const std::string& text,
        sf::Color color = sf::Color(30, 150, 105)
    );

    void drawField(const UiRect& rect, const std::string& text, bool active);
    void drawMessage(float x, float y, int size);
    void drawBackButton();

    // Pages
    void drawSidebar();
    void drawDashboard();
    void drawGenerator();
    void drawImageScanner();
    void drawCameraScanner();
    void drawOrderPage();
    void drawProductPanel();
    void drawCartTable();
    void drawOrderSummary();
    void drawPayment();
    void drawBill();

    // Existing operations
    void generateBarcode();
    void scanImage();
    void scanCamera();
    void loadPreview(const std::string& path);

    // Order operations
    bool runCameraScan(std::string& code, std::string& error);
    void scanImageIntoOrder();
    void scanCameraIntoOrder();
    void refreshProductLookup();
    void addProductToCart();
    void removeSelectedProduct();
    void updateSelectedQuantity();
    void clearOrderForm();
    void goToPayment();
    void confirmPayment();
    void saveBillToFile();
    void clampCartScroll();
    void clampBillScroll();
    void cycleField();
    void setMessage(const std::string& text, bool success);

    double cartSubtotal() const;
    double cartTotalTax() const;
    double cartTotalDiscount() const;
    double cartTotal() const;
    int cartUnits() const;
    std::string* activeText();
    std::string buildBillText() const;

public:
    MainWindow();

    void run();
};
