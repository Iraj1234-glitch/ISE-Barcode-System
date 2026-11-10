#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class CameraScanner
{
public:

    // Opens the webcam , scans until a valid code is found
    std::string scanUsingCamera();

private:

    // Checks whether the barcode contains exactly 6 digits
    bool isValidCode(const std::string& code);
};
