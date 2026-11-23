#pragma once

#include <string>
#include <vector>

// Forward declarations
struct Bill;
struct BillDetail;

// Generates a genuine, standard PDF (PDF-1.4) document for bills (Requirement 7)
class PDFGenerator {
private:
    // Helper to escape PDF text special characters: parentheses and backslash
    static std::string escapePdfText(const std::string& text);

    // Formats monetary values with 2 decimal places: e.g. "1250.00"
    static std::string formatMoney(double value);

public:
    // Generates a complete, multi-page receipt PDF for a given Bill and saves it to outputPath.
    // Creates directory if missing and prevents accidental overwrites.
    static bool generatePDF(const Bill& bill, const std::string& outputPath, std::string& errorMsg);

    // Opens the PDF using default Windows application
    static bool openPDF(const std::string& filePath);

    // Prints the PDF using default Windows print action
    static bool printPDF(const std::string& filePath);
};
