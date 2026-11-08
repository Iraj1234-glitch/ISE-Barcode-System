#include <iostream>
#include <string>
#include <cmath>
#include "../../include/image_scanner/imageScanner.h"
// stb_image.h is a single header file that loads PNG / JPG / BMP images.
// It must be in the folder include/image_scanner/
// STB_IMAGE_IMPLEMENTATION must be defined in ONE .cpp file only, which is this one.
#define STB_IMAGE_IMPLEMENTATION
#include "../../include/image_scanner/stb_image.h"
using namespace std;


//// Barcode Image Upload (Izzah)
// Reads a Code 128 barcode from an image file and gives back the 6-digit code.
// The code is always kept as a string so "000123" never becomes 123.


// Bar/space widths of every Code 128 symbol (0 to 105) and the stop pattern (106)
static string patterns[107] = {
    "212222", "222122", "222221", "121223", "121322", "131222",
    "122213", "122312", "132212", "221213", "221312", "231212",
    "112232", "122132", "122231", "113222", "123122", "123221",
    "223211", "221132", "221231", "213212", "223112", "312131",
    "311222", "321122", "321221", "312212", "322112", "322211",
    "212123", "212321", "232121", "111323", "131123", "131321",
    "112313", "132113", "132311", "211313", "231113", "231311",
    "112133", "112331", "132131", "113123", "113321", "133121",
    "313121", "211331", "231131", "213113", "213311", "213131",
    "311123", "311321", "331121", "312113", "312311", "332111",
    "314111", "221411", "431111", "111224", "111422", "121124",
    "121421", "141122", "141221", "112214", "112412", "122114",
    "122411", "142112", "142211", "241211", "221114", "413111",
    "241112", "134111", "111242", "121142", "121241", "114212",
    "124112", "124211", "411212", "421112", "421211", "212141",
    "214121", "412121", "111143", "111341", "131141", "114113",
    "114311", "411113", "411311", "113141", "114131", "311141",
    "411131", "211412", "211214", "211232", "2331112"
};


