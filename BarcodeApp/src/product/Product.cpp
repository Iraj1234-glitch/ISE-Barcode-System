#include "../../include/product/Product.h"
#include <sstream>
#include <iomanip>

// Default constructor
Product::Product() {
    productId = "";
    barcode = "";
    acronym = "";
    productName = "";
    measurementUnit = "";
    description = "";
    price = 0.0;
    tax = 0.0;
    discount = 0.0;
    stockQuantity = 0;
    barcodeImagePath = "";
    categoryId = "";
}

// Parameterized constructor without stock (defaults to 0)
Product::Product(const std::string& id, const std::string& bCode,
                 const std::string& name, const std::string& desc,
                 double pr) {
    productId = id;
    barcode = bCode;
    acronym = "";
    productName = name;
    measurementUnit = "";
    description = desc;
    price = (pr >= 0.0) ? pr : 0.0;
    tax = 0.0;
    discount = 0.0;
    stockQuantity = 0;
    barcodeImagePath = "";
    categoryId = "";
}

// Parameterized constructor with stock
Product::Product(const std::string& id, const std::string& bCode,
                 const std::string& name, const std::string& desc,
                 double pr, int stock) {
    productId = id;
    barcode = bCode;
    acronym = "";
    productName = name;
    measurementUnit = "";
    description = desc;
    price = (pr >= 0.0) ? pr : 0.0;
    tax = 0.0;
    discount = 0.0;
    stockQuantity = (stock >= 0) ? stock : 0;
    barcodeImagePath = "";
    categoryId = "";
}

// Increment 2 full constructor
Product::Product(const std::string& id, const std::string& bCode,
                 const std::string& acr, const std::string& name,
                 const std::string& unit, double pr, double tx,
                 double disc, int stock, const std::string& imgPath,
                 const std::string& desc, const std::string& catId) {
    productId = id;
    barcode = bCode;
    acronym = acr;
    productName = name;
    measurementUnit = unit;
    price = (pr >= 0.0) ? pr : 0.0;
    tax = (tx >= 0.0) ? tx : 0.0;
    discount = (disc >= 0.0) ? disc : 0.0;
    stockQuantity = (stock >= 0) ? stock : 0;
    barcodeImagePath = imgPath;
    description = desc;
    categoryId = catId;
}

// Getters
std::string Product::getProductId() const {
    return productId;
}

std::string Product::getBarcode() const {
    return barcode;
}

std::string Product::getAcronym() const {
    return acronym;
}

std::string Product::getProductName() const {
    return productName;
}

std::string Product::getMeasurementUnit() const {
    return measurementUnit;
}

std::string Product::getDescription() const {
    return description;
}

double Product::getPrice() const {
    return price;
}

double Product::getTax() const {
    return tax;
}

double Product::getDiscount() const {
    return discount;
}

int Product::getStockQuantity() const {
    return stockQuantity;
}

std::string Product::getBarcodeImagePath() const {
    return barcodeImagePath;
}

std::string Product::getCategoryId() const {
    return categoryId;
}

// Setters
void Product::setProductId(const std::string& id) {
    productId = id;
}

void Product::setBarcode(const std::string& bCode) {
    barcode = bCode;
}

void Product::setAcronym(const std::string& acr) {
    acronym = acr;
}

void Product::setProductName(const std::string& name) {
    productName = name;
}

void Product::setMeasurementUnit(const std::string& unit) {
    measurementUnit = unit;
}

void Product::setDescription(const std::string& desc) {
    description = desc;
}

void Product::setPrice(double pr) {
    if (pr >= 0.0) {
        price = pr;
    }
}

void Product::setTax(double tx) {
    if (tx >= 0.0) {
        tax = tx;
    }
}

void Product::setDiscount(double disc) {
    if (disc >= 0.0) {
        discount = disc;
    }
}

void Product::setStockQuantity(int stock) {
    if (stock >= 0) {
        stockQuantity = stock;
    }
}

void Product::setBarcodeImagePath(const std::string& path) {
    barcodeImagePath = path;
}

void Product::setCategoryId(const std::string& catId) {
    categoryId = catId;
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
    oss << "Product ID:  " << productId << "\n"
        << "Barcode:     " << barcode << "\n";
    if (!acronym.empty()) {
        oss << "Acronym:     " << acronym << "\n";
    }
    oss << "Name:        " << productName << "\n";
    if (!measurementUnit.empty()) {
        oss << "Unit:        " << measurementUnit << "\n";
    }
    oss << "Description: " << description << "\n"
        << "Price:       Rs. " << std::fixed << std::setprecision(2) << price << "\n";
    if (tax > 0.0) {
        oss << "Tax:         Rs. " << std::fixed << std::setprecision(2) << tax << "\n";
    }
    if (discount > 0.0) {
        oss << "Discount:    Rs. " << std::fixed << std::setprecision(2) << discount << "\n";
    }
    oss << "Stock:       " << stockQuantity;
    if (!barcodeImagePath.empty()) {
        oss << "\nBarcode Img: " << barcodeImagePath;
    }
    if (!categoryId.empty()) {
        oss << "\nCategory ID: " << categoryId;
    }
    return oss.str();
}
