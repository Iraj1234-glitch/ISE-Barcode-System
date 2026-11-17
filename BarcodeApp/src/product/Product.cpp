#include "../../include/product/Product.h"
#include <sstream>
#include <iomanip>

// Default constructor
Product::Product() {
    productId = "";
    barcode = "";
    productName = "";
    description = "";
    price = 0.0;
    stockQuantity = 0;
}

// Parameterized constructor without stock (defaults to 0)
Product::Product(const std::string& id, const std::string& bCode,
                 const std::string& name, const std::string& desc,
                 double pr) {
    productId = id;
    barcode = bCode;
    productName = name;
    description = desc;
    price = (pr >= 0.0) ? pr : 0.0;
    stockQuantity = 0;
}

// Parameterized constructor with stock
Product::Product(const std::string& id, const std::string& bCode,
                 const std::string& name, const std::string& desc,
                 double pr, int stock) {
    productId = id;
    barcode = bCode;
    productName = name;
    description = desc;
    price = (pr >= 0.0) ? pr : 0.0;
    stockQuantity = (stock >= 0) ? stock : 0;
}

// Getters
std::string Product::getProductId() const {
    return productId;
}

std::string Product::getBarcode() const {
    return barcode;
}

std::string Product::getProductName() const {
    return productName;
}

std::string Product::getDescription() const {
    return description;
}

double Product::getPrice() const {
    return price;
}

int Product::getStockQuantity() const {
    return stockQuantity;
}

// Setters
void Product::setProductId(const std::string& id) {
    productId = id;
}

void Product::setBarcode(const std::string& bCode) {
    barcode = bCode;
}

void Product::setProductName(const std::string& name) {
    productName = name;
}

void Product::setDescription(const std::string& desc) {
    description = desc;
}

void Product::setPrice(double pr) {
    if (pr >= 0.0) {
        price = pr;
    }
}

void Product::setStockQuantity(int stock) {
    if (stock >= 0) {
        stockQuantity = stock;
    }
}

// Stock operation methods
bool Product::isAvailable(int requestedQty) const {
    if (requestedQty <= 0) {
        return false;
    }
    return stockQuantity >= requestedQty;
}

bool Product::deductStock(int qty) {
    if (qty <= 0) {
        return false;
    }
    if (stockQuantity < qty) {
        return false;
    }
    stockQuantity -= qty;
    return true;
}

bool Product::addStock(int qty) {
    if (qty <= 0) {
        return false;
    }
    stockQuantity += qty;
    return true;
}

std::string Product::getFormattedDetails() const {
    std::ostringstream oss;
    oss << "Product ID: " << productId << "\n"
        << "Barcode:    " << barcode << "\n"
        << "Name:       " << productName << "\n"
        << "Description: " << description << "\n"
        << "Price:      Rs. " << std::fixed << std::setprecision(2) << price << "\n"
        << "Stock:      " << stockQuantity;
    return oss.str();
}
