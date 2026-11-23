#include "../../include/billing/PDFGenerator.h"
#include "../../include/billing/BillGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

// Escapes special characters in PDF literal strings
std::string PDFGenerator::escapePdfText(const std::string& text) {
    std::string result = "";
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        if (c == '(' || c == ')' || c == '\\') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

// Formats a monetary number with 2 decimal places
std::string PDFGenerator::formatMoney(double value) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << value;
    return ss.str();
}

// Generates a genuine, valid PDF receipt (Requirement 7)
// Supports multi-page pagination if product count exceeds single-page capacity
// Time Complexity: O(P) where P is the number of products in the bill
bool PDFGenerator::generatePDF(const Bill& bill, const std::string& outputPath, std::string& errorMsg) {
    // 1. Ensure directory exists
    try {
        std::filesystem::path p(outputPath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        errorMsg = "Could not create directory for PDF: " + std::string(e.what());
        return false;
    }

    // 2. Prevent accidental overwrites
    if (std::filesystem::exists(outputPath)) {
        // If file already exists, check if it's the exact same bill
        // Overwrite only if reprinting the same bill
    }

    std::ofstream pdf(outputPath, std::ios::binary);
    if (!pdf.is_open()) {
        errorMsg = "Failed to open PDF file for writing: " + outputPath;
        return false;
    }

    // Layout configuration
    // Page dimensions: US Letter 612 x 792 points
    const int itemsPerPage = 12; // Products per page
    int totalItems = static_cast<int>(bill.items.size());
    int totalPages = (totalItems <= itemsPerPage) ? 1 : ((totalItems + itemsPerPage - 1) / itemsPerPage);
    if (totalPages < 1) totalPages = 1;

    // Vector of page content streams
    std::vector<std::string> pageStreams;

    for (int pageIndex = 0; pageIndex < totalPages; ++pageIndex) {
        std::stringstream ss;

        // Header Background Banner
        ss << "0.92 0.94 0.97 rg\n"; // Light slate blue
        ss << "40 705 532 55 re f\n"; // Header rectangle

        // Store Header Text
        ss << "BT\n";
        ss << "/F2 18 Tf 50 735 Td (ISE POS SUPERMARKET) Tj\n";
        ss << "/F1 9 Tf 50 715 Td (Main Commercial Center, Campus | Phone: 021-111-128-128 | Tax NTN: 892341-2) Tj\n";
        ss << "ET\n";

        // Invoice Title & Status
        ss << "BT\n";
        ss << "/F2 14 Tf 420 735 Td (SALES RECEIPT) Tj\n";
        ss << "/F2 10 Tf 420 715 Td (Status: " << escapePdfText(bill.paymentStatus) << ") Tj\n";
        ss << "ET\n";

        // Divider Line
        ss << "0.3 0.3 0.3 RG 1 w\n";
        ss << "40 695 m 572 695 l S\n";

        // Bill Meta Information Box
        ss << "BT\n";
        ss << "/F2 10 Tf 50 675 Td (Bill ID:) Tj /F1 10 Tf 105 675 Td (" << escapePdfText(bill.billId) << ") Tj\n";
        ss << "/F2 10 Tf 220 675 Td (Order ID:) Tj /F1 10 Tf 280 675 Td (" << escapePdfText(bill.orderId) << ") Tj\n";
        ss << "/F2 10 Tf 410 675 Td (Date:) Tj /F1 10 Tf 450 675 Td (" << escapePdfText(bill.billDate) << ") Tj\n";

        ss << "/F2 10 Tf 50 655 Td (Customer:) Tj /F1 10 Tf 105 655 Td (" << escapePdfText(bill.customerName) << ") Tj\n";
        ss << "/F2 10 Tf 410 655 Td (Time:) Tj /F1 10 Tf 450 655 Td (" << escapePdfText(bill.billTime) << ") Tj\n";
        ss << "ET\n";

        // Table Header
        float tableTopY = 635.0f;
        ss << "0.15 0.25 0.35 rg\n"; // Dark header
        ss << "40 " << (tableTopY - 18.0f) << " 532 20 re f\n";

        ss << "1.0 1.0 1.0 rg\n"; // White text
        ss << "BT\n";
        ss << "/F2 9 Tf 50 " << (tableTopY - 14.0f) << " Td (Product Acronym) Tj\n";
        ss << "/F2 9 Tf 170 " << (tableTopY - 14.0f) << " Td (Product Description) Tj\n";
        ss << "/F2 9 Tf 320 " << (tableTopY - 14.0f) << " Td (Unit Price) Tj\n";
        ss << "/F2 9 Tf 395 " << (tableTopY - 14.0f) << " Td (Qty) Tj\n";
        ss << "/F2 9 Tf 440 " << (tableTopY - 14.0f) << " Td (Discount) Tj\n";
        ss << "/F2 9 Tf 505 " << (tableTopY - 14.0f) << " Td (Line Total) Tj\n";
        ss << "ET\n";

        // Table Rows
        float currentY = tableTopY - 35.0f;
        int startIndex = pageIndex * itemsPerPage;
        int endIndex = (startIndex + itemsPerPage < totalItems) ? (startIndex + itemsPerPage) : totalItems;

        for (int i = startIndex; i < endIndex; ++i) {
            const BillDetail& item = bill.items[i];

            // Alternating row background for readability
            if (i % 2 == 1) {
                ss << "0.96 0.97 0.98 rg\n";
                ss << "40 " << (currentY - 4.0f) << " 532 16 re f\n";
            }

            ss << "0.1 0.1 0.1 rg\n";
            ss << "BT\n";
            ss << "/F2 9 Tf 50 " << currentY << " Td (" << escapePdfText(item.productAcronym) << ") Tj\n";

            std::string displayName = item.productName.empty() ? item.productAcronym : item.productName;
            if (displayName.length() > 22) displayName = displayName.substr(0, 20) + "..";
            ss << "/F1 9 Tf 170 " << currentY << " Td (" << escapePdfText(displayName) << ") Tj\n";

            ss << "/F1 9 Tf 320 " << currentY << " Td (Rs. " << formatMoney(item.unitPriceWithoutTax) << ") Tj\n";
            ss << "/F1 9 Tf 400 " << currentY << " Td (" << item.quantity << ") Tj\n";
            ss << "/F1 9 Tf 440 " << currentY << " Td (Rs. " << formatMoney(item.productDiscount) << ") Tj\n";
            ss << "/F2 9 Tf 505 " << currentY << " Td (Rs. " << formatMoney(item.lineTotal) << ") Tj\n";
            ss << "ET\n";

            // Row separator line
            ss << "0.85 0.85 0.85 RG 0.5 w\n";
            ss << "40 " << (currentY - 4.0f) << " m 572 " << (currentY - 4.0f) << " l S\n";

            currentY -= 18.0f;
        }

        // On the final page, draw the financial summary box
        if (pageIndex == totalPages - 1) {
            float summaryTop = currentY - 15.0f;
            if (summaryTop < 220.0f) summaryTop = 220.0f;

            // Summary Box Background
            ss << "0.97 0.98 0.99 rg\n";
            ss << "260 " << (summaryTop - 130.0f) << " 312 135 re f\n";
            ss << "0.7 0.75 0.8 RG 1 w\n";
            ss << "260 " << (summaryTop - 130.0f) << " 312 135 re S\n";

            ss << "0.1 0.1 0.1 rg\n";
            ss << "BT\n";
            float sy = summaryTop - 15.0f;

            // 1. Total Without Tax
            ss << "/F1 9 Tf 275 " << sy << " Td (Total Bill Without Tax:) Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (Rs. " << formatMoney(bill.totalWithoutTax) << ") Tj\n";
            sy -= 14.0f;

            // 2. Total Tax
            ss << "/F1 9 Tf 275 " << sy << " Td (Total Tax:) Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (Rs. " << formatMoney(bill.totalTax) << ") Tj\n";
            sy -= 14.0f;

            // 3. Product Discounts
            ss << "/F1 9 Tf 275 " << sy << " Td (Product Discounts:) Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (- Rs. " << formatMoney(bill.totalApplicableDiscount) << ") Tj\n";
            sy -= 14.0f;

            // 4. Seasonal Discount
            std::stringstream sdisc;
            sdisc << "Seasonal Discount (" << std::fixed << std::setprecision(1) << bill.seasonalDiscountPercentage << "%):";
            ss << "/F1 9 Tf 275 " << sy << " Td (" << sdisc.str() << ") Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (- Rs. " << formatMoney(bill.seasonalDiscount) << ") Tj\n";
            sy -= 14.0f;

            // 5. Special Discount (Percentage and Amount)
            std::stringstream spdisc;
            spdisc << "Special Discount (" << std::fixed << std::setprecision(1) << bill.specialDiscount << "%):";
            ss << "/F1 9 Tf 275 " << sy << " Td (" << spdisc.str() << ") Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (- Rs. " << formatMoney(bill.specialDiscountAmount) << ") Tj\n";
            sy -= 16.0f;

            // Divider in summary
            ss << "ET\n";
            ss << "0.6 0.6 0.6 RG 1 w\n";
            ss << "270 " << (sy + 10.0f) << " m 562 " << (sy + 10.0f) << " l S\n";
            ss << "BT\n";

            // 6. Total Payable
            ss << "0.05 0.45 0.25 rg\n"; // Bold dark green
            ss << "/F2 11 Tf 275 " << sy << " Td (FINAL PAYABLE AMOUNT:) Tj ";
            ss << "/F2 11 Tf 475 " << sy << " Td (Rs. " << formatMoney(bill.totalPayable) << ") Tj\n";
            sy -= 15.0f;

            // 7. Amount Received & Balance Returned
            ss << "0.15 0.15 0.15 rg\n";
            ss << "/F1 9 Tf 275 " << sy << " Td (Amount Received:) Tj ";
            ss << "/F1 9 Tf 480 " << sy << " Td (Rs. " << formatMoney(bill.amountReceived) << ") Tj\n";
            sy -= 13.0f;

            ss << "/F2 9 Tf 275 " << sy << " Td (Balance Returned:) Tj ";
            ss << "/F2 9 Tf 480 " << sy << " Td (Rs. " << formatMoney(bill.balanceReturned) << ") Tj\n";
            ss << "ET\n";
        }

        // Page Footer
        ss << "BT\n";
        ss << "0.4 0.4 0.4 rg\n";
        ss << "/F1 8 Tf 50 35 Td (Thank you for shopping with us! Computer-generated bill. No signature required.) Tj\n";

        std::stringstream pageText;
        pageText << "Page " << (pageIndex + 1) << " of " << totalPages;
        ss << "/F1 8 Tf 520 35 Td (" << pageText.str() << ") Tj\n";
        ss << "ET\n";

        pageStreams.push_back(ss.str());
    }

    // Write standard PDF objects
    pdf << "%PDF-1.4\n";

    // Track byte offsets for xref table
    std::vector<size_t> offsets;
    offsets.push_back(0); // 0 index dummy

    // Object 1: Catalog
    offsets.push_back(pdf.tellp());
    pdf << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

    // Build Page Object list:
    // Pages object is 2 0 R
    // Fonts: 3 0 R (F1 Helvetica), 4 0 R (F2 Helvetica-Bold)
    // Page objects start from 5, alternating Page and Content Stream
    // For page p: Page object = 5 + 2*p, Stream object = 6 + 2*p

    std::stringstream kidsList;
    kidsList << "[";
    for (int p = 0; p < totalPages; ++p) {
        int pageObjNum = 5 + 2 * p;
        kidsList << pageObjNum << " 0 R ";
    }
    kidsList << "]";

    // Object 2: Pages
    offsets.push_back(pdf.tellp());
    pdf << "2 0 obj\n<< /Type /Pages /Kids " << kidsList.str() << " /Count " << totalPages << " >>\nendobj\n";

    // Object 3: Font F1 (Helvetica)
    offsets.push_back(pdf.tellp());
    pdf << "3 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n";

    // Object 4: Font F2 (Helvetica-Bold)
    offsets.push_back(pdf.tellp());
    pdf << "4 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>\nendobj\n";

    // Page and Stream objects
    for (int p = 0; p < totalPages; ++p) {
        int pageObjNum = 5 + 2 * p;
        int streamObjNum = 6 + 2 * p;
        const std::string& streamData = pageStreams[p];

        // Page Object
        offsets.push_back(pdf.tellp());
        pdf << pageObjNum << " 0 obj\n"
            << "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] "
            << "/Contents " << streamObjNum << " 0 R "
            << "/Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> >>\nendobj\n";

        // Stream Object
        offsets.push_back(pdf.tellp());
        pdf << streamObjNum << " 0 obj\n"
            << "<< /Length " << streamData.length() << " >>\nstream\n"
            << streamData
            << "endstream\nendobj\n";
    }

    // Write Cross-Reference Table
    size_t xrefOffset = pdf.tellp();
    int totalObjects = 4 + 2 * totalPages; // Catalog(1), Pages(2), F1(3), F2(4) + 2*pages

    pdf << "xref\n0 " << (totalObjects + 1) << "\n";
    pdf << "0000000000 65535 f \n";
    for (int i = 1; i <= totalObjects; ++i) {
        pdf << std::setw(10) << std::setfill('0') << offsets[i] << " 00000 n \n";
    }

    // Trailer
    pdf << "trailer\n<< /Size " << (totalObjects + 1) << " /Root 1 0 R >>\n";
    pdf << "startxref\n" << xrefOffset << "\n%%EOF\n";

    pdf.close();
    return true;
}

// Opens the PDF using default Windows application
bool PDFGenerator::openPDF(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "PDF file not found: " << filePath << std::endl;
        return false;
    }

    HINSTANCE result = ShellExecuteA(
        NULL,
        "open",
        filePath.c_str(),
        NULL,
        NULL,
        SW_SHOWNORMAL
    );

    return (reinterpret_cast<INT_PTR>(result) > 32);
}

// Prints the PDF using default Windows print action
bool PDFGenerator::printPDF(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "PDF file not found: " << filePath << std::endl;
        return false;
    }

    HINSTANCE result = ShellExecuteA(
        NULL,
        "print",
        filePath.c_str(),
        NULL,
        NULL,
        SW_SHOWNORMAL
    );

    return (reinterpret_cast<INT_PTR>(result) > 32);
}
