#include "../../include/product/ManagerProductService.h"
#include "../../include/product/ProductFileHandler.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static void ensureDirectoryExists(const std::string& path) {
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}

ManagerProductService::ManagerProductService(ProductManager& products, CategoryManager& categories, const std::string& barcodeDir) {
    productManager = &products;
    categoryManager = &categories;
    barcodeDirectory = barcodeDir;
    reservedBarcodeCount = 0;
}

bool ManagerProductService::isBarcodeReserved(const std::string& barcode) const {
    for (int i = 0; i < reservedBarcodeCount; i++) {
        if (reservedBarcodes[i] == barcode) {
            return true;
        }
    }
    return false;
}

void ManagerProductService::reserveBarcode(const std::string& barcode) {
    if (barcode.empty() || isBarcodeReserved(barcode) || reservedBarcodeCount >= MAX_PRODUCTS) {
        return;
    }
    reservedBarcodes[reservedBarcodeCount++] = barcode;
}

std::string ManagerProductService::validateProductFields(const Product& product, bool requireExistingCategory) const {
    if (ProductFileHandler::trim(product.getProductId()).empty()) {
        return "Product ID is required.";
    }
    if (!ProductManager::isValidBarcodeFormat(product.getBarcode())) {
        return "Product barcode must be exactly 6 digits and must not start with 0.";
    }
    if (product.getAcronym().length() > 8) {
        return "Product acronym cannot exceed 8 characters.";
    }
    if (ProductFileHandler::trim(product.getProductName()).empty()) {
        return "Product name is required.";
    }
    if (product.getMeasurementUnit() != "count" && product.getMeasurementUnit() != "weight") {
        return "Measurement unit must be count or weight.";
    }
    if (product.getPrice() < 0.0 || product.getTax() < 0.0 || product.getDiscount() < 0.0) {
        return "Price, tax and discount cannot be negative.";
    }
    if (product.getStockQuantity() < 0) {
        return "Available quantity cannot be negative.";
    }
    if (product.getCategoryId().empty()) {
        return "A product category must be selected.";
    }
    if (!CategoryFileHandler::isValidCategoryId(product.getCategoryId())) {
        return "Category ID must be exactly two digits and the first digit cannot be 0.";
    }
    if (requireExistingCategory && categoryManager->findCategoryById(product.getCategoryId()) == nullptr) {
        return "Selected category is not configured.";
    }
    if (product.getBarcode().substr(0, 2) != product.getCategoryId()) {
        return "Barcode first two digits must match the selected category ID.";
    }
    return "";
}

std::string ManagerProductService::addCategory(const std::string& categoryId, const std::string& name, const std::string& acronym) {
    std::string error = categoryManager->addCategory(categoryId, name, acronym);
    if (!error.empty()) {
        return error;
    }
    if (!categoryManager->saveCategories()) {
        return "Category was added in memory but could not be saved to PCategory.txt.";
    }
    return "";
}

std::string ManagerProductService::generateUniqueBarcode(const std::string& categoryId) const {
    std::string base = productManager->generateUniqueBarcodeForCategory(categoryId);
    if (base.empty() || !isBarcodeReserved(base)) {
        return base;
    }
    ProductManager scratch;
    for (int i = 0; i < productManager->getProductCount(); i++) {
        scratch.addProduct(*productManager->getProductAt(i));
    }
    for (int i = 0; i < reservedBarcodeCount; i++) {
        Product placeholder;
        placeholder.setProductId(std::string("R") + std::to_string(i));
        placeholder.setBarcode(reservedBarcodes[i]);
        scratch.addProduct(placeholder);
    }
    return scratch.generateUniqueBarcodeForCategory(categoryId);
}

std::string ManagerProductService::generateBarcodeForProduct(Product& product, const std::string& categoryId) {
    if (!CategoryFileHandler::isValidCategoryId(categoryId)) {
        return "Configure a valid two-digit category before generating a barcode.";
    }
    if (categoryManager->findCategoryById(categoryId) == nullptr) {
        return "Selected category is not configured.";
    }

    std::string code = generateUniqueBarcode(categoryId);
    if (code.empty()) {
        return "Could not generate a unique barcode for this category.";
    }
    reserveBarcode(code);

    ensureDirectoryExists(barcodeDirectory);
    std::string outputPath = barcodeDirectory + "/" + code + ".bmp";

    if (!barcodeGenerator.generateBarcode(code, outputPath)) {
        return "Barcode image generation failed.";
    }

    product.setCategoryId(categoryId);
    product.setBarcode(code);
    product.setBarcodeImagePath(outputPath);
    return "";
}

std::string ManagerProductService::addNewProduct(const Product& product) {
    std::string error = validateProductFields(product, true);
    if (!error.empty()) {
        return error;
    }
    if (!productManager->addProduct(product)) {
        return "Duplicate product ID or barcode is not allowed.";
    }
    if (!productManager->saveProducts()) {
        return "Product was added in memory but could not be saved to Product.txt.";
    }
    return "";
}

std::string ManagerProductService::editExistingProduct(const std::string& originalProductId, const Product& updatedProduct) {
    if (productManager->findProductById(originalProductId) == nullptr) {
        return "Product to edit was not found.";
    }
    std::string error = validateProductFields(updatedProduct, true);
    if (!error.empty()) {
        return error;
    }
    if (!productManager->updateProductById(originalProductId, updatedProduct)) {
        return "Could not update product. Check for duplicate product ID or barcode.";
    }
    if (!productManager->saveProducts()) {
        return "Product was updated in memory but could not be saved to Product.txt.";
    }
    return "";
}

bool ManagerProductService::saveAllProducts() {
    return productManager->saveProducts();
}

bool ManagerProductService::saveAllCategories() {
    return categoryManager->saveCategories();
}
