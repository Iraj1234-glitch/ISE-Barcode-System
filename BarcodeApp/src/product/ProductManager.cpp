#include "../../include/product/ProductManager.h"
#include "../../include/order/Order.h"
#include <iostream>
#include <iomanip>
#include <sstream>

// Constructors
ProductManager::ProductManager() {
    productCount = 0;
    productFilePath = "Product.txt";
    inventoryFilePath = "";
    processedOrderCount = 0;
}

ProductManager::ProductManager(const std::string& productFile) {
    productCount = 0;
    productFilePath = productFile;
    inventoryFilePath = "";
    processedOrderCount = 0;
}

ProductManager::ProductManager(const std::string& productFile, const std::string& inventoryFile) {
    productCount = 0;
    productFilePath = productFile;
    inventoryFilePath = inventoryFile;
    processedOrderCount = 0;
}

// Internal index lookup helpers
int ProductManager::findIndexByBarcode(const std::string& barcode) const {
    if (barcode.empty()) return -1;
    for (int i = 0; i < productCount; i++) {
        if (products[i].getBarcode() == barcode) {
            return i;
        }
    }
    return -1;
}

int ProductManager::findIndexById(const std::string& productId) const {
    if (productId.empty()) return -1;
    for (int i = 0; i < productCount; i++) {
        if (products[i].getProductId() == productId) {
            return i;
        }
    }
    return -1;
}

int ProductManager::findIndex(const std::string& identifier) const {
    if (identifier.empty()) return -1;
    int idx = findIndexByBarcode(identifier);
    if (idx != -1) return idx;
    return findIndexById(identifier);
}

// ==========================================
// 1. Product File Management
// ==========================================
bool ProductManager::loadProducts() {
    return loadProducts(productFilePath);
}

bool ProductManager::loadProducts(const std::string& filePath) {
    productFilePath = filePath;
    bool success = fileHandler.loadProducts(filePath, products, productCount, MAX_PRODUCTS);
    if (success && !inventoryFilePath.empty()) {
        fileHandler.loadInventory(inventoryFilePath, products, productCount);
    }
    return success;
}

bool ProductManager::saveProducts() {
    return saveProducts(productFilePath, true);
}

bool ProductManager::saveProducts(const std::string& filePath, bool includeStock) {
    productFilePath = filePath;
    return fileHandler.saveProducts(filePath, products, productCount, includeStock);
}

bool ProductManager::loadInventory() {
    if (inventoryFilePath.empty()) return false;
    return loadInventory(inventoryFilePath);
}

bool ProductManager::loadInventory(const std::string& filePath) {
    inventoryFilePath = filePath;
    return fileHandler.loadInventory(filePath, products, productCount);
}

bool ProductManager::saveInventory() {
    if (inventoryFilePath.empty()) return false;
    return saveInventory(inventoryFilePath);
}

bool ProductManager::saveInventory(const std::string& filePath) {
    inventoryFilePath = filePath;
    return fileHandler.saveInventory(filePath, products, productCount);
}

// ==========================================
// 2. In-Memory Collection Management
// ==========================================
int ProductManager::getProductCount() const {
    return productCount;
}

const Product* ProductManager::getProductAt(int index) const {
    if (index >= 0 && index < productCount) {
        return &products[index];
    }
    return nullptr;
}

Product* ProductManager::getProductAt(int index) {
    if (index >= 0 && index < productCount) {
        return &products[index];
    }
    return nullptr;
}

bool ProductManager::addProduct(const Product& product) {
    if (productCount >= MAX_PRODUCTS) {
        return false;
    }
    // Check for duplicate product ID
    if (findIndexById(product.getProductId()) != -1) {
        return false;
    }
    // Check for duplicate barcode
    if (findIndexByBarcode(product.getBarcode()) != -1) {
        return false;
    }

    products[productCount++] = product;
    return true;
}

bool ProductManager::updateProduct(const Product& product) {
    int idx = findIndexById(product.getProductId());
    if (idx == -1) {
        idx = findIndexByBarcode(product.getBarcode());
    }
    if (idx != -1) {
        products[idx] = product;
        return true;
    }
    return false;
}

