//
// Created by siyuan on 06/05/2021.
//
#include <iomanip>
#include <iostream>
#include <cmath>
#include <string>
#include <bitset>

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

/**
 * Hex-encode a string. Implementation comes from the following post:
 * https://stackoverflow.com/questions/3381614/c-convert-string-to-hexadecimal-and-vice-versa
 * @param input: the string to be encoded.
 * @return hex-encoded string.
 */
std::string hexEncode(const std::string& input)
{
    static const char hexDigits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
        output.push_back('\\');
        output.push_back('x');
        output.push_back(hexDigits[c >> 4]);
        output.push_back(hexDigits[c & 15]);
    }
    return output;
}


/**
 * Checks if the bit at the given index of a BitField string is set to
 * 1 or not.
 */
bool hasPiece(const std::string& bitField, int index)
{
    int byteIndex = floor(index / 8);
    int offset = index % 8;
    return (bitField[byteIndex] >> (7 - offset) & 1) != 0;
}

/**
 * Sets the given index of the Bitfield to 1.
 */
void setPiece(std::string& bitField, int index)
{
    int byteIndex = floor(index / 8);
    int offset = index % 8;
    bitField[byteIndex] |= (1 << (7 - offset));
}

std::string toBinary(int n)
{
    std::string r;
    while(n!=0) {r=(n%2==0 ?"0":"1")+r; n/=2;}
    return r;
}

/**
 * Converts a series of bytes in a string format to an integer.
 */
int bytesToInt(std::string bytes)
{
    // FIXME: Use bitwise operation to convert
    std::string binStr;
    int byteCount = bytes.size();
    for (int i = 0; i < byteCount; i++)
        binStr += std::bitset<8>(bytes[i]).to_string();
    return stoi(binStr, 0, 2);
}