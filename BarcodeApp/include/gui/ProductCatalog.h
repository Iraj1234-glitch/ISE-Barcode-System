#pragma once

#include <cstring>
#include <vector>
#include <algorithm>

// Maximum number of products the GUI keeps in memory
const int MAX_CATALOG_PRODUCTS = 500;

// One product as the GUI needs it (lookup, display, billing)
struct ProductInfo
{
    std::string productId;
    std::string barcode;          // exactly 6 digits, first digit not zero
    std::string acronym;          // product acronym (max 8 chars)
    std::string name;
    std::string measurementUnit;  // count or weight
    std::string description;
    double      price = 0.0;
    double      tax = 0.0;        // tax in PKR
    double      discount = 0.0;   // discount per unit
    int         stock = 0;        // available quantity
    std::string barcodeImagePath; // barcode image path
    std::string categoryId;       // two‑digit category ID (optional)
    int         sold = 0;         // total units sold (used by reports)
};

// Thin adapter between the GUI and the product data.
class ProductCatalog
{
private:
    ProductInfo items[MAX_CATALOG_PRODUCTS];
    int         count;
    std::string filePath;

public:
    explicit ProductCatalog(const std::string& path = "Product.txt");

    bool load();
    int  getCount() const;

    const ProductInfo* findByBarcode(const std::string& barcode) const;
    const ProductInfo* findById(const std::string& productId) const;

    // ---------------------------------------------------------------
    //  NEW PUBLIC HELPERS – used by manager reports
    // ---------------------------------------------------------------
    struct ReportInfo {
        std::string name;
        int         sold;   // total units sold
        int         stock;  // remaining inventory
    };

    // Return top‑N products by total units sold
    std::vector<ReportInfo> topNSold(std::size_t n) const
    {
        std::vector<ReportInfo> out;
        for (int i = 0; i < count; ++i) {
            const ProductInfo& p = items[i];
            out.push_back({p.name, p.sold, p.stock});
        }
        std::sort(out.begin(), out.end(),
                  [](const ReportInfo& a, const ReportInfo& b){ return a.sold > b.sold; });
        if (out.size() > n) out.resize(n);
        return out;
    }

    // Return top‑N products by remaining stock
    std::vector<ReportInfo> topNStock(std::size_t n) const
    {
        std::vector<ReportInfo> out;
        for (int i = 0; i < count; ++i) {
            const ProductInfo& p = items[i];
            out.push_back({p.name, p.sold, p.stock});
        }
        std::sort(out.begin(), out.end(),
                  [](const ReportInfo& a, const ReportInfo& b){ return a.stock > b.stock; });
        if (out.size() > n) out.resize(n);
        return out;
    }
};
