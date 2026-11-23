#include "../../include/gui/ManagerProductWindow.h"
#include "../../include/product/ProductFileHandler.h"
#include <sstream>
#include <iomanip>

enum ManagerField {
    MF_NONE = 0,
    MF_CAT_ID,
    MF_CAT_NAME,
    MF_CAT_ACRONYM,
    MF_PROD_ID,
    MF_ACRONYM,
    MF_NAME,
    MF_UNIT,
    MF_PRICE,
    MF_TAX,
    MF_DISCOUNT,
    MF_STOCK,
    MF_DESC
};

ManagerProductWindow::ManagerProductWindow(const std::string& productFile,
                                           const std::string& categoryFile,
                                           const std::string& barcodeDir)
    : productManager(productFile),
      categoryManager(categoryFile),
      service(productManager, categoryManager, barcodeDir)
{
    window.create(sf::VideoMode(1280, 760), "ISE Manager - Products and Categories");
    window.setFramerateLimit(60);

    fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    productManager.loadProducts();
    categoryManager.loadCategories();

    selectedCategoryIndex = -1;
    selectedProductIndex = -1;
    categoryScroll = 0;
    productScroll = 0;
    activeField = MF_NONE;
    editingExisting = false;
    unitInput = "count";
    statusSuccess = false;
}

void ManagerProductWindow::run() {
    while (window.isOpen()) {
        handleEvents();
        draw();
    }
}

void ManagerProductWindow::setStatus(const std::string& message, bool success) {
    statusMessage = message;
    statusSuccess = success;
}

bool ManagerProductWindow::hit(float x, float y, float rx, float ry, float rw, float rh) const {
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

std::string ManagerProductWindow::nextProductId() const {
    int maxNum = 0;
    for (int i = 0; i < productManager.getProductCount(); i++) {
        const Product* p = productManager.getProductAt(i);
        if (p == nullptr) {
            continue;
        }
        std::string id = p->getProductId();
        if (id.size() >= 2 && (id[0] == 'P' || id[0] == 'p')) {
            try {
                int n = std::stoi(id.substr(1));
                if (n > maxNum) {
                    maxNum = n;
                }
            } catch (...) {
            }
        }
    }
    std::ostringstream oss;
    oss << "P" << std::setw(3) << std::setfill('0') << (maxNum + 1);
    return oss.str();
}

Product ManagerProductWindow::buildProductFromForm() const {
    double price = 0.0;
    double tax = 0.0;
    double discount = 0.0;
    int stock = 0;
    try { price = std::stod(priceInput.empty() ? "0" : priceInput); } catch (...) { price = -1.0; }
    try { tax = std::stod(taxInput.empty() ? "0" : taxInput); } catch (...) { tax = -1.0; }
    try { discount = std::stod(discountInput.empty() ? "0" : discountInput); } catch (...) { discount = -1.0; }
    try { stock = std::stoi(stockInput.empty() ? "0" : stockInput); } catch (...) { stock = -1; }

    Product product(
        ProductFileHandler::trim(productIdInput),
        ProductFileHandler::trim(barcodeInput),
        ProductFileHandler::trim(acronymInput),
        ProductFileHandler::trim(nameInput),
        ProductFileHandler::trim(unitInput),
        price, tax, discount, stock,
        ProductFileHandler::trim(imagePathInput),
        ProductFileHandler::trim(descriptionInput),
        ProductFileHandler::trim(selectedCategoryId)
    );
    return product;
}

void ManagerProductWindow::clearProductForm() {
    editingExisting = false;
    selectedProductIndex = -1;
    productIdInput = nextProductId();
    acronymInput = "";
    nameInput = "";
    unitInput = "count";
    priceInput = "";
    taxInput = "0";
    discountInput = "0";
    stockInput = "0";
    descriptionInput = "";
    barcodeInput = "";
    imagePathInput = "";
}

void ManagerProductWindow::loadProductIntoForm(int index) {
    const Product* p = productManager.getProductAt(index);
    if (p == nullptr) {
        return;
    }
    editingExisting = true;
    selectedProductIndex = index;
    productIdInput = p->getProductId();
    acronymInput = p->getAcronym();
    nameInput = p->getProductName();
    unitInput = p->getMeasurementUnit();
    std::ostringstream pss, tss, dss;
    pss << std::fixed << std::setprecision(2) << p->getPrice();
    tss << std::fixed << std::setprecision(2) << p->getTax();
    dss << std::fixed << std::setprecision(2) << p->getDiscount();
    priceInput = pss.str();
    taxInput = tss.str();
    discountInput = dss.str();
    stockInput = std::to_string(p->getStockQuantity());
    descriptionInput = p->getDescription();
    barcodeInput = p->getBarcode();
    imagePathInput = p->getBarcodeImagePath();
    selectedCategoryId = p->getCategoryId();
    if (selectedCategoryId.empty()) {
        selectedCategoryId = "10";
    }
    selectedCategoryIndex = -1;
    for (int i = 0; i < categoryManager.getCategoryCount(); i++) {
        if (categoryManager.getCategoryAt(i)->getCategoryId() == selectedCategoryId) {
            selectedCategoryIndex = i;
            break;
        }
    }
}

void ManagerProductWindow::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            handleClick(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
        }
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.x < 430) {
                categoryScroll -= static_cast<int>(event.mouseWheelScroll.delta);
            } else {
                productScroll -= static_cast<int>(event.mouseWheelScroll.delta);
            }
            if (categoryScroll < 0) categoryScroll = 0;
            if (productScroll < 0) productScroll = 0;
        }
        if (event.type == sf::Event::TextEntered) {
            handleTyping(event.text.unicode);
        }
    }
}

