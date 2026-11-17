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

    // File loading functions
    // Reads records from filePath into products array.
    // Populates productCount with number of successfully parsed records.
    // Returns true on success or if file is empty; returns false if file cannot be opened.
    bool loadProducts(const std::string& filePath, Product products[], int& productCount, int maxCapacity = MAX_PRODUCTS);

    // File saving functions
    // Saves products array into filePath with formatting preserving quotation marks and delimiters.
    // If includeStock is true, includes the stock quantity before '#'.
    bool saveProducts(const std::string& filePath, const Product products[], int productCount, bool includeStock = true);

    // Optional separate inventory file support (ProductID Stock #)
    bool loadInventory(const std::string& filePath, Product products[], int productCount);
    bool saveInventory(const std::string& filePath, const Product products[], int productCount);
};
