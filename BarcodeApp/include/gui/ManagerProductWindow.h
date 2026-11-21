#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "../product/ProductManager.h"
#include "../product/CategoryManager.h"
#include "../product/ManagerProductService.h"

// Manager-facing GUI for Requirements 1, 2 and 6.
// Hiba can construct this window from the manager role screen.
// Optional standalone launch: BarcodeApp.exe --manager
class ManagerProductWindow {
private:
    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded;

    ProductManager productManager;
    CategoryManager categoryManager;
    ManagerProductService service;

    int selectedCategoryIndex;
    int selectedProductIndex;
    int categoryScroll;
    int productScroll;
    int activeField;
    bool editingExisting;

    std::string catIdInput;
    std::string catNameInput;
    std::string catAcronymInput;

    std::string productIdInput;
    std::string acronymInput;
    std::string nameInput;
    std::string unitInput;
    std::string priceInput;
    std::string taxInput;
    std::string discountInput;
    std::string stockInput;
    std::string descriptionInput;
    std::string barcodeInput;
    std::string imagePathInput;
    std::string selectedCategoryId;

    std::string statusMessage;
    bool statusSuccess;

    void handleEvents();
    void handleClick(float x, float y);
    void handleTyping(unsigned int unicode);
    void draw();
    void drawText(const std::string& text, float x, float y, unsigned int size, sf::Color color);
    void drawButton(float x, float y, float w, float h, const std::string& text, sf::Color color);
    void drawField(float x, float y, float w, float h, const std::string& text, bool active);
    bool hit(float x, float y, float rx, float ry, float rw, float rh) const;
    void loadProductIntoForm(int index);
    void clearProductForm();
    Product buildProductFromForm() const;
    void setStatus(const std::string& message, bool success);
    std::string nextProductId() const;

public:
    ManagerProductWindow(const std::string& productFile = "Product.txt",
                         const std::string& categoryFile = "PCategory.txt",
                         const std::string& barcodeDir = "barcodes");
    void run();
};
