#include "../BarcodeApp/include/product/Product.h"
#include "../BarcodeApp/include/product/ProductFileHandler.h"
#include "../BarcodeApp/include/product/ProductManager.h"
#include "../BarcodeApp/include/product/CategoryManager.h"
#include "../BarcodeApp/include/product/ManagerProductService.h"
#include "../BarcodeApp/include/generation/BarcodeGenerator.h"
#include "../BarcodeApp/include/order/Order.h"
#include "../BarcodeApp/include/order/OrderManager.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <string>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using namespace std;

#define TEST_ASSERT(cond, msg) \
    if (!(cond)) { \
        std::cerr << "FAIL: " << msg << std::endl; \
        return false; \
    } else { \
        std::cout << "PASS: " << msg << std::endl; \
    }

static void ensureDir(const string& path) {
#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0755);
#endif
}

bool testCategories() {
    cout << "\n--- Category tests ---" << endl;
    const string catFile = "tests/tmp_PCategory.txt";
    std::remove(catFile.c_str());

    CategoryManager cm(catFile);
    string err = cm.addCategory("10", "Dairy", "DRY");
    TEST_ASSERT(err.empty(), "Add a valid category");
    TEST_ASSERT(cm.saveCategories(), "Save it to PCategory.txt");

    CategoryManager loaded(catFile);
    TEST_ASSERT(loaded.loadCategories(), "Load it again");
    TEST_ASSERT(loaded.getCategoryCount() == 1, "Loaded one category");
    const Category* c = loaded.findCategoryById("10");
    TEST_ASSERT(c != nullptr, "Category 10 found");
    TEST_ASSERT(c->getCategoryId() == "10", "Category Id field");
    TEST_ASSERT(c->getName() == "Dairy", "Category Name field");
    TEST_ASSERT(c->getAcronym() == "DRY", "Category Acronym field");

    string tooShort = cm.addCategory("1", "X", "X");
    TEST_ASSERT(!tooShort.empty(), "Reject a category ID with fewer than two digits");
    string tooLong = cm.addCategory("100", "X", "X");
    TEST_ASSERT(!tooLong.empty(), "Reject a category ID with more than two digits");
    string nonDigit = cm.addCategory("1A", "X", "X");
    TEST_ASSERT(!nonDigit.empty(), "Reject a non-digit category ID");
    string leadingZero = cm.addCategory("01", "X", "X");
    TEST_ASSERT(!leadingZero.empty(), "Reject a category ID whose first digit is 0");
    string duplicate = cm.addCategory("10", "Other", "OTH");
    TEST_ASSERT(!duplicate.empty(), "Reject duplicate category IDs");

    err = cm.addCategory("11", "Fresh Produce", "FR PR");
    TEST_ASSERT(err.empty(), "Add category whose acronym contains a space");
    TEST_ASSERT(cm.saveCategories(), "Save category with spaced acronym");

    ifstream check(catFile);
    string line1, line2;
    getline(check, line1);
    getline(check, line2);
    check.close();
    TEST_ASSERT(!line1.empty() && line1.back() == '#', "Verify records end with #");
    TEST_ASSERT(!line2.empty() && line2.back() == '#', "Second record ends with #");

    CategoryManager spaced(catFile);
    TEST_ASSERT(spaced.loadCategories(), "Reload categories with spaces");
    const Category* produce = spaced.findCategoryById("11");
    TEST_ASSERT(produce != nullptr && produce->getName() == "Fresh Produce", "Verify names with spaces parse correctly");
    TEST_ASSERT(produce->getAcronym() == "FR PR", "Verify acronyms with spaces parse correctly");

    std::remove(catFile.c_str());
    return true;
}

