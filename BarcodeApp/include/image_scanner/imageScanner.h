#include <string>

// Reads a Code 128 barcode from an image file (PNG / JPG / BMP).
// Returns true and the 6-digit code (as a string, leading zeros kept),
// or false and an error message that can be shown to the user.
bool readBarcodeFromImage(std::string path, std::string &code, std::string &error);