void ProductManager::clear() {
    productCount = 0;
    processedOrderCount = 0;
}

// ==========================================
// 3. Barcode & ID-Based Product Lookup
// ==========================================
bool ProductManager::isValidBarcodeFormat(const std::string& barcode) {
    if (barcode.length() != 6) {
        return false;
    }
    if (barcode[0] == '0') {
        return false;
    }
    for (char c : barcode) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

Product* ProductManager::findProductByBarcode(const std::string& barcode) {
    int idx = findIndexByBarcode(barcode);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

const Product* ProductManager::findProductByBarcode(const std::string& barcode) const {
    int idx = findIndexByBarcode(barcode);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

bool ProductManager::getProductByBarcode(const std::string& barcode, Product& outProduct) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        outProduct = *p;
        return true;
    }
    return false;
}

Product* ProductManager::findProductById(const std::string& productId) {
    int idx = findIndexById(productId);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

const Product* ProductManager::findProductById(const std::string& productId) const {
    int idx = findIndexById(productId);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

bool ProductManager::getProductById(const std::string& productId, Product& outProduct) const {
    const Product* p = findProductById(productId);
    if (p != nullptr) {
        outProduct = *p;
        return true;
    }
    return false;
}

Product* ProductManager::findProduct(const std::string& identifier) {
    int idx = findIndex(identifier);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

const Product* ProductManager::findProduct(const std::string& identifier) const {
    int idx = findIndex(identifier);
    if (idx != -1) {
        return &products[idx];
    }
    return nullptr;
}

// ==========================================
// 4. Product Description Retrieval (for Hiba)
// ==========================================
std::string ProductManager::getProductDescriptionByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getDescription();
    }
    return "";
}

std::string ProductManager::getProductDescriptionById(const std::string& productId) const {
    const Product* p = findProductById(productId);
    if (p != nullptr) {
        return p->getDescription();
    }
    return "";
}

std::string ProductManager::getFullProductDetails(const std::string& identifier) const {
    const Product* p = findProduct(identifier);
    if (p != nullptr) {
        return p->getFormattedDetails();
    }
    return "Product not found.";
}

// ==========================================
// 5. Price Retrieval (for Wajiha & Hiba)
// ==========================================
double ProductManager::getProductPriceByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getPrice();
    }
    return 0.0;
}

double ProductManager::getProductPriceById(const std::string& productId) const {
    const Product* p = findProductById(productId);
    if (p != nullptr) {
        return p->getPrice();
    }
    return 0.0;
}

// ==========================================
// 5b. Increment 2 Field Retrieval
// ==========================================
std::string ProductManager::getProductAcronymByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getAcronym();
    }
    return "";
}

std::string ProductManager::getProductMeasurementUnitByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getMeasurementUnit();
    }
    return "";
}

double ProductManager::getProductTaxByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getTax();
    }
    return 0.0;
}

double ProductManager::getProductDiscountByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getDiscount();
    }
    return 0.0;
}

std::string ProductManager::getProductBarcodeImagePathByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getBarcodeImagePath();
    }
    return "";
}

// ==========================================
// 6. Stock Validation (without deduction)
// ==========================================
StockValidationResult ProductManager::validateStockByBarcode(const std::string& barcode, int requestedQuantity) const {
    StockValidationResult res;
    res.productBarcode = barcode;
    res.requestedQuantity = requestedQuantity;

    if (barcode.empty()) {
        res.code = StockValidationCode::INVALID_IDENTIFIER;
        res.isValid = false;
        res.message = "Error: Barcode cannot be empty.";
        return res;
    }

    if (requestedQuantity == 0) {
        res.code = StockValidationCode::ZERO_QUANTITY;
        res.isValid = false;
        res.message = "Error: Requested quantity cannot be zero.";
        return res;
    }

    if (requestedQuantity < 0) {
        res.code = StockValidationCode::NEGATIVE_QUANTITY;
        res.isValid = false;
        res.message = "Error: Requested quantity cannot be negative.";
        return res;
    }

    const Product* p = findProductByBarcode(barcode);
    if (p == nullptr) {
        res.code = StockValidationCode::PRODUCT_NOT_FOUND;
        res.isValid = false;
        res.message = "Error: Product with barcode " + barcode + " not found.";
        return res;
    }

    res.productId = p->getProductId();
    res.productName = p->getProductName();
    res.availableStock = p->getStockQuantity();

    if (p->getStockQuantity() < requestedQuantity) {
        res.code = StockValidationCode::INSUFFICIENT_STOCK;
        res.isValid = false;
        res.message = "Error: Insufficient stock. Available: " +
                      std::to_string(p->getStockQuantity()) +
                      ", Requested: " + std::to_string(requestedQuantity);
        return res;
    }

    res.code = StockValidationCode::VALID;
    res.isValid = true;
    res.message = "Success: Stock is available.";
    return res;
}

