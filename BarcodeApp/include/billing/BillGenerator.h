#pragma once

#include <string>
#include <vector>
#include "../order/Order.h"
#include "../product/ProductManager.h"
#include "SeasonManager.h"

// Structure representing an individual itemized line in a bill (Requirement 9)
// Exactly 5 fields saved to BillDetail.txt:
// BillID ProductAcronym UnitPriceWithoutTax Quantity ProductDiscount#
struct BillDetail {
    std::string billId;              // 9-digit unique Bill ID
    std::string productAcronym;      // Product acronym / code (e.g. "P001" or "COLA")
    double unitPriceWithoutTax;      // Price per unit before tax
    int quantity;                    // Quantity purchased
    double productDiscount;          // Total discount for this line (unit discount * quantity)

    // Additional fields for display and PDF presentation
    std::string productName;         // Descriptive name (e.g. "Milk 1L")
    double lineTotal;                // (unitPriceWithoutTax * quantity) - productDiscount

    BillDetail();
    BillDetail(const std::string& bId, const std::string& acronym, double unitPrice,
               int qty, double discount, const std::string& name = "");
};

// Structure representing a complete bill record (Requirement 8)
// Exactly 14 fields saved to Bill.txt:
// BillID CustomerName OrderID TotalWithoutTax TotalTax TotalApplicableDiscount
// TotalPayable AmountReceived BalanceReturned PaymentStatus SeasonalDiscount
// SpecialDiscount BillDate BillTime#
struct Bill {
    std::string billId;                 // 1. Unique 9-digit Bill ID
    std::string customerName;           // 2. Customer name (quoted if contains spaces)
    std::string orderId;                // 3. 8-digit Order ID from Order.txt
    double totalWithoutTax;             // 4. Sum of unit price * qty
    double totalTax;                    // 5. Total tax amount
    double totalApplicableDiscount;     // 6. Sum of all product-level line discounts
    double totalPayable;                // 7. Final amount payable after all discounts and tax
    double amountReceived;              // 8. Amount received from customer
    double balanceReturned;             // 9. Change returned to customer
    std::string paymentStatus;          // 10. Status from Order.txt (e.g. "paid")
    double seasonalDiscount;            // 11. Actual monetary amount of seasonal discount
    double specialDiscount;             // 12. Special discount percentage (<= 8.00)
    std::string billDate;               // 13. Generation date (YYYY-MM-DD)
    std::string billTime;               // 14. Generation time (HH:MM:SS)

    // Helper fields used for display and PDF generation
    double seasonalDiscountPercentage;  // Configured seasonal percentage
    double specialDiscountAmount;       // Calculated monetary amount of special discount
    std::vector<BillDetail> items;      // List of itemized product lines

    Bill();
};

// Billing module coordinator: handles calculation, persistence, and PDF creation
class BillGenerator {
private:
    std::string billFilePath;         // Path to Bill.txt
    std::string detailFilePath;       // Path to BillDetail.txt
    std::string billsDirectory;       // Directory to save PDFs ("Bills/")
    SeasonManager seasonManager;      // Season manager for automatic seasonal discounts

    // Helper string functions
    static std::string trim(const std::string& str);
    static std::string cleanLine(const std::string& line);
    static std::string formatField(const std::string& str);
    static int splitSpaceQuoted(const std::string& line, std::string tokens[], int maxTokens);

public:
    // Constructors
    BillGenerator();
    BillGenerator(const std::string& billFile, const std::string& detailFile, const std::string& seasonFile);

    // ==========================================
    // 1. Bill ID Generation & Lookup (Req 8)
    // ==========================================
    // Generates a unique 9-digit Bill ID that does not exist in Bill.txt
    std::string generateUniqueBillId();

    // Checks whether a given Bill ID already exists in Bill.txt
    bool billIdExists(const std::string& billId) const;

    // ==========================================
    // 2. Bill Calculation (Req 10, 15, Special <= 8%)
    // ==========================================
    // Calculates all financial totals with intermediate variables:
    // A. Total bill without tax = sum(unit price * quantity)
    // B. Total tax = sum(tax)
    // C. Total applicable discount = sum(product unit discount * quantity)
    // D. Seasonal discount = (Total bill without tax - Total applicable discount) * Seasonal % / 100
    // E. Special discount = (Total bill without tax - Total applicable discount - Seasonal discount) * Special % / 100
    // F. Total payable = Total without tax + Total tax - Total applicable discount - Seasonal discount - Special discount
    bool calculateBill(const Order& order,
                       const ProductManager& productManager,
                       double specialDiscountPercent,
                       Bill& outBill,
                       std::string& errorMsg,
                       double customTaxRate = 0.0);

    // ==========================================
    // 3. File Persistence (Req 8 & Req 9)
    // ==========================================
    // Saves exactly 14 fields to Bill.txt ending with '#'
    bool saveBill(const Bill& bill);

    // Saves exactly 5 fields per item to BillDetail.txt ending with '#'
    bool saveBillDetails(const Bill& bill);

    // Loads all bills from Bill.txt
    bool loadBills(std::vector<Bill>& outBills) const;

    // Loads items for a specific Bill ID from BillDetail.txt
    bool loadBillDetails(const std::string& billId, std::vector<BillDetail>& outDetails) const;

    // Checks if an order already has a bill generated
    bool hasBillForOrder(const std::string& orderId, std::string* outBillId = nullptr) const;

    // ==========================================
    // 4. PDF Generation (Req 7)
    // ==========================================
    // Generates a genuine, valid PDF receipt in Bills/Bill_<BillID>.pdf
    bool generateBillPDF(const Bill& bill, std::string& outPdfPath);

    // Opens the generated PDF in the default Windows application
    static bool openPDF(const std::string& pdfPath);

    // Prints the generated PDF using the default Windows print action
    static bool printPDF(const std::string& pdfPath);

    // ==========================================
    // 5. Complete Billing Pipeline
    // ==========================================
    // Calculates bill, saves Bill.txt and BillDetail.txt, and generates PDF
    bool processOrderBilling(const Order& order,
                             const ProductManager& productManager,
                             double specialDiscountPercent,
                             Bill& outBill,
                             std::string& outPdfPath,
                             std::string& statusMsg,
                             double customTaxRate = 0.0);

    // Displays bill summary to console (useful for debugging and evaluation)
    static void displayBill(const Bill& bill);

    // Season manager accessor
    SeasonManager& getSeasonManager();
    const SeasonManager& getSeasonManager() const;
};
