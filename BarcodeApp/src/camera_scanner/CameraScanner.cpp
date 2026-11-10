#include "../../include/camera_scanner/CameraScanner.h"
#include<exception>
#include <iostream>
#include <string>

#include <ZXing/ZXingCpp.h>

using namespace std;

bool CameraScanner::isValidCode(const string& code)
{
    // Code must contain exactly 6 characters
    if (code.length() != 6)
    {
        return false;
    }

    // First digit must not be zero
    if (code[0] == '0')
    {
        return false;
    }

    // Every character must be a digit
    for (int i = 0; i < 6; i++)
    {
        if (code[i] < '0' || code[i] > '9')
        {
            return false;
        }
    }

    return true;
}


string CameraScanner::scanUsingCamera()
{
    // Open the default webcam
    cv::VideoCapture camera(0);

    // Check whether the camera opened successfully
    if (!camera.isOpened())
    {
        cout << "Error: Camera could not be opened!" << endl;
        return "";
    }

    cout << "Camera started successfully." << endl;
    cout << "Show a Code 128 barcode containing 6 digits." << endl;
    cout << "Press ESC to stop scanning." << endl;

    string scannedCode = "";

    try
    {
        while (true)
        {
            cv::Mat frame;

            // Capture one frame from the camera
            camera >> frame;

            // Check whether frame was captured
            if (frame.empty())
            {
                cout << "Error: Could not capture camera frame." << endl;
                break;
            }

            // Convert image to grayscale for barcode reading
            cv::Mat grayImage;

            cv::cvtColor(
                frame,
                grayImage,
                cv::COLOR_BGR2GRAY
            );

            // Convert OpenCV image into ZXing image format
            ZXing::ImageView image(
                grayImage.data,
                grayImage.cols,
                grayImage.rows,
                ZXing::ImageFormat::Lum
            );

            // Read barcodes from the current frame
            auto barcodes = ZXing::ReadBarcodes(image);

            // Check every detected barcode
            for (const auto& barcode : barcodes)
            {
                // Accept only Code 128
                if (barcode.format() == ZXing::BarcodeFormat::Code128)
                {
                    string code = barcode.text();

                    // Accept only exactly 6 numeric digits
                    if (isValidCode(code))
                    {
                        scannedCode = code;

                        cout << "Barcode scanned successfully!" << endl;
                        cout << "Code: " << scannedCode << endl;

                        cv::putText(
                            frame,
                            "Code: " + scannedCode,
                            cv::Point(30, 50),
                            cv::FONT_HERSHEY_SIMPLEX,
                            1,
                            cv::Scalar(0, 255, 0),
                            2
                        );

                        break;
                    }
                }
            }

            // show the live camera feed
            cv::imshow("Camera Scanner", frame);

            // Stop when a valid barcode is found
            if (scannedCode != "")
            {
                cv::waitKey(1000);
                break;
            }

            // ESC key stops scanning
            if (cv::waitKey(1) == 27)
            {
                cout << "Scanning stopped by user." << endl;
                break;
            }
        }
    }
    catch (const std::exception& error)
    {
        cout << "Scanning error: " << error.what() << endl;
    }

    // Release camera resources
    camera.release();

    // Close camera window
    cv::destroyAllWindows();

    return scannedCode;
}