StockValidationResult ProductManager::validateStockById(const std::string& productId, int requestedQuantity) const {
    StockValidationResult res;
    res.productId = productId;
    res.requestedQuantity = requestedQuantity;

    if (productId.empty()) {
        res.code = StockValidationCode::INVALID_IDENTIFIER;
        res.isValid = false;
        res.message = "Error: Product ID cannot be empty.";
        return res;
    }

    if (requestedQuantity == 0) {
        res.code = StockValidationCode::ZERO_QUANTITY;
        res.isValid = false;
        res.message = "Error: Requested quantity cannot be zero.";
        return res;
    }

    if (requestedQuantity < 0) {
        res.code = StockValidationCode::NEGATIVE_QUANTITY;
        res.isValid = false;
        res.message = "Error: Requested quantity cannot be negative.";
        return res;
    }

    const Product* p = findProductById(productId);
    if (p == nullptr) {
        res.code = StockValidationCode::PRODUCT_NOT_FOUND;
        res.isValid = false;
        res.message = "Error: Product with ID " + productId + " not found.";
        return res;
    }

    res.productBarcode = p->getBarcode();
    res.productName = p->getProductName();
    res.availableStock = p->getStockQuantity();

    if (p->getStockQuantity() < requestedQuantity) {
        res.code = StockValidationCode::INSUFFICIENT_STOCK;
        res.isValid = false;
        res.message = "Error: Insufficient stock. Available: " +
                      std::to_string(p->getStockQuantity()) +
                      ", Requested: " + std::to_string(requestedQuantity);
        return res;
    }

    res.code = StockValidationCode::VALID;
    res.isValid = true;
    res.message = "Success: Stock is available.";
    return res;
}

StockValidationResult ProductManager::validateStock(const std::string& identifier, int requestedQuantity) const {
    if (identifier.empty()) {
        StockValidationResult res;
        res.code = StockValidationCode::INVALID_IDENTIFIER;
        res.isValid = false;
        res.message = "Error: Identifier cannot be empty.";
        return res;
    }

    // Try finding by barcode first; if not found, try by ID
    if (findIndexByBarcode(identifier) != -1) {
        return validateStockByBarcode(identifier, requestedQuantity);
    }
    return validateStockById(identifier, requestedQuantity);
}

// ==========================================
// 7. Inventory & Quantity Updates
// ==========================================
int ProductManager::getStockByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getStockQuantity();
    }
    return -1;
}

int ProductManager::getStockById(const std::string& productId) const {
    const Product* p = findProductById(productId);
    if (p != nullptr) {
        return p->getStockQuantity();
    }
    return -1;
}

int ProductManager::getStock(const std::string& identifier) const {
    const Product* p = findProduct(identifier);
    if (p != nullptr) {
        return p->getStockQuantity();
    }
    return -1;
}

bool ProductManager::updateStockQuantity(const std::string& identifier, int newQuantity) {
    if (newQuantity < 0) {
        return false; // Prevent negative inventory
    }
    Product* p = findProduct(identifier);
    if (p != nullptr) {
        p->setStockQuantity(newQuantity);
        return true;
    }
    return false;
}

