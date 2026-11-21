#pragma once
#include "Category.h"
#include <string>

const int MAX_CATEGORIES = 50;

class CategoryFileHandler {
public:
    static bool isValidCategoryId(const std::string& categoryId);

    bool loadCategories(const std::string& filePath, Category categories[], int& categoryCount, int maxCapacity = MAX_CATEGORIES);
    bool saveCategories(const std::string& filePath, const Category categories[], int categoryCount);
};
