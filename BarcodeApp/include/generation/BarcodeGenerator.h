#pragma once
#include <string>
#include <ZXing/ImageView.h>

class BarcodeGenerator
{
public:
    ZXing::Image generateBarcode(const std::string& code);
};
