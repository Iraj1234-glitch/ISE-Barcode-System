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
    if (str.empty()) {
        return "\"\"";
    }
    if (str.find(' ') != std::string::npos || str.find('\t') != std::string::npos) {
        return "\"" + str + "\"";
    }
    return str;
}

// Preserved descriptions for the Increment 1 product catalog
std::string ProductFileHandler::getPreservedDescription(const std::string& productId, const std::string& productName) {
    if (productId == "P001" || productName == "Milk 1L") {
        return "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.";
    }
    if (productId == "P002" || productName == "Bread") {
        return "Sliced white bread, 400 g loaf. Best before 3 days from purchase.";
    }
    if (productId == "P003" || productName == "Eggs (12)") {
        return "Farm eggs, one dozen in a protective tray.";
    }
    if (productId == "P004" || productName == "Rice 5kg") {
        return "Premium long-grain basmati rice in a 5 kg bag.";
    }
    if (productId == "P005" || productName == "Cooking Oil 1L") {
        return "Refined sunflower cooking oil, 1 litre bottle.";
    }
    if (productId == "P006" || productName == "Sugar 1kg") {
        return "Fine white sugar, 1 kg pack.";
    }
    return "";
}

// Authorized default tax amounts in PKR
double ProductFileHandler::getDefaultTaxForProduct(const std::string& productId) {
    if (productId == "P001") return 0.00; // Milk (staple exempt)
    if (productId == "P002") return 0.00; // Bread (staple exempt)
    if (productId == "P003") return 0.00; // Eggs (staple exempt)
    if (productId == "P004") return 0.00; // Rice (staple exempt)
    if (productId == "P005") return 20.00; // Cooking Oil (standard PKR tax)
    if (productId == "P006") return 10.00; // Sugar (standard PKR tax)
    return 0.00;
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

static bool looksLikeTwoDigitId(const std::string& value) {
    return value.length() == 2 &&
           value[0] >= '0' && value[0] <= '9' &&
           value[1] >= '0' && value[1] <= '9';
}

static void writeIncrementProductLine(std::ostream& file, const Product& product) {
    file << product.getProductId() << " "
         << product.getBarcode() << " "
         << ProductFileHandler::formatField(product.getAcronym()) << " "
         << ProductFileHandler::formatField(product.getProductName()) << " "
         << ProductFileHandler::formatField(product.getMeasurementUnit()) << " "
         << std::fixed << std::setprecision(2) << product.getPrice() << " "
         << std::fixed << std::setprecision(2) << product.getTax() << " "
         << std::fixed << std::setprecision(2) << product.getDiscount() << " "
         << product.getStockQuantity() << " "
         << ProductFileHandler::formatField(product.getBarcodeImagePath()) << " "
         << ProductFileHandler::formatField(product.getCategoryId());
    if (!product.getDescription().empty()) {
        file << " " << ProductFileHandler::formatField(product.getDescription());
    }
    file << " #\n";
}

bool ProductFileHandler::loadProducts(const std::string& filePath, Product products[], int& productCount, int maxCapacity) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
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

        std::string tokens[20];
        int tokenCount = splitSpaceQuoted(cleaned, tokens, 20);

        // Minimal check: requires at least 5 tokens
        if (tokenCount < 5) {
            continue;
        }

        if (tokenCount >= 10) {
            // ========================================================
            // Increment 2/3 Product Record
            // 1-10: Increment 2 fields
            // 11: Category ID (exactly two digits) — appended for Increment 3
            // 12: Optional description
            // Existing 10-field Increment 2 files remain valid.
            // ========================================================
            std::string pId = tokens[0];
            std::string barcode = tokens[1];
            std::string acronym = tokens[2];
            std::string name = tokens[3];
            std::string unit = tokens[4];

            double price = 0.0;
            try { price = std::stod(tokens[5]); } catch (...) { continue; }

            double tax = 0.0;
            try { tax = std::stod(tokens[6]); } catch (...) { tax = getDefaultTaxForProduct(pId); }

            double discount = 0.0;
            try { discount = std::stod(tokens[7]); } catch (...) { discount = 0.0; }

            int stock = 0;
            try { stock = std::stoi(tokens[8]); if (stock < 0) stock = 0; } catch (...) { stock = 0; }

            std::string barcodeImgPath = tokens[9];

            std::string catId = "";
            std::string extraDesc = "";
            if (tokenCount >= 11) {
                bool looksLikeCategory = looksLikeTwoDigitId(tokens[10]);
                if (looksLikeCategory) {
                    catId = tokens[10];
                    if (tokenCount >= 12) {
                        extraDesc = tokens[11];
                    }
                } else {
                    extraDesc = tokens[10];
                }
            }

            std::string desc = getPreservedDescription(pId, name);
            if (desc.empty() && !extraDesc.empty()) {
                desc = extraDesc;
            }

            if (productCount < maxCapacity) {
                products[productCount] = Product(pId, barcode, acronym, name, unit, price, tax, discount, stock, barcodeImgPath, desc, catId);
                productCount++;
            }
        } else {
            // ========================================================
            // Increment 1: Legacy 5 or 6-Field Product Record
            // 1. Product ID
            // 2. Barcode
            // 3. Name
            // 4. Description
            // 5. Price
            // 6. [Optional] Stock
            // ========================================================
            std::string pId = tokens[0];
            std::string barcode = tokens[1];
            std::string name = tokens[2];
            std::string desc = tokens[3];

            double price = 0.0;
            try { price = std::stod(tokens[4]); } catch (...) { continue; }

            int stock = 0;
            if (tokenCount >= 6) {
                try {
                    stock = std::stoi(tokens[5]);
                    if (stock < 0) stock = 0;
                } catch (...) {
                    stock = 0;
                }
            }

            double tax = getDefaultTaxForProduct(pId);

            if (productCount < maxCapacity) {
                products[productCount] = Product(pId, barcode, "", name, "", price, tax, 0.0, stock, "", desc);
                productCount++;
            }
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
        // If the product has Increment 2 fields populated, serialize all 10 fields
        bool hasIncrement2Fields = (!products[i].getAcronym().empty() || !products[i].getMeasurementUnit().empty() || !products[i].getBarcodeImagePath().empty());

        if (hasIncrement2Fields) {
            writeIncrementProductLine(file, products[i]);
        } else {
            // Legacy 5/6 fields format
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
    }

    file.close();

    // Replace original file safely
    std::remove(filePath.c_str());
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
        // Fallback direct write
        std::ofstream fallback(filePath);
        if (!fallback.is_open()) {
            return false;
        }
        for (int i = 0; i < productCount; i++) {
            bool hasIncrement2Fields = (!products[i].getAcronym().empty() || !products[i].getMeasurementUnit().empty() || !products[i].getBarcodeImagePath().empty());

            if (hasIncrement2Fields) {
                writeIncrementProductLine(fallback, products[i]);
            } else {
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
        if (cleaned.empty()) continue;

        std::string tokens[5];
        int count = splitSpaceQuoted(cleaned, tokens, 5);
        if (count < 2) continue;

        std::string pId = tokens[0];
        int stock = 0;
        try {
            stock = std::stoi(tokens[1]);
            if (stock < 0) stock = 0;
        } catch (...) {
            continue;
        }

        for (int i = 0; i < productCount; i++) {
            if (products[i].getProductId() == pId) {
                products[i].setStockQuantity(stock);
                break;
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
