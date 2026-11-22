#include "../../include/billing/BillGenerator.h"
#include "../../include/billing/PDFGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <filesystem>

// ==========================================
// BillDetail Implementation
// ==========================================

BillDetail::BillDetail() {
    billId = "";
    productAcronym = "";
    unitPriceWithoutTax = 0.0;
    quantity = 0;
    productDiscount = 0.0;
    productName = "";
    lineTotal = 0.0;
}

BillDetail::BillDetail(const std::string& bId, const std::string& acronym, double unitPrice,
                       int qty, double discount, const std::string& name) {
    billId = bId;
    productAcronym = acronym;
    unitPriceWithoutTax = unitPrice;
    quantity = qty;
    productDiscount = discount;
    productName = name.empty() ? acronym : name;

    // Line total = (Unit Price * Quantity) - Discount
    lineTotal = (unitPriceWithoutTax * quantity) - productDiscount;
    if (lineTotal < 0.0) {
        lineTotal = 0.0;
    }
}

// ==========================================
// Bill Implementation
// ==========================================

Bill::Bill() {
    billId = "";
    customerName = "";
    orderId = "";
    totalWithoutTax = 0.0;
    totalTax = 0.0;
    totalApplicableDiscount = 0.0;
    totalPayable = 0.0;
    amountReceived = 0.0;
    balanceReturned = 0.0;
    paymentStatus = "in-progress";
    seasonalDiscount = 0.0;
    specialDiscount = 0.0;
    billDate = "";
    billTime = "";
    seasonalDiscountPercentage = 0.0;
    specialDiscountAmount = 0.0;
}

// ==========================================
// Helper String Functions
// ==========================================

std::string BillGenerator::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, (end - start + 1));
}

std::string BillGenerator::cleanLine(const std::string& line) {
    std::string cleaned = trim(line);
    // Remove trailing '#' delimiter if present
    if (!cleaned.empty() && cleaned.back() == '#') {
        cleaned.pop_back();
    }
    return trim(cleaned);
}

// Quotes a string if it contains spaces or tabs
std::string BillGenerator::formatField(const std::string& str) {
    if (str.empty() || str.find(' ') != std::string::npos || str.find('\t') != std::string::npos) {
        return "\"" + str + "\"";
    }
    return str;
}

// Parses a space-separated line into tokens, respecting double-quoted strings
int BillGenerator::splitSpaceQuoted(const std::string& line, std::string tokens[], int maxTokens) {
    int count = 0;
    std::string current = "";
    bool inQuotes = false;
    bool hadQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            hadQuotes = true;
        } else if ((c == ' ' || c == '\t') && !inQuotes) {
            if (!current.empty() || hadQuotes) {
                if (count < maxTokens) {
                    tokens[count++] = current;
                }
                current = "";
                hadQuotes = false;
            }
        } else {
            current += c;
        }
    }

    if (!current.empty() || hadQuotes) {
        if (count < maxTokens) {
            tokens[count++] = current;
        }
    }

    return count;
}

// ==========================================
// BillGenerator Implementation
// ==========================================

BillGenerator::BillGenerator() {
    billFilePath = "Bill.txt";
    detailFilePath = "BillDetail.txt";
    billsDirectory = "Bills/";
    seasonManager.loadSeasons("Seasons.txt");
}

BillGenerator::BillGenerator(const std::string& billFile, const std::string& detailFile, const std::string& seasonFile) {
    billFilePath = billFile;
    detailFilePath = detailFile;
    billsDirectory = "Bills/";
    seasonManager.loadSeasons(seasonFile);
}

SeasonManager& BillGenerator::getSeasonManager() {
    return seasonManager;
}

const SeasonManager& BillGenerator::getSeasonManager() const {
    return seasonManager;
}

