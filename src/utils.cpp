//
// Created by siyuan on 06/05/2021.
//
#include <iomanip>
#include <iostream>
#include <string>

/**
 * URL-encodes the given string.
 * Implementation from https://stackoverflow.com/questions/154536/encode-decode-urls-in-c.
 * @param value: the string to be url-encoded.
 * @return the encoded string.
 */
std::string urlEncode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value)
    {
        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

/**
 * Converts a string in hexadecimal form to a string in char form.
 * Implementation from https://stackoverflow.com/questions/3790613/how-to-convert-a-string-of-hex-values-to-a-string.
 */
std::string hexDecode(const std::string& value)
{
    int hashLength = value.length();
    std::string decodedHexString;
    for (int i = 0; i < hashLength; i += 2)
    {
        std::string byte = value.substr(i, 2);
        char c = (char) (int) strtol(byte.c_str(), nullptr, 16);
        decodedHexString.push_back(c);
    }
    return decodedHexString;
}