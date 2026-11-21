#include "../../include/product/CategoryFileHandler.h"
#include "../../include/product/ProductFileHandler.h"
#include <fstream>
#include <cctype>
#include <cstdio>

bool CategoryFileHandler::isValidCategoryId(const std::string& categoryId) {
    if (categoryId.length() != 2) {
        return false;
    }
    if (categoryId[0] < '0' || categoryId[0] > '9' || categoryId[1] < '0' || categoryId[1] > '9') {
        return false;
    }
    // Existing barcode rule: first digit cannot be zero.
    if (categoryId[0] == '0') {
        return false;
    }
    return true;
}

bool CategoryFileHandler::loadCategories(const std::string& filePath, Category categories[], int& categoryCount, int maxCapacity) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        categoryCount = 0;
        return false;
    }

    categoryCount = 0;
    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = ProductFileHandler::cleanLine(line);
        if (cleaned.empty()) {
            continue;
        }

        std::string tokens[10];
        int tokenCount = ProductFileHandler::splitSpaceQuoted(cleaned, tokens, 10);
        if (tokenCount < 3) {
            continue;
        }
        if (!isValidCategoryId(tokens[0])) {
            continue;
        }
        if (tokens[1].empty() || tokens[2].empty()) {
            continue;
        }

        bool duplicate = false;
        for (int i = 0; i < categoryCount; i++) {
            if (categories[i].getCategoryId() == tokens[0]) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            continue;
        }

        if (categoryCount < maxCapacity) {
            categories[categoryCount] = Category(tokens[0], tokens[1], tokens[2]);
            categoryCount++;
        }
    }

    file.close();
    return true;
}

bool CategoryFileHandler::saveCategories(const std::string& filePath, const Category categories[], int categoryCount) {
    std::string tempPath = filePath + ".tmp";
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        return false;
    }

    for (int i = 0; i < categoryCount; i++) {
        file << categories[i].getCategoryId() << " "
             << ProductFileHandler::formatField(categories[i].getName()) << " "
             << ProductFileHandler::formatField(categories[i].getAcronym())
             << " #\n";
    }
    file.close();

    std::remove(filePath.c_str());
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
        std::ofstream fallback(filePath);
        if (!fallback.is_open()) {
            return false;
        }
        for (int i = 0; i < categoryCount; i++) {
            fallback << categories[i].getCategoryId() << " "
                     << ProductFileHandler::formatField(categories[i].getName()) << " "
                     << ProductFileHandler::formatField(categories[i].getAcronym())
                     << " #\n";
        }
        fallback.close();
        std::remove(tempPath.c_str());
    }
    return true;
}
