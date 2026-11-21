#pragma once
#include "Product.h"
#include "ProductManager.h"
#include "CategoryManager.h"
#include "../generation/BarcodeGenerator.h"
#include <string>

// Non-GUI manager operations for categories, products and category-prefixed barcodes.
class ManagerProductService {
private:
    ProductManager* productManager;
    CategoryManager* categoryManager;
    BarcodeGenerator barcodeGenerator;
    std::string barcodeDirectory;
    std::string reservedBarcodes[MAX_PRODUCTS];
    int reservedBarcodeCount;

    std::string validateProductFields(const Product& product, bool requireExistingCategory) const;
    bool isBarcodeReserved(const std::string& barcode) const;
    void reserveBarcode(const std::string& barcode);

public:
    ManagerProductService(ProductManager& products, CategoryManager& categories, const std::string& barcodeDir = "barcodes");

    std::string addCategory(const std::string& categoryId, const std::string& name, const std::string& acronym);

    std::string generateUniqueBarcode(const std::string& categoryId) const;
    std::string generateBarcodeForProduct(Product& product, const std::string& categoryId);

    std::string addNewProduct(const Product& product);
    std::string editExistingProduct(const std::string& originalProductId, const Product& updatedProduct);

    bool saveAllProducts();
    bool saveAllCategories();
};