bool testBarcodes() {
    cout << "\n--- Barcode tests ---" << endl;
    const string prodFile = "tests/tmp_barcode_products.txt";
    const string catFile = "tests/tmp_barcode_categories.txt";
    const string imgDir = "tests/tmp_barcodes";
    std::remove(prodFile.c_str());
    std::remove(catFile.c_str());
    ensureDir(imgDir);

    ProductManager pm(prodFile);
    CategoryManager cm(catFile);
    ManagerProductService service(pm, cm, imgDir);
    TEST_ASSERT(service.addCategory("10", "Dairy", "DRY").empty(), "Configure category 10");

    Product product;
    string genErr = service.generateBarcodeForProduct(product, "10");
    TEST_ASSERT(genErr.empty(), "Generate a barcode using a configured category");
    TEST_ASSERT(product.getBarcode().substr(0, 2) == "10", "First two digits match selected category ID");
    TEST_ASSERT(ProductManager::isValidBarcodeFormat(product.getBarcode()), "Generated barcode is valid");
    TEST_ASSERT(product.getBarcode() == "100001", "First barcode in empty catalog is 100001");

    Product second;
    TEST_ASSERT(service.generateBarcodeForProduct(second, "10").empty(), "Generate a second barcode");
    TEST_ASSERT(second.getBarcode() != product.getBarcode(), "Generated barcode is unique");
    TEST_ASSERT(second.getBarcode().substr(0, 2) == "10", "Second barcode still uses category 10");

    ifstream img(product.getBarcodeImagePath().c_str(), ios::binary);
    TEST_ASSERT(img.good(), "Barcode image file exists");
    char header[2] = {0, 0};
    img.read(header, 2);
    TEST_ASSERT(header[0] == 'B' && header[1] == 'M', "Image saved as BMP using existing convention");
    img.close();
    TEST_ASSERT(product.getBarcodeImagePath().find(product.getBarcode()) != string::npos, "Image path stores the barcode value");

    BarcodeGenerator generator;
    string existingPath = imgDir + "/200099.bmp";
    TEST_ASSERT(generator.generateBarcode("200099", existingPath), "Existing barcode generation still works");

    std::remove(prodFile.c_str());
    std::remove(catFile.c_str());
    std::remove(product.getBarcodeImagePath().c_str());
    std::remove(second.getBarcodeImagePath().c_str());
    std::remove(existingPath.c_str());
    return true;
}

bool testProducts() {
    cout << "\n--- Product tests ---" << endl;
    const string prodFile = "tests/tmp_inc3_products.txt";
    const string catFile = "tests/tmp_inc3_categories.txt";
    const string imgDir = "tests/tmp_barcodes";
    std::remove(prodFile.c_str());
    std::remove(catFile.c_str());
    ensureDir(imgDir);

    ProductManager pm(prodFile);
    CategoryManager cm(catFile);
    ManagerProductService service(pm, cm, imgDir);
    TEST_ASSERT(service.addCategory("10", "Dairy", "DRY").empty(), "Category available for products");

    Product milk;
    milk.setProductId("P101");
    milk.setAcronym("MLK1L");
    milk.setProductName("Farm Milk 1L");
    milk.setMeasurementUnit("count");
    milk.setPrice(280.00);
    milk.setTax(0.00);
    milk.setDiscount(5.00);
    milk.setStockQuantity(20);
    milk.setDescription("Fresh milk with spaces in the name");
    TEST_ASSERT(service.generateBarcodeForProduct(milk, "10").empty(), "Generate barcode for new product");
    string addErr = service.addNewProduct(milk);
    TEST_ASSERT(addErr.empty(), "Add a new product");

    ProductManager reloaded(prodFile);
    TEST_ASSERT(reloaded.loadProducts(), "Product.txt remains readable after saving");
    const Product* loaded = reloaded.findProductById("P101");
    TEST_ASSERT(loaded != nullptr, "Added product found");
    TEST_ASSERT(loaded->getCategoryId() == "10", "Category ID is stored correctly");
    TEST_ASSERT(loaded->getProductId() == "P101", "Product ID preserved");
    TEST_ASSERT(loaded->getBarcode() == milk.getBarcode(), "Barcode preserved");
    TEST_ASSERT(loaded->getAcronym() == "MLK1L", "Acronym preserved");
    TEST_ASSERT(loaded->getProductName() == "Farm Milk 1L", "Product names containing spaces load correctly");
    TEST_ASSERT(loaded->getMeasurementUnit() == "count", "Measurement unit preserved");
    TEST_ASSERT(std::abs(loaded->getPrice() - 280.00) < 0.001, "Unit price preserved");
    TEST_ASSERT(std::abs(loaded->getTax() - 0.00) < 0.001, "Tax preserved");
    TEST_ASSERT(std::abs(loaded->getDiscount() - 5.00) < 0.001, "Discount preserved");
    TEST_ASSERT(loaded->getStockQuantity() == 20, "Available quantity preserved");
    TEST_ASSERT(loaded->getBarcodeImagePath() == milk.getBarcodeImagePath(), "Barcode image path stored correctly");
    TEST_ASSERT(loaded->getDescription() == "Fresh milk with spaces in the name", "Description preserved");

    ifstream saved(prodFile);
    string savedLine;
    getline(saved, savedLine);
    saved.close();
    TEST_ASSERT(!savedLine.empty() && savedLine.back() == '#', "Saved product record ends with #");

    Product edited = *loaded;
    edited.setPrice(300.00);
    edited.setStockQuantity(18);
    TEST_ASSERT(service.editExistingProduct("P101", edited).empty(), "Edit an existing product");

    ProductManager afterEdit(prodFile);
    afterEdit.loadProducts();
    const Product* e = afterEdit.findProductById("P101");
    TEST_ASSERT(e != nullptr, "Edited product reloaded");
    TEST_ASSERT(std::abs(e->getPrice() - 300.00) < 0.001, "Only intended price changed");
    TEST_ASSERT(e->getStockQuantity() == 18, "Only intended stock changed");
    TEST_ASSERT(e->getProductName() == "Farm Milk 1L", "Name not lost");
    TEST_ASSERT(e->getAcronym() == "MLK1L", "Acronym not lost");
    TEST_ASSERT(e->getCategoryId() == "10", "Category ID not lost");
    TEST_ASSERT(e->getBarcode() == milk.getBarcode(), "Barcode not lost");
    TEST_ASSERT(std::abs(e->getDiscount() - 5.00) < 0.001, "Discount not lost");
    TEST_ASSERT(e->getDescription() == "Fresh milk with spaces in the name", "Description not lost");

    Product dupId = edited;
    dupId.setBarcode("109999");
    TEST_ASSERT(!service.addNewProduct(dupId).empty(), "Duplicate product ID rejected");

    Product dupCode;
    dupCode.setProductId("P102");
    dupCode.setAcronym("X");
    dupCode.setProductName("Other");
    dupCode.setMeasurementUnit("count");
    dupCode.setPrice(1.0);
    dupCode.setBarcode(e->getBarcode());
    dupCode.setCategoryId("10");
    dupCode.setBarcodeImagePath(e->getBarcodeImagePath());
    TEST_ASSERT(!service.addNewProduct(dupCode).empty(), "Duplicate barcode rejected");

    std::remove(prodFile.c_str());
    std::remove(catFile.c_str());
    std::remove(milk.getBarcodeImagePath().c_str());
    return true;
}

