#include "../../include/product/Category.h"

Category::Category() {
    categoryId = "";
    name = "";
    acronym = "";
}

Category::Category(const std::string& id, const std::string& categoryName, const std::string& categoryAcronym) {
    categoryId = id;
    name = categoryName;
    acronym = categoryAcronym;
}

std::string Category::getCategoryId() const {
    return categoryId;
}

std::string Category::getName() const {
    return name;
}

std::string Category::getAcronym() const {
    return acronym;
}

void Category::setCategoryId(const std::string& id) {
    categoryId = id;
}

void Category::setName(const std::string& categoryName) {
    name = categoryName;
}

void Category::setAcronym(const std::string& categoryAcronym) {
    acronym = categoryAcronym;
}
