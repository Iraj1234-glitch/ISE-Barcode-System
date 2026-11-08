
#include "../../include/generation/BarcodeGenerator.h"
#include <fstream>

using namespace std;

// Code 128 standard patterns
const char* patterns[107] =
{
    "212222", "222122", "222221", "121223", "121322",
    "131222", "122213", "122312", "132212", "221213",
    "221312", "231212", "112232", "122132", "122231",
    "113222", "123122", "123221", "223211", "221132",
    "221231", "213212", "223112", "312131", "311222",
    "321122", "321221", "312212", "322112", "322211",
    "212123", "212321", "232121", "111323", "131123",
    "131321", "112313", "132113", "132311", "211313",
    "231113", "231311", "112133", "112331", "132131",
    "113123", "113321", "133121", "313121", "211331",
    "231131", "213113", "213311", "213131", "311123",
    "311321", "331121", "312113", "312311", "332111",
    "314111", "221411", "431111", "111224", "111422",
    "121124", "121421", "141122", "141221", "112214",
    "112412", "122114", "122411", "142112", "142211",
    "241211", "221114", "413111", "241112", "134111",
    "111242", "121142", "121241", "114212", "124112",
    "124211", "411212", "421112", "421211", "212141",
    "214121", "412121", "111143", "111341", "131141",
    "114113", "114311", "411113", "411311", "113141",
    "114131", "311141", "411131", "211412", "211214",
    "211232", "2331112"
};

// Write a 2-byte number
void writeU16(ofstream& file, unsigned short value)
{
    file.put(value & 255);
    file.put((value >> 8) & 255);
}

// Write a 4-byte number
void writeU32(ofstream& file, unsigned int value)
{
    file.put(value & 255);
    file.put((value >> 8) & 255);
    file.put((value >> 16) & 255);
    file.put((value >> 24) & 255);
}

bool BarcodeGenerator::generateBarcode(const string& code, const string& outputPath)
{
    // Input must contain exactly six digits
    if (code.length() != 6)
        return false;

    // First digit cannot be zero
    if (code[0] == '0')
        return false;

    // Check that every character is a digit
    for (int i = 0; i < 6; i++)
    {
        if (code[i] < '0' || code[i] > '9')
            return false;
    }

    // Code 128 Code B starts with symbol 104
    int symbols[10];

    symbols[0] = 104;

    // Convert digits into Code B symbols
    int checksum = 104;

    for (int i = 0; i < 6; i++)
    {
        symbols[i + 1] = code[i] - 32;
        checksum += symbols[i + 1] * (i + 1);
    }

    // Calculate checksum using modulo 103
    symbols[7] = checksum % 103;

    // Stop symbol
    symbols[8] = 106;

    // Calculate total barcode width
    int totalModules = 11 * 8 + 13;

    int height = 120;
    int scale = 3;
    int quietZone = 30;

    int barcodeWidth = totalModules * scale + quietZone * 2;
    int rowSize = (barcodeWidth * 3 + 3) & ~3;

    // Fixed-size image array
    static unsigned char pixels[10000 * 120];

    // Fill image with white pixels
    for (int i = 0; i < rowSize * height; i++)
        pixels[i] = 255;

    int x = quietZone;

    // Draw all barcode symbols
    for (int s = 0; s < 9; s++)
    {
        const char* pattern = patterns[symbols[s]];

        bool black = true;

        for (int j = 0; pattern[j] != '\0'; j++)
        {
            int width = (pattern[j] - '0') * scale;

            if (black)
            {
                for (int px = x; px < x + width; px++)
                {
                    for (int y = 0; y < height; y++)
                    {
                        int index = (height - 1 - y) * rowSize + px * 3;

                        pixels[index] = 0;
                        pixels[index + 1] = 0;
                        pixels[index + 2] = 0;
                    }
                }
            }

            x += width;
            black = !black;
        }
    }

    // Write BMP image
    ofstream file(outputPath, ios::binary);

    if (!file)
        return false;

    file.put('B');
    file.put('M');

    writeU32(file, 54 + rowSize * height);
    writeU16(file, 0);
    writeU16(file, 0);
    writeU32(file, 54);

    writeU32(file, 40);
    writeU32(file, barcodeWidth);
    writeU32(file, height);
    writeU16(file, 1);
    writeU16(file, 24);
    writeU32(file, 0);
    writeU32(file, rowSize * height);
    writeU32(file, 2835);
    writeU32(file, 2835);
    writeU32(file, 0);
    writeU32(file, 0);

    file.write(reinterpret_cast<const char*>(pixels), rowSize * height);

    file.close();

    return file.good();
}
