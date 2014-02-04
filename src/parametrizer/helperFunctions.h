#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#define TRIM_BOTH  0
#define TRIM_LEFT  1
#define TRIM_RIGHT 2

#define MATH_CONST_PI 3.141592653589793
#define MATH_CONST_E  2.718281828459045

#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <iomanip>
#include <typeinfo>


// String functions:

int  stringToUpper(std::string &strToConvert);
int  stringToLower(std::string &strToConvert);

void stringTrim(std::string &strToConvert, int mode = TRIM_BOTH);
int  stringTok(std::string &strToConvert, std::vector<std::string> &tokens, const std::string &delimiters = " ");

int stringReplace(std::string &strToConvert, const std::string &seachStr, const std::string &replStr);

std::string degToDms(double number);
std::string degToHms(double number);



// Mathematical functions:

int mathSgn(int value);

double mathAbs(double value);

long mathRound(float value);
long mathRound(double value);



// String-number conversion templates:

template <typename T> std::string numberToString(T number, int decimals = -1, bool scientific = false)
{
    std::ostringstream convert;
    
    if(decimals >= 0 and (typeid(T) == typeid(float) or typeid(T) == typeid(double)))
    {
        if(decimals > std::numeric_limits<T>::digits10 + 1) decimals = std::numeric_limits<T>::digits10 + 1;
        
        // Choose between fixed-point and scientific notation for float and double types:
        if(scientific == true) convert << std::scientific;
        else                   convert << std::fixed;
        
        // Set the number of digits for float and double types:
        convert << std::setprecision(decimals);
    }
    
    // Let's hope that this makes any sense for a particular type T:
    convert << number;
    
    return convert.str();
};



// Template function to convert std::string to numerical value of type T:
// WARNING: This function does not correctly convert numbers in scientific 
// notation to integer types! For example, "-3.95E+1" will become -39.5 
// when converted to float, but -3 when converted to int!
template <typename T> T stringToNumber(const std::string &strToConvert)
{
    std::stringstream stringStream(strToConvert);
    T result;
    return stringStream >> result ? result : 0;
};



#endif
