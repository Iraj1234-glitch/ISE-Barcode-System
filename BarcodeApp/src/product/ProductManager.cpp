#include "../../include/product/ProductManager.h"
#include "../../include/order/Order.h"
#include "../../include/order/OrderManager.h"
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

bool ProductManager::updateProductById(const std::string& originalProductId, const Product& product) {
    int idx = findIndexById(originalProductId);
    if (idx == -1) {
        return false;
    }
    if (product.getProductId() != originalProductId) {
        if (findIndexById(product.getProductId()) != -1) {
            return false;
        }
    }
    int barcodeIdx = findIndexByBarcode(product.getBarcode());
    if (barcodeIdx != -1 && barcodeIdx != idx) {
        return false;
    }
    products[idx] = product;
    return true;
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

std::string ProductManager::getProductCategoryIdByBarcode(const std::string& barcode) const {
    const Product* p = findProductByBarcode(barcode);
    if (p != nullptr) {
        return p->getCategoryId();
    }
    return "";
}

std::string ProductManager::generateUniqueBarcodeForCategory(const std::string& categoryId) const {
    if (categoryId.length() != 2 || categoryId[0] < '1' || categoryId[0] > '9' ||
        categoryId[1] < '0' || categoryId[1] > '9') {
        return "";
    }

    for (int sequence = 1; sequence <= 9999; sequence++) {
        std::ostringstream oss;
        oss << categoryId << std::setw(4) << std::setfill('0') << sequence;
        std::string candidate = oss.str();
        if (isValidBarcodeFormat(candidate) && findIndexByBarcode(candidate) == -1) {
            return candidate;
        }
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
// 7. Inventory & Quantity Updates (Requirement 4)
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

bool ProductManager::updateStockQuantity(const std::string& identifier, int newQuantity, std::string& outMessage) {
    if (identifier.empty()) {
        outMessage = "Error: Product identifier cannot be empty.";
        return false;
    }
    if (newQuantity < 0) {
        outMessage = "Error: Stock quantity cannot be negative (" + std::to_string(newQuantity) + ").";
        return false;
    }
    Product* p = findProduct(identifier);
    if (p == nullptr) {
        outMessage = "Error: Product with identifier '" + identifier + "' not found.";
        return false;
    }
    p->setStockQuantity(newQuantity);
    outMessage = "Success: Stock quantity updated to " + std::to_string(newQuantity) + " for " + p->getProductName() + ".";
    return true;
}

bool ProductManager::updateStockQuantity(const std::string& identifier, int newQuantity) {
    std::string msg;
    return updateStockQuantity(identifier, newQuantity, msg);
}

bool ProductManager::addStock(const std::string& identifier, int quantityToAdd, std::string& outMessage) {
    if (identifier.empty()) {
        outMessage = "Error: Product identifier cannot be empty.";
        return false;
    }
    if (quantityToAdd <= 0) {
        outMessage = "Error: Quantity to add must be positive (" + std::to_string(quantityToAdd) + ").";
        return false;
    }
    Product* p = findProduct(identifier);
    if (p == nullptr) {
        outMessage = "Error: Product with identifier '" + identifier + "' not found.";
        return false;
    }
    bool ok = p->addStock(quantityToAdd);
    if (ok) {
        outMessage = "Success: Added " + std::to_string(quantityToAdd) + " units to " + p->getProductName() +
                     ". New available stock: " + std::to_string(p->getStockQuantity()) + ".";
    } else {
        outMessage = "Error: Could not add stock to product.";
    }
    return ok;
}

bool ProductManager::addStock(const std::string& identifier, int quantityToAdd) {
    std::string msg;
    return addStock(identifier, quantityToAdd, msg);
}

bool ProductManager::deductStock(const std::string& identifier, int quantityToDeduct, std::string& outMessage) {
    if (identifier.empty()) {
        outMessage = "Error: Product identifier cannot be empty.";
        return false;
    }
    if (quantityToDeduct <= 0) {
        outMessage = "Error: Quantity to deduct must be positive (" + std::to_string(quantityToDeduct) + ").";
        return false;
    }
    Product* p = findProduct(identifier);
    if (p == nullptr) {
        outMessage = "Error: Product with identifier '" + identifier + "' not found.";
        return false;
    }
    if (p->getStockQuantity() < quantityToDeduct) {
        outMessage = "Error: Insufficient stock for " + p->getProductName() + ". Available: " +
                     std::to_string(p->getStockQuantity()) + ", Requested deduction: " +
                     std::to_string(quantityToDeduct) + ".";
        return false;
    }
    bool ok = p->deductStock(quantityToDeduct);
    if (ok) {
        outMessage = "Success: Deducted " + std::to_string(quantityToDeduct) + " units from " + p->getProductName() +
                     ". Remaining stock: " + std::to_string(p->getStockQuantity()) + ".";
    }
    return ok;
}

bool ProductManager::deductStock(const std::string& identifier, int quantityToDeduct) {
    std::string msg;
    return deductStock(identifier, quantityToDeduct, msg);
}

// ==========================================
// 8. Order Management Workflow Coordination (Requirements 5 & 10)
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

bool ProductManager::markOrderProcessed(const std::string& orderId) {
    if (orderId.empty()) return false;
    if (isOrderProcessed(orderId)) return true;
    if (processedOrderCount >= MAX_PROCESSED_ORDERS) return false;
    processedOrderIds[processedOrderCount++] = orderId;
    return true;
}

void ProductManager::syncProcessedOrdersFromOrderManager(const OrderManager& orderMgr) {
    int total = orderMgr.getOrderCount();
    for (int i = 0; i < total; i++) {
        const Order* o = orderMgr.getOrderAt(i);
        if (o != nullptr && o->getOrderStatus() == "paid") {
            markOrderProcessed(o->getOrderId());
        }
    }
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

    // Step 1: Pre-validation of all order items (Atomic check: ensure complete order can be fulfilled)
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

    // Step 2: Deduct stock for each item in memory
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

    // Step 3: Persist updated stock to file. If saving fails, rollback changes immediately!
    bool saved = saveProducts();
    if (!saved) {
        // Rollback all deducted quantities
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
                p->addStock(qty);
            }
        }
        return false;
    }

    if (!inventoryFilePath.empty()) {
        saveInventory();
    }

    // Step 4: Record order as processed
    if (processedOrderCount < MAX_PROCESSED_ORDERS) {
        processedOrderIds[processedOrderCount++] = oId;
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

    // Step 1: Add back stock in memory
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

    // Step 2: Persist restored stock. If saving fails, rollback changes!
    bool saved = saveProducts();
    if (!saved) {
        // Rollback all restored quantities
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
                p->deductStock(qty);
            }
        }
        return false;
    }

    if (!inventoryFilePath.empty()) {
        saveInventory();
    }

    // Step 3: Remove from processed orders list
    for (int i = foundIdx; i < processedOrderCount - 1; i++) {
        processedOrderIds[i] = processedOrderIds[i + 1];
    }
    processedOrderCount--;

    return true;
}

void ProductManager::resetProcessedOrders() {
    processedOrderCount = 0;
}
