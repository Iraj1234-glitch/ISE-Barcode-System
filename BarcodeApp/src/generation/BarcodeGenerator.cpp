#include "../../include/generation/BarcodeGenerator.h"
#include <ZXing/CreateBarcode.h>
#include <ZXing/WriteBarcode.h>
#include <ZXing/BarcodeFormat.h>
ZXing::Image BarcodeGenerator::generateBarcode(const std::string& code)
{
    // Check that code contains exactly 6 characters
    if (code.length() != 6)
        return {};

    // Check that all 6 characters are digits
    for (int i = 0; i < 6; i++)
    {
        if (code[i] < '0' || code[i] > '9')
            return {};
    }

    // Select Code 128 barcode format
    ZXing::CreatorOptions options(ZXing::BarcodeFormat::Code128);

    // Create barcode from the input code
    auto barcode = ZXing::CreateBarcodeFromText(code, options);

    // Set barcode image options
    ZXing::WriterOptions writerOptions;
    writerOptions.scale(3);
    writerOptions.addHRT(false);

    // Convert barcode into an image and return it
    return ZXing::WriteBarcodeToImage(barcode, writerOptions);
}