// Compare 6 run widths with symbols 0 to 105, return the closest symbol
static int matchSymbol(double r[], double &dist) {
    double total = 0;
    for (int i = 0; i < 6; i++)
        total += r[i];
    int best = -1;
    dist = 1000;
    for (int v = 0; v < 106; v++) {
        double d = 0;
        for (int k = 0; k < 6; k++)
            d += fabs(r[k] * 11 / total - (patterns[v][k] - '0'));
        if (d < dist) {
            dist = d;
            best = v;
        }
    }
    return best;
}
// Compare 7 run widths with the stop pattern
static double stopDistance(double r[]) {
    double total = 0;
    for (int i = 0; i < 7; i++)
        total += r[i];
    double d = 0;
    for (int k = 0; k < 7; k++)
        d += fabs(r[k] * 13 / total - (patterns[106][k] - '0'));
    return d;
}
// Turn the symbol values into text (Code set A, B or C)
static string symbolsToText(int vals[], int count) {
    char set = 'A';
    if (vals[0] == 104)
        set = 'B';
    if (vals[0] == 105)
        set = 'C';
    string text = "";
    for (int i = 1; i < count - 1; i++) {
        int v = vals[i];
        if (set == 'C') {
            if (v < 100) {
                text += char('0' + v / 10);
                text += char('0' + v % 10);
            }
            else if (v == 100)
                set = 'B';
            else if (v == 101)
                set = 'A';
        }
        else {
            if (v < 96) {
                if (set == 'A' && v >= 64)
                    text += '?';
                else
                    text += char(32 + v);
            }
            else if (v == 99)
                set = 'C';
            else if (v == 100 && set == 'A')
                set = 'B';
            else if (v == 101 && set == 'B')
                set = 'A';
        }
    }
    return text;
}
// Look for Start - symbols - checksum - Stop inside a list of run widths
static bool decodeRuns(double runs[], int n, string &text) {
    for (int i = 0; i + 19 <= n; i++) {
        double dist;
        int start = matchSymbol(&runs[i], dist);
        // the first 6 runs must look like a start symbol, not like a normal symbol
        if (start < 103 || dist > 2.2)
            continue;
        double total = 0;
        for (int k = 0; k < 6; k++)
            total += runs[i + k];
        double module = total / 11;
        // quiet (white) space must come before the barcode
        if (i > 0 && runs[i - 1] < 3 * module)
            continue;
        int vals[80];
        int count = 0;
        vals[count++] = start;
        int pos = i + 6;
        bool gotStop = false;
        while (count < 80) {
            double symDist = 1000;
            int sym = -1;
            if (pos + 6 <= n)
                sym = matchSymbol(&runs[pos], symDist);
            double stopDist = 1000;
            if (pos + 7 <= n)
                stopDist = stopDistance(&runs[pos]);
            bool stopOk = stopDist <= 2.6;
            bool symOk = sym >= 0 && symDist <= 2.2;
            if (stopOk && (!symOk || stopDist / 7 <= symDist / 6)) {
                gotStop = true;
                break;
            }
            if (!symOk)
                break;
            vals[count++] = sym;
            pos += 6;
        }
        if (!gotStop || count < 3)
            continue;
        // checksum
        long sum = vals[0];
        for (int k = 1; k < count - 1; k++)
            sum += (long)k * vals[k];
        if (sum % 103 != vals[count - 1])
            continue;
        text = symbolsToText(vals, count);
        return true;
    }
    return false;
}
// Read one row of the image and try to decode it
static bool decodeRow(unsigned char img[], int w, int y, string &text) {
    int lo = 255, hi = 0;
    for (int x = 0; x < w; x++) {
        int p = img[y * w + x];
        if (p < lo)
            lo = p;
        if (p > hi)
            hi = p;
    }
    if (hi - lo < 30)
        return false;   // flat row, no barcode here
    double levels[3] = { 0.5, 0.35, 0.65 };
    double* runs = new double[w + 1];
    double* backwards = new double[w + 1];
    bool found = false;
    for (int t = 0; t < 3 && !found; t++) {
        double limit = lo + levels[t] * (hi - lo);
        // measure the width of each black/white run
        int n = 0;
        bool dark = img[y * w] < limit;
        double count = 0;
        for (int x = 0; x < w; x++) {
            bool d = img[y * w + x] < limit;
            if (d == dark)
                count++;
            else {
                runs[n++] = count;
                dark = d;
                count = 1;
            }
        }
        runs[n++] = count;
        if (decodeRuns(runs, n, text))
            found = true;
        else {
            // barcode may be upside down
            for (int i = 0; i < n; i++)
                backwards[i] = runs[n - 1 - i];
            if (decodeRuns(backwards, n, text))
                found = true;
        }
    }
    delete[] runs;
    delete[] backwards;
    return found;
}
// Main function of the module.
// Returns true and the 6-digit code, or false and an error message.
bool readBarcodeFromImage(string path, string &code, string &error) {
    code = "";
    error = "";
    if (path == "") {
        error = "No image selected. Please choose a file.";
        return false;
    }
    int w, h, channels;
    // the last 1 means: convert the image to grayscale
    unsigned char* img = stbi_load(path.c_str(), &w, &h, &channels, 1);
    if (img == nullptr) {
        error = "Could not open the image (file not found, empty or not a valid image).";
        return false;
    }
    // scan up to 40 rows; the same text must be read from at least 2 rows
    int step = h / 40;
    if (step < 1)
        step = 1;
    string found = "";
    int agree = 0;
    int scanned = 0;
    for (int y = 0; y < h; y += step) {
        scanned++;
        string text;
        if (decodeRow(img, w, y, text)) {
            if (agree == 0) {
                found = text;
                agree = 1;
            }
            else if (text == found)
                agree++;
        }
    }
    stbi_image_free(img);
    int needed = 2;
    if (scanned < 2)
        needed = 1;
    if (agree < needed) {
        error = "Could not read a barcode. Choose a sharp image that shows the whole barcode.";
        return false;
    }
    // validate: exactly 6 digits
    bool valid = found.size() == 6;
    for (int i = 0; i < (int)found.size(); i++)
        if (found[i] < '0' || found[i] > '9')
            valid = false;
    if (!valid) {
        error = "A barcode was found but it does not contain exactly 6 digits.";
        return false;
    }
    code = found;
    return true;
}
//int main() {
//    string path;
//    cout << "Enter the path of the barcode image: ";
//    getline(cin, path);
//    // remove quotes if the path was copied with "Copy as path"
//    if (path.size() >= 2 && path[0] == '"' && path[path.size() - 1] == '"')
//        path = path.substr(1, path.size() - 2);
//    string code, error;
//    if (readBarcodeFromImage(path, code, error))
//        cout << "Decoded code: " << code << endl;
//    else
//        cout << "Error: " << error << endl;
//    return 0;
//}