void ManagerProductWindow::handleTyping(unsigned int unicode) {
    if (activeField == MF_NONE) {
        return;
    }
    std::string* target = nullptr;
    int maxLen = 80;
    switch (activeField) {
        case MF_CAT_ID: target = &catIdInput; maxLen = 2; break;
        case MF_CAT_NAME: target = &catNameInput; maxLen = 40; break;
        case MF_CAT_ACRONYM: target = &catAcronymInput; maxLen = 12; break;
        case MF_PROD_ID: if (editingExisting) return; target = &productIdInput; maxLen = 12; break;
        case MF_ACRONYM: target = &acronymInput; maxLen = 8; break;
        case MF_NAME: target = &nameInput; maxLen = 40; break;
        case MF_UNIT: target = &unitInput; maxLen = 10; break;
        case MF_PRICE: target = &priceInput; maxLen = 12; break;
        case MF_TAX: target = &taxInput; maxLen = 12; break;
        case MF_DISCOUNT: target = &discountInput; maxLen = 12; break;
        case MF_STOCK: target = &stockInput; maxLen = 8; break;
        case MF_DESC: target = &descriptionInput; maxLen = 80; break;
        default: return;
    }

    if (unicode == 8) {
        if (!target->empty()) {
            target->pop_back();
        }
        return;
    }
    if (unicode < 32 || unicode > 126) {
        return;
    }
    if (static_cast<int>(target->size()) >= maxLen) {
        return;
    }
    *target += static_cast<char>(unicode);
}

