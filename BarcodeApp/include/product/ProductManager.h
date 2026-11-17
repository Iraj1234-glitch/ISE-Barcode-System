#pragma once
#include "Product.h"
#include "ProductFileHandler.h"
#include <string>

// Forward declaration of Order class to maintain loose coupling
class Order;

// Enumeration representing every stock validation outcome
enum class StockValidationCode {
    VALID = 0,               // Case 1: Product exists and sufficient stock is available
    INSUFFICIENT_STOCK,      // Case 2: Product exists but stock is insufficient
    PRODUCT_NOT_FOUND,       // Case 3: Product does not exist
    ZERO_QUANTITY,           // Case 4: Requested quantity is zero
    NEGATIVE_QUANTITY,       // Case 5: Requested quantity is negative
    INVALID_IDENTIFIER       // Case 6: Invalid barcode or product ID format
};

// Detailed result structure returned by stock validation
struct StockValidationResult {
    StockValidationCode code;
    bool isValid;
    std::string message;
    int availableStock;
    int requestedQuantity;
    std::string productId;
    std::string productBarcode;
    std::string productName;

    StockValidationResult()
        : code(StockValidationCode::INVALID_IDENTIFIER),
          isValid(false),
          message(""),
          availableStock(0),
          requestedQuantity(0),
          productId(""),
          productBarcode(""),
          productName("") {}
};

// Capacity for tracking processed orders to avoid duplicate deduction
const int MAX_PROCESSED_ORDERS = 100;

class ProductManager {
private:
    Product products[MAX_PRODUCTS];
    int productCount;

    std::string productFilePath;
    std::string inventoryFilePath;
    ProductFileHandler fileHandler;

    std::string processedOrderIds[MAX_PROCESSED_ORDERS];
    int processedOrderCount;

    // Internal index lookup helpers
    int findIndexByBarcode(const std::string& barcode) const;
    int findIndexById(const std::string& productId) const;
    int findIndex(const std::string& identifier) const;

public:
    // Constructors
    ProductManager();
    explicit ProductManager(const std::string& productFile);
    ProductManager(const std::string& productFile, const std::string& inventoryFile);

    // ==========================================
    // 1. Product File Management
    // ==========================================
    bool loadProducts();
    bool loadProducts(const std::string& filePath);
    bool saveProducts();
    bool saveProducts(const std::string& filePath, bool includeStock = true);

    bool loadInventory();
    bool loadInventory(const std::string& filePath);
    bool saveInventory();
    bool saveInventory(const std::string& filePath);

    // ==========================================
    // 2. In-Memory Collection Management
    // ==========================================
    int getProductCount() const;
    const Product* getProductAt(int index) const;
    Product* getProductAt(int index);
    bool addProduct(const Product& product);
    bool updateProduct(const Product& product);
    void clear();

    // ==========================================
    // 3. Barcode & ID-Based Product Lookup
    // ==========================================
    // Static barcode format validation (6 digits, first != '0')
    static bool isValidBarcodeFormat(const std::string& barcode);

    // Barcode search
    Product* findProductByBarcode(const std::string& barcode);
    const Product* findProductByBarcode(const std::string& barcode) const;
    bool getProductByBarcode(const std::string& barcode, Product& outProduct) const;

    // Product ID search
    Product* findProductById(const std::string& productId);
    const Product* findProductById(const std::string& productId) const;
    bool getProductById(const std::string& productId, Product& outProduct) const;

    // Unified lookup (checks barcode first, then product ID)
    Product* findProduct(const std::string& identifier);
    const Product* findProduct(const std::string& identifier) const;

    // ==========================================
    // 4. Product Description Retrieval (for Hiba)
    // ==========================================
    std::string getProductDescriptionByBarcode(const std::string& barcode) const;
    std::string getProductDescriptionById(const std::string& productId) const;
    std::string getFullProductDetails(const std::string& identifier) const;

    // ==========================================
    // 5. Price Retrieval (for Wajiha & Hiba)
    // ==========================================
    double getProductPriceByBarcode(const std::string& barcode) const;
    double getProductPriceById(const std::string& productId) const;

    // ==========================================
    // 6. Stock Validation (without deduction)
    // ==========================================
    StockValidationResult validateStockByBarcode(const std::string& barcode, int requestedQuantity) const;
    StockValidationResult validateStockById(const std::string& productId, int requestedQuantity) const;
    StockValidationResult validateStock(const std::string& identifier, int requestedQuantity) const;

    // ==========================================
    // 7. Inventory & Quantity Updates
    // ==========================================
    int getStockByBarcode(const std::string& barcode) const;
    int getStockById(const std::string& productId) const;
    int getStock(const std::string& identifier) const;

    bool updateStockQuantity(const std::string& identifier, int newQuantity);
    bool addStock(const std::string& identifier, int quantityToAdd);
    bool deductStock(const std::string& identifier, int quantityToDeduct);

    // ==========================================
    // 8. Order Management Workflow Coordination (for Iraj)
    // ==========================================
    bool isOrderProcessed(const std::string& orderId) const;
    bool processOrderDeduction(const Order& order);
    bool restoreOrderStock(const Order& order);
    void resetProcessedOrders();
};
