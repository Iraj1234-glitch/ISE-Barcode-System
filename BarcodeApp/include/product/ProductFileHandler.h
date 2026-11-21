#pragma once
#include "Product.h"
#include <string>

// Maximum number of products that can be read or stored
const int MAX_PRODUCTS = 200;

class ProductFileHandler {
public:
    // Helper string functions
    static std::string trim(const std::string& str);
    static std::string cleanLine(const std::string& line);
    static std::string formatField(const std::string& str);
    static int splitSpaceQuoted(const std::string& line, std::string tokens[], int maxTokens);

    // Known descriptions preserver for Increment 1 products
    static std::string getPreservedDescription(const std::string& productId, const std::string& productName);

    // Reasonable default tax assignment in PKR (as authorized in Increment 2 specifications)
    static double getDefaultTaxForProduct(const std::string& productId);

    // File loading functions
    // Reads records from filePath into products array.
    // Supports Increment 3 11/12-field records, Increment 2 10-field records and legacy 5/6-field records.
    // Increment 3 Product.txt (safest compatible extension — Category ID appended after barcode image path):
    //   productId barcode acronym "Name" unit price tax discount stock "barcodeImagePath" categoryId ["description"] #
    bool loadProducts(const std::string& filePath, Product products[], int& productCount, int maxCapacity = MAX_PRODUCTS);

    // File saving functions
    // Saves products array into filePath with formatting preserving quotation marks and delimiters.
    // If includeStock is true, includes stock quantity.
    // Increment 2/3 products are written as 11 fields (Category ID last required field) plus optional description, then '#'.
    bool saveProducts(const std::string& filePath, const Product products[], int productCount, bool includeStock = true);

    // Optional separate inventory file support (ProductID Stock #)
    bool loadInventory(const std::string& filePath, Product products[], int productCount);
    bool saveInventory(const std::string& filePath, const Product products[], int productCount);
};
