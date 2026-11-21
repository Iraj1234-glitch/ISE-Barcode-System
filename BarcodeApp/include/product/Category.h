#pragma once
#include <string>

// One product category stored in PCategory.txt
// Record format (space-separated, quoted names/acronyms with spaces, ends with #):
//   CategoryId CategoryName CategoryAcronym #
// CategoryId is exactly two digits and is the first two digits of generated barcodes.
class Category {
private:
    std::string categoryId;
    std::string name;
    std::string acronym;

public:
    Category();
    Category(const std::string& id, const std::string& categoryName, const std::string& categoryAcronym);

    std::string getCategoryId() const;
    std::string getName() const;
    std::string getAcronym() const;

    void setCategoryId(const std::string& id);
    void setName(const std::string& categoryName);
    void setAcronym(const std::string& categoryAcronym);
};