void ManagerProductWindow::handleClick(float x, float y) {
    activeField = MF_NONE;

    if (hit(x, y, 20, 70, 70, 28)) activeField = MF_CAT_ID;
    else if (hit(x, y, 100, 70, 180, 28)) activeField = MF_CAT_NAME;
    else if (hit(x, y, 290, 70, 80, 28)) activeField = MF_CAT_ACRONYM;
    else if (hit(x, y, 450, 70, 140, 28)) activeField = MF_PROD_ID;
    else if (hit(x, y, 600, 70, 120, 28)) activeField = MF_ACRONYM;
    else if (hit(x, y, 730, 70, 250, 28)) activeField = MF_NAME;
    else if (hit(x, y, 450, 130, 120, 28)) activeField = MF_UNIT;
    else if (hit(x, y, 580, 130, 100, 28)) activeField = MF_PRICE;
    else if (hit(x, y, 690, 130, 100, 28)) activeField = MF_TAX;
    else if (hit(x, y, 800, 130, 100, 28)) activeField = MF_DISCOUNT;
    else if (hit(x, y, 910, 130, 80, 28)) activeField = MF_STOCK;
    else if (hit(x, y, 450, 190, 530, 28)) activeField = MF_DESC;

    if (hit(x, y, 20, 108, 160, 32)) {
        std::string error = service.addCategory(catIdInput, catNameInput, catAcronymInput);
        if (error.empty()) {
            setStatus("Category saved to PCategory.txt.", true);
            catIdInput = "";
            catNameInput = "";
            catAcronymInput = "";
        } else {
            setStatus(error, false);
        }
        return;
    }

    const int catVisible = 8;
    float catListY = 160;
    int catStart = categoryScroll;
    if (catStart > categoryManager.getCategoryCount()) catStart = 0;
    for (int i = 0; i < catVisible; i++) {
        int idx = catStart + i;
        if (idx >= categoryManager.getCategoryCount()) break;
        if (hit(x, y, 20, catListY + i * 28, 390, 26)) {
            selectedCategoryIndex = idx;
            selectedCategoryId = categoryManager.getCategoryAt(idx)->getCategoryId();
            setStatus("Category " + selectedCategoryId + " selected for barcode generation.", true);
        }
    }

    const int prodVisible = 10;
    float prodListY = 360;
    int prodStart = productScroll;
    for (int i = 0; i < prodVisible; i++) {
        int idx = prodStart + i;
        if (idx >= productManager.getProductCount()) break;
        if (hit(x, y, 20, prodListY + i * 28, 1240, 26)) {
            loadProductIntoForm(idx);
            setStatus("Editing " + productIdInput + ". Change fields and click Save Changes.", true);
        }
    }

    if (hit(x, y, 450, 230, 200, 36)) {
        if (selectedCategoryId.empty()) {
            setStatus("Select a category before generating a barcode.", false);
            return;
        }
        Product draft = buildProductFromForm();
        std::string error = service.generateBarcodeForProduct(draft, selectedCategoryId);
        if (error.empty()) {
            barcodeInput = draft.getBarcode();
            imagePathInput = draft.getBarcodeImagePath();
            selectedCategoryId = draft.getCategoryId();
            setStatus("Generated barcode " + barcodeInput + " and saved " + imagePathInput, true);
        } else {
            setStatus(error, false);
        }
        return;
    }

    if (hit(x, y, 660, 230, 140, 36)) {
        unitInput = (unitInput == "count") ? "weight" : "count";
        return;
    }

    if (hit(x, y, 810, 230, 160, 36)) {
        clearProductForm();
        setStatus("New product form ready. Select a category, generate a barcode, then save.", true);
        return;
    }

    if (hit(x, y, 980, 230, 160, 36)) {
        Product product = buildProductFromForm();
        std::string error;
        if (editingExisting) {
            error = service.editExistingProduct(product.getProductId(), product);
            if (error.empty()) {
                setStatus("Product updated in Product.txt.", true);
            }
        } else {
            error = service.addNewProduct(product);
            if (error.empty()) {
                setStatus("Product added to Product.txt.", true);
                clearProductForm();
            }
        }
        if (!error.empty()) {
            setStatus(error, false);
        }
        return;
    }
}

void ManagerProductWindow::drawText(const std::string& text, float x, float y, unsigned int size, sf::Color color) {
    if (!fontLoaded) return;
    sf::Text displayedText;
    displayedText.setFont(font);
    displayedText.setString(text);
    displayedText.setCharacterSize(size);
    displayedText.setFillColor(color);
    displayedText.setPosition(x, y);
    window.draw(displayedText);
}

void ManagerProductWindow::drawButton(float x, float y, float w, float h, const std::string& text, sf::Color color) {
    sf::RectangleShape button(sf::Vector2f(w, h));
    button.setPosition(x, y);
    button.setFillColor(color);
    window.draw(button);
    if (!fontLoaded) return;
    sf::Text label;
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(16);
    label.setFillColor(sf::Color::White);
    sf::FloatRect bounds = label.getLocalBounds();
    label.setPosition(x + (w - bounds.width) / 2.0f - bounds.left, y + (h - bounds.height) / 2.0f - bounds.top);
    window.draw(label);
}

void ManagerProductWindow::drawField(float x, float y, float w, float h, const std::string& text, bool active) {
    sf::RectangleShape box(sf::Vector2f(w, h));
    box.setPosition(x, y);
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(active ? 2.0f : 1.0f);
    box.setOutlineColor(active ? sf::Color(40, 110, 200) : sf::Color(160, 160, 160));
    window.draw(box);
    drawText(text, x + 6.0f, y + 4.0f, 16, sf::Color::Black);
}

