
#pragma once

#include <string>

class BarcodeGenerator
{
public:
    bool generateBarcode(const std::string& code, const std::string& outputPath);
};