bool testRegression() {
    cout << "\n--- Increment 2 regression tests ---" << endl;
    ProductManager pm("BarcodeApp/Product.txt");
    TEST_ASSERT(pm.loadProducts(), "Existing products still load");
    TEST_ASSERT(pm.getProductCount() == 6, "All 6 existing products load");
    TEST_ASSERT(pm.findProductByBarcode("100001") != nullptr, "Existing product lookup by barcode still works");
    TEST_ASSERT(pm.getProductDescriptionByBarcode("100001").find("Fresh full-cream milk") != string::npos,
                "Product descriptions still work");
    TEST_ASSERT(pm.findProductByBarcode("100002")->getProductName() == "Bread", "Bread lookup intact");
    TEST_ASSERT(std::abs(pm.findProductByBarcode("100005")->getTax() - 20.00) < 0.001, "Existing tax field intact");

    pm.findProductByBarcode("100001")->setStockQuantity(20);
    Order order("REG12345", "21/11/2026", "12:00:00", "Iraj", "03001234567",
                "in-progress", 560.00, "cash", "CashCounter");
    order.addProduct("P001", "100001", 2);
    TEST_ASSERT(pm.processOrderDeduction(order), "Existing order workflow is not broken");
    TEST_ASSERT(pm.getStockByBarcode("100001") == 18, "Existing checkout stock deduction still works");
    TEST_ASSERT(pm.restoreOrderStock(order), "Existing stock restoration still works");

    TEST_ASSERT(ProductManager::isValidBarcodeFormat("100001"), "Existing barcode format rule unchanged");
    return true;
}

int main() {
    cout << "========================================" << endl;
    cout << "INCREMENT 3 MEMBER 1 TESTS (Req 1, 2, 6)" << endl;
    cout << "========================================" << endl;
    if (!testCategories()) return 1;
    if (!testBarcodes()) return 1;
    if (!testProducts()) return 1;
    if (!testRegression()) return 1;
    cout << "\nALL INCREMENT 3 MEMBER 1 TESTS PASSED" << endl;
    return 0;
}
