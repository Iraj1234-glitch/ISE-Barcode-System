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

        string fields[6];
        int fieldCount = splitQuoted(cleaned, fields, 6);

        if (fieldCount < 5)
            continue;

        if (!isValidBarcode(fields[1]))
            continue;

        double price = 0.0;

        try
        {
            price = stod(fields[4]);
        }
        catch (...)
        {
            continue;
        }

        if (price < 0.0 || count >= MAX_CATALOG_PRODUCTS)
            continue;

        items[count].productId = fields[0];
        items[count].barcode = fields[1];
        items[count].name = fields[2];
        items[count].description = fields[3];
        items[count].price = price;
        count++;
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
