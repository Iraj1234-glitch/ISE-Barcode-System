#pragma once
#include "Category.h"
#include "CategoryFileHandler.h"
#include <string>

class CategoryManager {
private:
    Category categories[MAX_CATEGORIES];
    int categoryCount;
    std::string filePath;
    CategoryFileHandler fileHandler;

    int findIndexById(const std::string& categoryId) const;

public:
    CategoryManager();
    explicit CategoryManager(const std::string& categoryFile);

    bool loadCategories();
    bool loadCategories(const std::string& path);
    bool saveCategories();
    bool saveCategories(const std::string& path);

    int getCategoryCount() const;
    const Category* getCategoryAt(int index) const;
    const Category* findCategoryById(const std::string& categoryId) const;

    // Returns an empty string on success, otherwise a validation error message.
    std::string addCategory(const std::string& categoryId, const std::string& name, const std::string& acronym);
    void clear();
};