// Checks if a 9-digit Bill ID already exists in Bill.txt
// Time Complexity: O(N) where N is the number of bills
bool BillGenerator::billIdExists(const std::string& billId) const {
    std::ifstream file(billFilePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) {
            continue;
        }

        std::string tokens[15];
        int count = splitSpaceQuoted(cleaned, tokens, 15);
        if (count > 0 && tokens[0] == billId) {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

// Generates a unique 9-digit Bill ID (e.g. 100000001, 100000002, ...)
// Time Complexity: O(N) where N is number of bills in Bill.txt
std::string BillGenerator::generateUniqueBillId() {
    long long highestId = 100000000LL;

    std::ifstream file(billFilePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::string cleaned = cleanLine(line);
            if (cleaned.empty()) continue;

            std::string tokens[15];
            int count = splitSpaceQuoted(cleaned, tokens, 15);
            if (count > 0 && tokens[0].length() == 9) {
                try {
                    long long current = std::stoll(tokens[0]);
                    if (current > highestId) {
                        highestId = current;
                    }
                } catch (...) {
                    // Ignore non-numeric IDs safely
                }
            }
        }
        file.close();
    }

    long long nextId = highestId + 1;
    std::string candidate = std::to_string(nextId);

    // Verify it doesn't collide, retry if needed
    while (billIdExists(candidate)) {
        nextId++;
        candidate = std::to_string(nextId);
    }

    return candidate;
}

// Checks if a bill has already been created for an Order ID
bool BillGenerator::hasBillForOrder(const std::string& orderId, std::string* outBillId) const {
    std::ifstream file(billFilePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) continue;

        std::string tokens[15];
        int count = splitSpaceQuoted(cleaned, tokens, 15);
        // Field 0: Bill ID, Field 2: Order ID
        if (count >= 3 && tokens[2] == orderId) {
            if (outBillId != nullptr) {
                *outBillId = tokens[0];
            }
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

// Calculates bill totals with clear intermediate variables (Requirement 10)
// Time Complexity: O(M) where M is the number of items in the order
bool BillGenerator::calculateBill(const Order& order,
                                 const ProductManager& productManager,
                                 double specialDiscountPercent,
                                 Bill& outBill,
                                 std::string& errorMsg,
                                 double customTaxRate) {
    // 1. Validate special discount percentage: STRICT 8% LIMIT (Requirement 9/10)
    if (specialDiscountPercent < 0.0) {
        errorMsg = "Special discount percentage cannot be negative.";
        return false;
    }
    if (specialDiscountPercent > 8.0) {
        errorMsg = "Special discount percentage cannot exceed 8.00% (Strict Sir Rule).";
        return false;
    }

    // 2. Order must contain at least one product
    if (order.getDetailCount() <= 0) {
        errorMsg = "Cannot generate bill: Order contains no products.";
        return false;
    }

    // Reset bill fields
    outBill = Bill();
    outBill.orderId = order.getOrderId();
    outBill.customerName = order.getCustomerName();
    outBill.paymentStatus = order.getOrderStatus();
    outBill.billDate = SeasonManager::getCurrentDate();
    outBill.billTime = SeasonManager::getCurrentTime();
    outBill.specialDiscount = specialDiscountPercent;

    double sumMerchandiseWithoutTax = 0.0;
    double sumProductDiscounts = 0.0;

    // 3. Process each purchased product in the order (Step A & Step C)
    for (int i = 0; i < order.getDetailCount(); ++i) {
        const OrderDetail& detail = order.getDetail(i);
        std::string prodId = detail.getProductId();
        std::string prodBarcode = detail.getProductBarcode();
        int qty = detail.getQuantity();

        if (qty <= 0) {
            continue;
        }

        // Look up product in catalog by ID or Barcode
        const Product* prod = productManager.findProductById(prodId);
        if (prod == nullptr && !prodBarcode.empty()) {
            prod = productManager.findProductByBarcode(prodBarcode);
        }

        double unitPrice = 0.0;
        double unitDiscount = 0.0;
        std::string acronym = prodId;
        std::string prodName = "";

        if (prod != nullptr) {
            unitPrice = prod->getPrice();
            prodName = prod->getProductName();
            if (!prod->getAcronym().empty()) {
                acronym = prod->getAcronym();
            } else if (acronym.empty()) {
                acronym = prod->getProductId();
            }
            unitDiscount = prod->getDiscount();
        }
        double lineDiscount = unitDiscount * qty;

        // Line merchandise total before tax
        double lineAmountWithoutTax = unitPrice * qty;

        sumMerchandiseWithoutTax += lineAmountWithoutTax;
        sumProductDiscounts += lineDiscount;

        // Build itemized detail record
        BillDetail item("", acronym, unitPrice, qty, lineDiscount, prodName);
        outBill.items.push_back(item);
    }

    // A. Total bill without tax
    outBill.totalWithoutTax = sumMerchandiseWithoutTax;

    // B. Total tax
    // Uses provided tax rate or zero if none configured
    double calculatedTax = 0.0;
    if (customTaxRate > 0.0) {
        calculatedTax = (sumMerchandiseWithoutTax * customTaxRate) / 100.0;
    }
    outBill.totalTax = std::round(calculatedTax * 100.0) / 100.0;

    // C. Total applicable discount
    outBill.totalApplicableDiscount = sumProductDiscounts;

    // D. Seasonal discount (Requirement 15)
    // Applied to merchandise amount remaining after product discounts
    double merchandiseAfterProductDiscount = sumMerchandiseWithoutTax - sumProductDiscounts;
    if (merchandiseAfterProductDiscount < 0.0) {
        merchandiseAfterProductDiscount = 0.0;
    }

    double seasonalPercent = seasonManager.getActiveSeasonDiscount(outBill.billDate);
    outBill.seasonalDiscountPercentage = seasonalPercent;

    double seasonalDiscountAmount = (merchandiseAfterProductDiscount * seasonalPercent) / 100.0;
    seasonalDiscountAmount = std::round(seasonalDiscountAmount * 100.0) / 100.0;
    outBill.seasonalDiscount = seasonalDiscountAmount;

    // E. Special discount (Strict 8% limit applied to amount after seasonal discount)
    double remainingAfterSeasonal = merchandiseAfterProductDiscount - seasonalDiscountAmount;
    if (remainingAfterSeasonal < 0.0) {
        remainingAfterSeasonal = 0.0;
    }

    double specialDiscountAmount = (remainingAfterSeasonal * specialDiscountPercent) / 100.0;
    specialDiscountAmount = std::round(specialDiscountAmount * 100.0) / 100.0;
    outBill.specialDiscountAmount = specialDiscountAmount;

    // F. Total payable bill
    double finalPayable = outBill.totalWithoutTax + outBill.totalTax
                        - outBill.totalApplicableDiscount
                        - outBill.seasonalDiscount
                        - outBill.specialDiscountAmount;

    finalPayable = std::round(finalPayable * 100.0) / 100.0;
    if (finalPayable < 0.0) {
        finalPayable = 0.0;
    }
    outBill.totalPayable = finalPayable;

    // Amount received and balance returned
    outBill.amountReceived = order.getPaymentAmount();
    if (outBill.amountReceived >= outBill.totalPayable) {
        outBill.balanceReturned = std::round((outBill.amountReceived - outBill.totalPayable) * 100.0) / 100.0;
    } else {
        outBill.balanceReturned = 0.0;
    }

    return true;
}

// Saves exactly 14 fields to Bill.txt ending with '#' (Requirement 8)
// Format:
// BillID CustomerName OrderID TotalWithoutTax TotalTax TotalApplicableDiscount
// TotalPayable AmountReceived BalanceReturned PaymentStatus SeasonalDiscount
// SpecialDiscount BillDate BillTime#
bool BillGenerator::saveBill(const Bill& bill) {
    std::ofstream file(billFilePath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << billFilePath << " for writing.\n";
        return false;
    }

    file << bill.billId << " "
         << formatField(bill.customerName) << " "
         << bill.orderId << " "
         << std::fixed << std::setprecision(2)
         << bill.totalWithoutTax << " "
         << bill.totalTax << " "
         << bill.totalApplicableDiscount << " "
         << bill.totalPayable << " "
         << bill.amountReceived << " "
         << bill.balanceReturned << " "
         << bill.paymentStatus << " "
         << bill.seasonalDiscount << " "
         << bill.specialDiscount << " "
         << bill.billDate << " "
         << bill.billTime << "#\n";

    file.close();
    return true;
}

// Saves exactly 5 fields per item to BillDetail.txt ending with '#' (Requirement 9)
// Format:
// BillID ProductAcronym UnitPriceWithoutTax Quantity ProductDiscount#
bool BillGenerator::saveBillDetails(const Bill& bill) {
    std::ofstream file(detailFilePath, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << detailFilePath << " for writing.\n";
        return false;
    }

    for (size_t i = 0; i < bill.items.size(); ++i) {
        file << bill.billId << " "
             << bill.items[i].productAcronym << " "
             << std::fixed << std::setprecision(2)
             << bill.items[i].unitPriceWithoutTax << " "
             << bill.items[i].quantity << " "
             << bill.items[i].productDiscount << "#\n";
    }

    file.close();
    return true;
}

// Loads all bills from Bill.txt
bool BillGenerator::loadBills(std::vector<Bill>& outBills) const {
    outBills.clear();

    std::ifstream file(billFilePath);
    if (!file.is_open()) {
        return true; // Missing file treated safely as 0 bills
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) continue;

        std::string tokens[16];
        int count = splitSpaceQuoted(cleaned, tokens, 16);

        // Require exactly 14 fields
        if (count >= 14) {
            Bill b;
            b.billId = tokens[0];
            b.customerName = tokens[1];
            b.orderId = tokens[2];

            try {
                b.totalWithoutTax = std::stod(tokens[3]);
                b.totalTax = std::stod(tokens[4]);
                b.totalApplicableDiscount = std::stod(tokens[5]);
                b.totalPayable = std::stod(tokens[6]);
                b.amountReceived = std::stod(tokens[7]);
                b.balanceReturned = std::stod(tokens[8]);
                b.paymentStatus = tokens[9];
                b.seasonalDiscount = std::stod(tokens[10]);
                b.specialDiscount = std::stod(tokens[11]);
                b.billDate = tokens[12];
                b.billTime = tokens[13];
            } catch (...) {
                continue; // Skip corrupted record safely
            }

            outBills.push_back(b);
        }
    }

    file.close();
    return true;
}

// Loads itemized records for a specific Bill ID from BillDetail.txt
bool BillGenerator::loadBillDetails(const std::string& billId, std::vector<BillDetail>& outDetails) const {
    outDetails.clear();

    std::ifstream file(detailFilePath);
    if (!file.is_open()) {
        return true;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string cleaned = cleanLine(line);
        if (cleaned.empty()) continue;

        std::string tokens[6];
        int count = splitSpaceQuoted(cleaned, tokens, 6);

        // Exactly 5 fields: BillID ProductAcronym UnitPriceWithoutTax Quantity ProductDiscount#
        if (count >= 5 && tokens[0] == billId) {
            BillDetail d;
            d.billId = tokens[0];
            d.productAcronym = tokens[1];

            try {
                d.unitPriceWithoutTax = std::stod(tokens[2]);
                d.quantity = std::stoi(tokens[3]);
                d.productDiscount = std::stod(tokens[4]);
                d.lineTotal = (d.unitPriceWithoutTax * d.quantity) - d.productDiscount;
                if (d.lineTotal < 0.0) d.lineTotal = 0.0;
            } catch (...) {
                continue;
            }

            outDetails.push_back(d);
        }
    }

    file.close();
    return true;
}

// Generates PDF bill (Requirement 7)
bool BillGenerator::generateBillPDF(const Bill& bill, std::string& outPdfPath) {
    outPdfPath = billsDirectory + "Bill_" + bill.billId + ".pdf";
    std::string errorMsg = "";
    return PDFGenerator::generatePDF(bill, outPdfPath, errorMsg);
}

// Complete billing workflow execution
bool BillGenerator::processOrderBilling(const Order& order,
                                      const ProductManager& productManager,
                                      double specialDiscountPercent,
                                      Bill& outBill,
                                      std::string& outPdfPath,
                                      std::string& statusMsg,
                                      double customTaxRate) {
    // 1. Calculate bill totals
    std::string calcError = "";
    if (!calculateBill(order, productManager, specialDiscountPercent, outBill, calcError, customTaxRate)) {
        statusMsg = "Calculation error: " + calcError;
        return false;
    }

    // 2. Generate unique 9-digit Bill ID
    outBill.billId = generateUniqueBillId();
    for (size_t i = 0; i < outBill.items.size(); ++i) {
        outBill.items[i].billId = outBill.billId;
    }

    // 3. Save to Bill.txt (14 fields)
    if (!saveBill(outBill)) {
        statusMsg = "Error: Failed to write to Bill.txt.";
        return false;
    }

    // 4. Save to BillDetail.txt (5 fields per item)
    if (!saveBillDetails(outBill)) {
        statusMsg = "Error: Failed to write to BillDetail.txt.";
        return false;
    }

    // 5. Generate PDF
    if (!generateBillPDF(outBill, outPdfPath)) {
        statusMsg = "Bill saved to files, but PDF generation failed.";
        return false;
    }

    statusMsg = "Bill " + outBill.billId + " generated and PDF saved successfully!";
    return true;
}

// Displays bill summary to console
void BillGenerator::displayBill(const Bill& bill) {
    std::cout << "\n=======================================================\n";
    std::cout << "                  ISE POS SUPERMARKET                  \n";
    std::cout << "=======================================================\n";
    std::cout << "Bill ID:     " << bill.billId << "\n";
    std::cout << "Order ID:    " << bill.orderId << "\n";
    std::cout << "Customer:    " << bill.customerName << "\n";
    std::cout << "Date & Time: " << bill.billDate << " " << bill.billTime << "\n";
    std::cout << "Status:      " << bill.paymentStatus << "\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << std::left << std::setw(10) << "Acronym"
              << std::setw(20) << "Product Name"
              << std::setw(10) << "Price"
              << std::setw(6)  << "Qty"
              << std::setw(10) << "Discount"
              << "Line Total\n";
    std::cout << "-------------------------------------------------------\n";

    for (size_t i = 0; i < bill.items.size(); ++i) {
        const BillDetail& d = bill.items[i];
        std::cout << std::left << std::setw(10) << d.productAcronym
                  << std::setw(20) << (d.productName.length() > 18 ? d.productName.substr(0, 18) : d.productName)
                  << std::fixed << std::setprecision(2)
                  << std::setw(10) << d.unitPriceWithoutTax
                  << std::setw(6)  << d.quantity
                  << std::setw(10) << d.productDiscount
                  << d.lineTotal << "\n";
    }

    std::cout << "-------------------------------------------------------\n";
    std::cout << std::left << std::setw(35) << "Total Without Tax:"
              << "Rs. " << std::fixed << std::setprecision(2) << bill.totalWithoutTax << "\n";
    std::cout << std::left << std::setw(35) << "Total Tax:"
              << "Rs. " << bill.totalTax << "\n";
    std::cout << std::left << std::setw(35) << "Total Applicable Discount:"
              << "-Rs. " << bill.totalApplicableDiscount << "\n";
    std::cout << std::left << std::setw(35) << ("Seasonal Discount (" + std::to_string(static_cast<int>(bill.seasonalDiscountPercentage)) + "%):")
              << "-Rs. " << bill.seasonalDiscount << "\n";
    std::cout << std::left << std::setw(35) << ("Special Discount (" + std::to_string(static_cast<int>(bill.specialDiscount)) + "%):")
              << "-Rs. " << bill.specialDiscountAmount << "\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << std::left << std::setw(35) << "TOTAL PAYABLE:"
              << "Rs. " << bill.totalPayable << "\n";
    std::cout << std::left << std::setw(35) << "Amount Received:"
              << "Rs. " << bill.amountReceived << "\n";
    std::cout << std::left << std::setw(35) << "Balance Returned:"
              << "Rs. " << bill.balanceReturned << "\n";
    std::cout << "=======================================================\n\n";
}

bool BillGenerator::openPDF(const std::string& pdfPath) {
    return PDFGenerator::openPDF(pdfPath);
}

bool BillGenerator::printPDF(const std::string& pdfPath) {
    return PDFGenerator::printPDF(pdfPath);
}
