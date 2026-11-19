#pragma once

#include <string>

// Maximum number of products the GUI keeps in memory
const int MAX_CATALOG_PRODUCTS = 500;

// One product as the GUI needs it (lookup, display, billing)
struct ProductInfo
{
    std::string productId;
    std::string barcode;          // exactly 6 digits, first digit not zero
    std::string acronym;          // Increment 2: Product acronym (max 8 characters)
    std::string name;
    std::string measurementUnit;  // Increment 2: Measurement unit (count or weight)
    std::string description;
    double price = 0.0;
    double tax = 0.0;             // Increment 2: Tax in PKR
    double discount = 0.0;        // Increment 2: Discount per unit
    int stock = 0;                // Increment 2: Available quantity
    std::string barcodeImagePath; // Increment 2: Barcode image path
};

// Thin adapter between the GUI and the product data.
//
// File format (Product.txt), one product per line, ending with #:
// Increment 2:
//   productId barcode acronym "Name" unit price tax discount stock "barcodeImagePath" #
// Legacy Increment 1:
//   productId barcode "Name" "Description" price [stock] #
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
