#include "../../include/product/ProductFileHandler.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdio>

// Trims whitespace from both ends of a string
std::string ProductFileHandler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, (end - start + 1));
}

// Strips whitespace and the trailing '#' terminator if present
std::string ProductFileHandler::cleanLine(const std::string& line) {
    std::string cleaned = trim(line);
    if (!cleaned.empty() && cleaned.back() == '#') {
        cleaned.pop_back();
    }
    return trim(cleaned);
}

// Wraps a string in quotes if it contains spaces or tabs
std::string ProductFileHandler::formatField(const std::string& str) {
    if (str.empty() || str.find(' ') != std::string::npos || str.find('\t') != std::string::npos) {
        return "\"" + str + "\"";
    }
    return str;
}

// Splits space-separated line into tokens, respecting double-quoted strings
int ProductFileHandler::splitSpaceQuoted(const std::string& line, std::string tokens[], int maxTokens) {
    int count = 0;
    std::string current = "";
    bool inQuotes = false;
    bool hadQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            hadQuotes = true;
        } else if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!current.empty() || hadQuotes) {
                if (count < maxTokens) {
                    tokens[count++] = current;
                }
                current = "";
                hadQuotes = false;
            }
        } else {
            current += c;
        }
    }

    if (!current.empty() || hadQuotes) {
        if (count < maxTokens) {
            tokens[count++] = current;
        }
    }

    return count;
}

bool ProductFileHandler::loadProducts(const std::string& filePath, Product products[], int& productCount, int maxCapacity) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // Missing file handled safely
        productCount = 0;
        return false;
    }

    productCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue; // Skip blank lines safely
        }

        std::string tokens[10];
        int tokenCount = splitSpaceQuoted(cleaned, tokens, 10);

        // Required minimal fields: ProductID, Barcode, Name, Description, Price (5 fields)
        if (tokenCount < 5) {
            // Malformed record detected: skip to prevent corruption
            continue;
        }

        std::string pId = tokens[0];
        std::string barcode = tokens[1];
        std::string name = tokens[2];
        std::string desc = tokens[3];

        double price = 0.0;
        try {
            price = std::stod(tokens[4]);
        } catch (...) {
            // Malformed price field: skip corrupted record
            continue;
        }

        int stock = 0;
        if (tokenCount >= 6) {
            try {
                stock = std::stoi(tokens[5]);
                if (stock < 0) stock = 0;
            } catch (...) {
                stock = 0;
            }
        }

        if (productCount < maxCapacity) {
            products[productCount] = Product(pId, barcode, name, desc, price, stock);
            productCount++;
        }
    }

    file.close();
    return true;
}

bool ProductFileHandler::saveProducts(const std::string& filePath, const Product products[], int productCount, bool includeStock) {
    // Write to a temporary file first to prevent accidental corruption
    std::string tempPath = filePath + ".tmp";
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < productCount; i++) {
        file << products[i].getProductId() << " "
             << products[i].getBarcode() << " "
             << formatField(products[i].getProductName()) << " "
             << formatField(products[i].getDescription()) << " "
             << std::fixed << std::setprecision(2) << products[i].getPrice();

        if (includeStock) {
            file << " " << products[i].getStockQuantity();
        }

        file << " #\n";
    }

    file.close();

    // Replace original file safely
    std::remove(filePath.c_str());
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
        // If rename failed, try direct file stream write as fallback
        std::ofstream fallback(filePath);
        if (!fallback.is_open()) {
            return false;
        }
        for (int i = 0; i < productCount; i++) {
            fallback << products[i].getProductId() << " "
                     << products[i].getBarcode() << " "
                     << formatField(products[i].getProductName()) << " "
                     << formatField(products[i].getDescription()) << " "
                     << std::fixed << std::setprecision(2) << products[i].getPrice();
            if (includeStock) {
                fallback << " " << products[i].getStockQuantity();
            }
            fallback << " #\n";
        }
        fallback.close();
        std::remove(tempPath.c_str());
    }

    return true;
}

bool ProductFileHandler::loadInventory(const std::string& filePath, Product products[], int productCount) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue;
        }

        std::string tokens[4];
        int count = splitSpaceQuoted(cleaned, tokens, 4);
        if (count >= 2) {
            std::string identifier = tokens[0];
            int stock = 0;
            try {
                stock = std::stoi(tokens[1]);
                if (stock < 0) stock = 0;
            } catch (...) {
                continue;
            }

            for (int i = 0; i < productCount; i++) {
                if (products[i].getProductId() == identifier || products[i].getBarcode() == identifier) {
                    products[i].setStockQuantity(stock);
                    break;
                }
            }
        }
    }

    file.close();
    return true;
}

bool ProductFileHandler::saveInventory(const std::string& filePath, const Product products[], int productCount) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < productCount; i++) {
        file << products[i].getProductId() << " "
             << products[i].getStockQuantity() << " #\n";
    }

    file.close();
    return true;
}