void ManagerProductWindow::draw() {
    window.clear(sf::Color(236, 240, 245));

    sf::RectangleShape header(sf::Vector2f(1280, 48));
    header.setFillColor(sf::Color(32, 74, 128));
    window.draw(header);
    drawText("Manager: Product Categories, Barcodes and Product Editor", 20, 12, 22, sf::Color::White);

    drawText("Categories (PCategory.txt)", 20, 54, 16, sf::Color(32, 74, 128));
    drawField(20, 70, 70, 28, catIdInput, activeField == MF_CAT_ID);
    drawField(100, 70, 180, 28, catNameInput, activeField == MF_CAT_NAME);
    drawField(290, 70, 80, 28, catAcronymInput, activeField == MF_CAT_ACRONYM);
    drawText("ID", 20, 54, 12, sf::Color(90, 90, 90));
    drawButton(20, 108, 160, 32, "Add Category", sf::Color(39, 128, 82));

    drawText("Click a category to use it for barcode generation.", 190, 114, 14, sf::Color(80, 80, 80));

    for (int i = 0; i < 8; i++) {
        int idx = categoryScroll + i;
        sf::RectangleShape row(sf::Vector2f(390, 26));
        row.setPosition(20.f, 160.f + static_cast<float>(i * 28));
        bool selected = (idx == selectedCategoryIndex);
        row.setFillColor(selected ? sf::Color(186, 214, 245) : sf::Color::White);
        window.draw(row);
        if (idx < categoryManager.getCategoryCount()) {
            const Category* c = categoryManager.getCategoryAt(idx);
            drawText(c->getCategoryId() + "  " + c->getName() + "  (" + c->getAcronym() + ")", 28.f, 163.f + static_cast<float>(i * 28), 15, sf::Color::Black);
        }
    }

    drawText("Product form", 450, 54, 16, sf::Color(32, 74, 128));
    drawText("Product ID", 450, 54, 12, sf::Color(90, 90, 90));
    drawField(450, 70, 140, 28, productIdInput, activeField == MF_PROD_ID);
    drawField(600, 70, 120, 28, acronymInput, activeField == MF_ACRONYM);
    drawField(730, 70, 250, 28, nameInput, activeField == MF_NAME);
    drawText("Acronym", 600, 54, 12, sf::Color(90, 90, 90));
    drawText("Name", 730, 54, 12, sf::Color(90, 90, 90));

    drawText("Unit / Price / Tax / Discount / Stock", 450, 108, 12, sf::Color(90, 90, 90));
    drawField(450, 130, 120, 28, unitInput, activeField == MF_UNIT);
    drawField(580, 130, 100, 28, priceInput, activeField == MF_PRICE);
    drawField(690, 130, 100, 28, taxInput, activeField == MF_TAX);
    drawField(800, 130, 100, 28, discountInput, activeField == MF_DISCOUNT);
    drawField(910, 130, 80, 28, stockInput, activeField == MF_STOCK);

    drawText("Description", 450, 168, 12, sf::Color(90, 90, 90));
    drawField(450, 190, 530, 28, descriptionInput, activeField == MF_DESC);

    drawText("Category ID: " + (selectedCategoryId.empty() ? "(none selected)" : selectedCategoryId), 450, 280, 16, sf::Color::Black);
    drawText("Barcode: " + (barcodeInput.empty() ? "(generate)" : barcodeInput), 450, 302, 16, sf::Color::Black);
    drawText("Image: " + (imagePathInput.empty() ? "(none)" : imagePathInput), 450, 324, 14, sf::Color(70, 70, 70));

    drawButton(450, 230, 200, 36, "Generate Barcode", sf::Color(32, 74, 128));
    drawButton(660, 230, 140, 36, "Toggle Unit", sf::Color(90, 90, 90));
    drawButton(810, 230, 160, 36, "New Product", sf::Color(110, 80, 40));
    drawButton(980, 230, 160, 36, editingExisting ? "Save Changes" : "Add Product", sf::Color(39, 128, 82));

    drawText("Products (click a row to edit)", 20, 336, 16, sf::Color(32, 74, 128));
    for (int i = 0; i < 10; i++) {
        int idx = productScroll + i;
        sf::RectangleShape row(sf::Vector2f(1240, 26));
        row.setPosition(20.f, 360.f + static_cast<float>(i * 28));
        bool selected = (idx == selectedProductIndex);
        row.setFillColor(selected ? sf::Color(186, 214, 245) : ((i % 2 == 0) ? sf::Color::White : sf::Color(248, 248, 248)));
        window.draw(row);
        if (idx < productManager.getProductCount()) {
            const Product* p = productManager.getProductAt(idx);
            std::ostringstream line;
            line << p->getProductId() << "  " << p->getBarcode() << "  cat:" << (p->getCategoryId().empty() ? "--" : p->getCategoryId())
                 << "  " << p->getProductName() << "  Rs " << std::fixed << std::setprecision(2) << p->getPrice()
                 << "  stock " << p->getStockQuantity();
            drawText(line.str(), 28.f, 363.f + static_cast<float>(i * 28), 14, sf::Color::Black);
        }
    }

    sf::Color statusColor = statusSuccess ? sf::Color(20, 110, 50) : sf::Color(160, 30, 30);
    drawText(statusMessage.empty() ? "Select a category, generate a barcode, then add or edit a product." : statusMessage,
             20, 720, 16, statusMessage.empty() ? sf::Color(80, 80, 80) : statusColor);

    window.display();
}
