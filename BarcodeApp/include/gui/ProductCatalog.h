#pragma once

#include <string>

// Maximum number of products the GUI keeps in memory
const int MAX_CATALOG_PRODUCTS = 500;

// One product as the GUI needs it (lookup, display, billing)
struct ProductInfo
{
    std::string productId;
    std::string barcode;      // exactly 6 digits, first digit not zero
    std::string name;
    std::string description;
    double price = 0.0;
};

// Thin adapter between the GUI and the product data.
//
// File format (Product.txt), one product per line, same style as Order.txt:
//   productId barcode "Name" "Description" price #
//
// If your teammate's product module already exists, keep this class's public
// interface and change only the bodies in ProductCatalog.cpp to call it.
class ProductCatalog
{
private:
    ProductInfo items[MAX_CATALOG_PRODUCTS];
    int count;
    std::string filePath;

public:
    explicit ProductCatalog(const std::string& path = "Product.txt");

    bool load();
    int getCount() const;

    const ProductInfo* findByBarcode(const std::string& barcode) const;
    const ProductInfo* findById(const std::string& productId) const;
};
