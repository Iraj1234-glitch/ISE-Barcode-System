#include "../../include/product/CategoryManager.h"
#include "../../include/product/ProductFileHandler.h"

CategoryManager::CategoryManager() {
    categoryCount = 0;
    filePath = "PCategory.txt";
}

CategoryManager::CategoryManager(const std::string& categoryFile) {
    categoryCount = 0;
    filePath = categoryFile;
}

int CategoryManager::findIndexById(const std::string& categoryId) const {
    for (int i = 0; i < categoryCount; i++) {
        if (categories[i].getCategoryId() == categoryId) {
            return i;
        }
    }
    return -1;
}

bool CategoryManager::loadCategories() {
    return loadCategories(filePath);
}

bool CategoryManager::loadCategories(const std::string& path) {
    filePath = path;
    return fileHandler.loadCategories(path, categories, categoryCount, MAX_CATEGORIES);
}

bool CategoryManager::saveCategories() {
    return saveCategories(filePath);
}

bool CategoryManager::saveCategories(const std::string& path) {
    filePath = path;
    return fileHandler.saveCategories(path, categories, categoryCount);
}

int CategoryManager::getCategoryCount() const {
    return categoryCount;
}

const Category* CategoryManager::getCategoryAt(int index) const {
    if (index >= 0 && index < categoryCount) {
        return &categories[index];
    }
    return nullptr;
}

const Category* CategoryManager::findCategoryById(const std::string& categoryId) const {
    int idx = findIndexById(categoryId);
    if (idx == -1) {
        return nullptr;
    }
    return &categories[idx];
}

std::string CategoryManager::addCategory(const std::string& categoryId, const std::string& name, const std::string& acronym) {
    std::string id = ProductFileHandler::trim(categoryId);
    std::string categoryName = ProductFileHandler::trim(name);
    std::string categoryAcronym = ProductFileHandler::trim(acronym);

    if (!CategoryFileHandler::isValidCategoryId(id)) {
        return "Category ID must be exactly two digits and the first digit cannot be 0.";
    }
    if (categoryName.empty()) {
        return "Category name is required.";
    }
    if (categoryAcronym.empty()) {
        return "Category acronym is required.";
    }
    if (findIndexById(id) != -1) {
        return "Duplicate category ID is not allowed.";
    }
    if (categoryCount >= MAX_CATEGORIES) {
        return "Category list is full.";
    }

    categories[categoryCount++] = Category(id, categoryName, categoryAcronym);
    return "";
}

void CategoryManager::clear() {
    categoryCount = 0;
}