bool ProductManager::addStock(const std::string& identifier, int quantityToAdd) {
    if (quantityToAdd <= 0) {
        return false;
    }
    Product* p = findProduct(identifier);
    if (p != nullptr) {
        return p->addStock(quantityToAdd);
    }
    return false;
}

bool ProductManager::deductStock(const std::string& identifier, int quantityToDeduct) {
    if (quantityToDeduct <= 0) {
        return false;
    }
    Product* p = findProduct(identifier);
    if (p != nullptr) {
        return p->deductStock(quantityToDeduct);
    }
    return false;
}

// ==========================================
// 8. Order Management Workflow Coordination (for Iraj)
// ==========================================
bool ProductManager::isOrderProcessed(const std::string& orderId) const {
    if (orderId.empty()) return false;
    for (int i = 0; i < processedOrderCount; i++) {
        if (processedOrderIds[i] == orderId) {
            return true;
        }
    }
    return false;
}

bool ProductManager::processOrderDeduction(const Order& order) {
    std::string oId = order.getOrderId();
    if (oId.empty()) {
        return false;
    }

    // Guard: Prevent deducting stock twice for the same order
    if (isOrderProcessed(oId)) {
        return false;
    }

    int detailsCount = order.getDetailCount();
    if (detailsCount <= 0) {
        return false;
    }

    // Step 1: Pre-validation of all order items (Atomic check)
    for (int i = 0; i < detailsCount; i++) {
        const OrderDetail& detail = order.getDetail(i);
        std::string pId = detail.getProductId();
        std::string pBarcode = detail.getProductBarcode();
        int qty = detail.getQuantity();

        if (qty <= 0) {
            return false;
        }

        const Product* p = findProductById(pId);
        if (p == nullptr) {
            p = findProductByBarcode(pBarcode);
        }

        if (p == nullptr || p->getStockQuantity() < qty) {
            // Cannot fulfill complete order: abort without modifying stock
            return false;
        }
    }

    // Step 2: Deduct stock for each item
    for (int i = 0; i < detailsCount; i++) {
        const OrderDetail& detail = order.getDetail(i);
        std::string pId = detail.getProductId();
        std::string pBarcode = detail.getProductBarcode();
        int qty = detail.getQuantity();

        Product* p = findProductById(pId);
        if (p == nullptr) {
            p = findProductByBarcode(pBarcode);
        }

        if (p != nullptr) {
            p->deductStock(qty);
        }
    }

    // Step 3: Record order as processed
    if (processedOrderCount < MAX_PROCESSED_ORDERS) {
        processedOrderIds[processedOrderCount++] = oId;
    }

    // Step 4: Persist updated stock
    saveProducts();
    if (!inventoryFilePath.empty()) {
        saveInventory();
    }

    return true;
}

bool ProductManager::restoreOrderStock(const Order& order) {
    std::string oId = order.getOrderId();
    if (oId.empty()) {
        return false;
    }

    // Verify that the order was actually previously processed
    int foundIdx = -1;
    for (int i = 0; i < processedOrderCount; i++) {
        if (processedOrderIds[i] == oId) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1) {
        // Order was not processed, cannot restore
        return false;
    }

    int detailsCount = order.getDetailCount();
    for (int i = 0; i < detailsCount; i++) {
        const OrderDetail& detail = order.getDetail(i);
        std::string pId = detail.getProductId();
        std::string pBarcode = detail.getProductBarcode();
        int qty = detail.getQuantity();

        Product* p = findProductById(pId);
        if (p == nullptr) {
            p = findProductByBarcode(pBarcode);
        }

        if (p != nullptr && qty > 0) {
            p->addStock(qty);
        }
    }

    // Remove from processed orders list
    for (int i = foundIdx; i < processedOrderCount - 1; i++) {
        processedOrderIds[i] = processedOrderIds[i + 1];
    }
    processedOrderCount--;

    // Persist restored stock
    saveProducts();
    if (!inventoryFilePath.empty()) {
        saveInventory();
    }

    return true;
}

void ProductManager::resetProcessedOrders() {
    processedOrderCount = 0;
}
