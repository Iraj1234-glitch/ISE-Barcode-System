#include "../../include/gui/ProductCatalog.h"

#include <fstream>

using namespace std;

static string trimText(const string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Splits a line on spaces; text inside "double quotes" stays together
static int splitQuoted(const string& line, string tokens[], int maxTokens)
{
    int count = 0;
    string current = "";
    bool inQuotes = false;
    bool hadQuotes = false;

    for (size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];

        if (c == '"')
        {
            inQuotes = !inQuotes;
            hadQuotes = true;
        }
        else if ((c == ' ' || c == '\t') && !inQuotes)
        {
            if (!current.empty() || hadQuotes)
            {
                if (count < maxTokens)
                    tokens[count++] = current;

                current = "";
                hadQuotes = false;
            }
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty() || hadQuotes)
    {
        if (count < maxTokens)
            tokens[count++] = current;
    }

    return count;
}

static bool isValidBarcode(const string& code)
{
    if (code.length() != 6 || code[0] == '0')
        return false;

    for (int i = 0; i < 6; i++)
    {
        if (code[i] < '0' || code[i] > '9')
            return false;
    }

    return true;
}

static string getCatalogPreservedDescription(const string& productId, const string& productName)
{
    if (productId == "P001" || productName == "Milk 1L") {
        return "Fresh full-cream milk in a 1 litre pack. Keep refrigerated.";
    }
    if (productId == "P002" || productName == "Bread") {
        return "Sliced white bread, 400 g loaf. Best before 3 days from purchase.";
    }
    if (productId == "P003" || productName == "Eggs (12)") {
        return "Farm eggs, one dozen in a protective tray.";
    }
    if (productId == "P004" || productName == "Rice 5kg") {
        return "Premium long-grain basmati rice in a 5 kg bag.";
    }
    if (productId == "P005" || productName == "Cooking Oil 1L") {
        return "Refined sunflower cooking oil, 1 litre bottle.";
    }
    if (productId == "P006" || productName == "Sugar 1kg") {
        return "Fine white sugar, 1 kg pack.";
    }
    return "";
}

ProductCatalog::ProductCatalog(const string& path)
{
    count = 0;
    filePath = path;
}

bool ProductCatalog::load()
{
    count = 0;

    ifstream file(filePath.c_str());

    if (!file.is_open())
        return false;

    string line;

    while (getline(file, line))
    {
        string cleaned = trimText(line);

        // Remove the trailing '#'
        if (!cleaned.empty() && cleaned[cleaned.size() - 1] == '#')
            cleaned = trimText(cleaned.substr(0, cleaned.size() - 1));

        if (cleaned.empty())
            continue;

        string fields[20];
        int fieldCount = splitQuoted(cleaned, fields, 20);

        if (fieldCount < 5)
            continue;

        if (!isValidBarcode(fields[1]))
            continue;

        if (count >= MAX_CATALOG_PRODUCTS)
            break;

        if (fieldCount >= 10)
        {
            // Increment 2: 10 fields
            // 0: ID, 1: Barcode, 2: Acronym, 3: Name, 4: Unit, 5: Price, 6: Tax, 7: Discount, 8: Stock, 9: BarcodePath
            double price = 0.0;
            try { price = stod(fields[5]); } catch (...) { continue; }
            if (price < 0.0) continue;

            double tax = 0.0;
            try { tax = stod(fields[6]); } catch (...) { tax = 0.0; }

            double discount = 0.0;
            try { discount = stod(fields[7]); } catch (...) { discount = 0.0; }

            int stock = 0;
            try { stock = stoi(fields[8]); if (stock < 0) stock = 0; } catch (...) { stock = 0; }

            items[count].productId = fields[0];
            items[count].barcode = fields[1];
            items[count].acronym = fields[2];
            items[count].name = fields[3];
            items[count].measurementUnit = fields[4];
            items[count].price = price;
            items[count].tax = tax;
            items[count].discount = discount;
            items[count].stock = stock;
            items[count].barcodeImagePath = fields[9];
            items[count].categoryId = "";
            items[count].description = getCatalogPreservedDescription(fields[0], fields[3]);
            if (fieldCount >= 11)
            {
                bool looksLikeCategory =
                    fields[10].length() == 2 &&
                    fields[10][0] >= '0' && fields[10][0] <= '9' &&
                    fields[10][1] >= '0' && fields[10][1] <= '9';
                if (looksLikeCategory)
                {
                    items[count].categoryId = fields[10];
                    if (fieldCount >= 12 && items[count].description.empty())
                    {
                        items[count].description = fields[11];
                    }
                }
                else if (items[count].description.empty())
                {
                    items[count].description = fields[10];
                }
            }
            count++;
        }
        else
        {
            // Legacy 5/6 fields
            // 0: ID, 1: Barcode, 2: Name, 3: Description, 4: Price, 5: [Optional Stock]
            double price = 0.0;
            try { price = stod(fields[4]); } catch (...) { continue; }
            if (price < 0.0) continue;

            int stock = 0;
            if (fieldCount >= 6) {
                try { stock = stoi(fields[5]); if (stock < 0) stock = 0; } catch (...) { stock = 0; }
            }

            items[count].productId = fields[0];
            items[count].barcode = fields[1];
            items[count].name = fields[2];
            items[count].description = fields[3];
            items[count].price = price;
            items[count].stock = stock;
            items[count].acronym = "";
            items[count].measurementUnit = "";
            items[count].tax = 0.0;
            items[count].discount = 0.0;
            items[count].barcodeImagePath = "";
            items[count].categoryId = "";
            count++;
        }
    }

    file.close();
    return true;
}

int ProductCatalog::getCount() const
{
    return count;
}

const ProductInfo* ProductCatalog::findByBarcode(const string& barcode) const
{
    for (int i = 0; i < count; i++)
    {
        if (items[i].barcode == barcode)
            return &items[i];
    }

    return nullptr;
}

const ProductInfo* ProductCatalog::findById(const string& productId) const
{
    for (int i = 0; i < count; i++)
    {
        if (items[i].productId == productId)
            return &items[i];
    }

    return nullptr;
}
