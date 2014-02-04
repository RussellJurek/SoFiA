#include <iostream>

#include "helperFunctions.h"
#include "Unit.h"



// Create static data members holding basic unit information:

const std::vector<std::string>                 Unit::baseUnitList = Unit::createBaseUnitList();
const std::map<std::string, std::vector<int> > Unit::unitMap      = Unit::createUnitMap();
const std::map<std::string, int>               Unit::prefixMap    = Unit::createPrefixMap();



// -----------------------
// Overloaded constructor:
// -----------------------

Unit::Unit()
{
    units.resize(UNIT_NUMBER_BASE_UNITS, 0);
    
    this->clear();
    
    return;
}

Unit::Unit(const std::string &value)
{
    units.resize(UNIT_NUMBER_BASE_UNITS, 0);
    
    this->set(value);
    
    return;
}

// Copy constructor (must take const reference):

Unit::Unit(const Unit &unit)
{
    units.resize(UNIT_NUMBER_BASE_UNITS, 0);
    
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        this->units[i] = unit.units[i];
    }
    
    this->prefixes = unit.prefixes;
    
    return;
}



// --------------------------------------
// Reset unit to empty (= dimensionless):
// --------------------------------------

void Unit::clear()
{
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++) units[i] = 0;
    
    prefixes = 0;
    
    return;
}



// --------------------------------------------
// Check whether unit is empty / dimensionless:
// --------------------------------------------

bool Unit::isEmpty()
{
    bool empty = true;
    
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++) if(units[i] != 0) empty = false;
    
    return empty;
}

bool Unit::isDefined()
{
    return !(this->isEmpty());
}



// --------------
// Return prefix:
// --------------


int Unit::getPrefix()
{
    return prefixes;
}



// ------------
// Invert unit:
// ------------


void Unit::invert()
{
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        this->units[i] *= -1;
    }
    
    this->prefixes *= -1;
    
    return;
}



// ---------------------
// Overloaded operators:
// ---------------------

// Assignment operator (hopefully exception-safe):

Unit &Unit::operator = (const Unit &unit)
{
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        this->units[i] = unit.units[i];
    }
    
    this->prefixes     = unit.prefixes;
    
    return *this;
}

// Comparison operators:

bool Unit::operator == (const Unit &cmpUnit)
{
    bool same = true;
    
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        if(this->units[i] != cmpUnit.units[i]) same = false;
    }
    
    return same;
}

bool Unit::operator == (const std::string &cmpUnitStr)
{
    Unit cmpUnit(cmpUnitStr);
    
    return *this == cmpUnit;
}

bool Unit::operator != (const Unit &cmpUnit)
{
    return !(*this == cmpUnit);
}

bool Unit::operator != (const std::string &cmpUnitStr)
{
    Unit cmpUnit(cmpUnitStr);
    
    return *this != cmpUnit;
}

// Multiplication operators:

Unit &Unit::operator *= (const Unit &factor)
{
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        this->units[i] += factor.units[i];
    }
    
    this->prefixes += factor.prefixes;
    
    return *this;
}

Unit &Unit::operator *= (const std::string &factorStr)
{
    Unit factor(factorStr);
    
    return *this *= factor;
}

Unit Unit::operator * (const Unit &factor)
{
    Unit result(*this);
    
    return result *= factor;
}

Unit Unit::operator * (const std::string &factorStr)
{
    Unit result(*this);
    
    return result *= factorStr;
}



// -----------------------
// Print unit into string:
// -----------------------

std::string Unit::printString(const unsigned int mode)
{
    std::string result;
    std::string unicodeExp[10] = {"⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹"};
    
    if(prefixes != 0)
    {
        std::string strPre = numberToString<int>(prefixes);
        
        if(mode == UNIT_EXP)
        {
            for(int j = 0; j < 10; j++)
            {
                stringReplace(strPre, numberToString<int>(j), unicodeExp[j]);
            }
            
            stringReplace(strPre, "-", "⁻");
            
            result.append("10" + strPre);
        }
        else
        {
            result.append("10^" + strPre);
        }
    }
    
    for(size_t i = 0; i < UNIT_NUMBER_BASE_UNITS; i++)
    {
        if(units[i] == 1)
        {
            result.append(" " + baseUnitList[i]);
        }
        else if(units[i] != 0)
        {
            std::string strExp = numberToString<int>(units[i]);
            
            if(mode == UNIT_EXP)
            {
                for(int j = 0; j < 10; j++)
                {
                    stringReplace(strExp, numberToString<int>(j), unicodeExp[j]);
                }
                
                stringReplace(strExp, "-", "⁻");
                
                result.append(" " + baseUnitList[i] + strExp);
            }
            else
            {
                result.append(" " + baseUnitList[i] + "^" + strExp);
            }
        }
    }
    
    stringTrim(result);
    
    return result;
}



// ---------------------
// Set unit from string:
// ---------------------

int Unit::set(const std::string &value)
{
    this->clear();
    
    std::string unitString = value;
    stringTrim(unitString);
    
    if(unitString.empty()) return 0;
    
    stringReplace(unitString, "**", "");                   // remove exponentiation symbols
    stringReplace(unitString, "^",  "");
    stringReplace(unitString, "*",  " ");                  // replace multiplication symbols with space for tokenising
    stringReplace(unitString, "/",  " /");                 // pad division symbols with space for tokenising
    while(stringReplace(unitString, "/ ", "/") == 0);      // remove all spaces after division symbols
    
    std::vector<std::string> tokens;
    
    // Tokenise string using spaces:
    if(stringTok(unitString, tokens, " ") != 0)
    {
        std::cerr << "Error (Unit): Failed to tokenise unit string." << std::endl;
        this->clear();
        return 1;
    }
    
    if(tokens.size() < 1)
    {
        std::cerr << "Error (Unit): Failed to tokenise unit string." << std::endl;
        this->clear();
        return 1;
    }
    
    // Loop through all tokens:
    for(size_t i = 0; i < tokens.size(); i++)
    {
        int factorDiv = 1;     // Will become -1 if division sign '/' found.
        int exponent  = 1;     // Exponent of the unit; defaults to 1 if unspecified.
        
        
        // Check for division symbol:
        
        if(tokens[i].substr(0, 1) == "/")
        {
            tokens[i] = tokens[i].substr(1, tokens[i].size() - 1);
            factorDiv = -1;
        }
        
        // Extract exponent:
        
        size_t posExp = tokens[i].find_first_of("+-0123456789");
        
        if(posExp != std::string::npos)
        {
            exponent = stringToNumber<int>(tokens[i].substr(posExp));
        }
        
        exponent *= factorDiv;
        
        // Only proceed if unit is not unity:
        if(exponent != 0 and tokens[i] != "1")
        {
            // Extract base unit and prefix:
            bool success = false;
            
            std::map<std::string, std::vector<int> >::const_iterator iterUnit = unitMap.begin();
            do
            {
                std::map<std::string, int>::const_iterator iterPrefix = prefixMap.begin();
                do
                {
                    std::string searchKey = iterPrefix->first + iterUnit->first;
                    
                    if(searchKey == tokens[i].substr(0, posExp))
                    {
                        success = true;
                        
                        int prefix = iterPrefix->second;
                        std::vector<int> unit(iterUnit->second);
                        
                        // Take care of special case 'kg':
                        if(iterUnit->first == "g") prefix -= 3;
                        
                        for(size_t ii = 0; ii < UNIT_NUMBER_BASE_UNITS; ii++)
                        {
                            units[ii] += unit[ii] * exponent;
                        }
                        
                        prefixes += prefix * exponent;
                    }
                    
                    iterPrefix++;
                }
                while(success == false and iterPrefix != prefixMap.end());
                
                iterUnit++;
            }
            while(success == false and iterUnit != unitMap.end());
            
            if(success == false)
            {
                std::cerr << "Error (Unit): Unknown unit: \'" << value << "\'."   << std::endl;
                std::cerr << "              Creating dimensionless unit instead." << std::endl;
                clear();
                return 1;
            }
        }
    }
    
    return 0;
}


std::vector<std::string> Unit::createBaseUnitList()
{
    std::vector<std::string> baseUnitList;
    
    baseUnitList.push_back("kg");
    baseUnitList.push_back("m");
    baseUnitList.push_back("s");
    baseUnitList.push_back("A");
    baseUnitList.push_back("K");
    baseUnitList.push_back("mol");
    baseUnitList.push_back("cd");
    
    return baseUnitList;
}

std::map<std::string, std::vector<int> > Unit::createUnitMap()
{
    std::map<std::string, std::vector<int> > unitMap;
    
    std::vector<int> value(UNIT_NUMBER_BASE_UNITS, 0);
    
    // (Kilo)gram (Internally treated as 'kg', but search key is 'g'. Prefix will be corrected in function set().)
    value[0] = 1;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("g", value));
    
    // Metre
    value[0] = 0;
    value[1] = 1;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("m", value));
    
    // Second
    value[0] = 0;
    value[1] = 0;
    value[2] = 1;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("s", value));
    
    // Ampere
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value[3] = 1;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("A", value));
    
    // Kelvin
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    value[4] = 1;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("K", value));
    
    // Mole
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 1;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("mol", value));
    
    // Candela / Lumen
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 1;
    unitMap.insert(std::pair<std::string, std::vector<int> >("cd", value));
    unitMap.insert(std::pair<std::string, std::vector<int> >("lm", value));
    
    // Radian / Steradian
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("rad", value));
    unitMap.insert(std::pair<std::string, std::vector<int> >("sr", value));
    
    // Hertz / Becquerel
    value[0] = 0;
    value[1] = 0;
    value[2] = -1;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("Hz", value));
    unitMap.insert(std::pair<std::string, std::vector<int> >("Bq", value));
    
    // Newton
    value[0] = 1;
    value[1] = 1;
    value[2] = -2;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("N", value));
    
    // Pascal
    value[0] = 1;
    value[1] = -1;
    value[2] = -2;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("Pa", value));
    
    // Joule
    value[0] = 1;
    value[1] = 2;
    value[2] = -2;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("J", value));
    
    // Watt
    value[0] = 1;
    value[1] = 2;
    value[2] = -3;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("W", value));
    
    // Coulomb
    value[0] = 0;
    value[1] = 0;
    value[2] = 1;
    value[3] = 1;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("C", value));
    
    // Volt
    value[0] = 1;
    value[1] = 2;
    value[2] = -3;
    value[3] = -1;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("V", value));
    
    // Farad
    value[0] = -1;
    value[1] = -2;
    value[2] = 4;
    value[3] = 2;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("F", value));
    
    // Ohm
    value[0] = 1;
    value[1] = 2;
    value[2] = -3;
    value[3] = -2;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("Ω", value));
    
    // Siemens
    value[0] = -1;
    value[1] = -2;
    value[2] = 3;
    value[3] = 2;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("S", value));
    
    // Weber
    value[0] = 1;
    value[1] = 2;
    value[2] = -2;
    value[3] = -1;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("Wb", value));
    
    // Tesla
    value[0] = 1;
    value[1] = 0;
    value[2] = -2;
    value[3] = -1;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("T", value));
    
    // Henry
    value[0] = 1;
    value[1] = 2;
    value[2] = -2;
    value[3] = -2;
    value[4] = 0;
    value[5] = 0;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("H", value));
    
    // Lux
    value[0] = 0;
    value[1] = -2;
    value[2] = 0;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 1;
    unitMap.insert(std::pair<std::string, std::vector<int> >("lx", value));
    
    // Gray / Sievert
    value[0] = 0;
    value[1] = 2;
    value[2] = -2;
    value[3] = 0;
    value[4] = 0;
    value[5] = 0;
    value[6] = 1;
    unitMap.insert(std::pair<std::string, std::vector<int> >("Gy", value));
    unitMap.insert(std::pair<std::string, std::vector<int> >("Sv", value));
    
    // Katal
    value[0] = 0;
    value[1] = 0;
    value[2] = -1;
    value[3] = 0;
    value[4] = 0;
    value[5] = 1;
    value[6] = 0;
    unitMap.insert(std::pair<std::string, std::vector<int> >("kat", value));
    
    return unitMap;
}

std::map<std::string, int> Unit::createPrefixMap()
{
    std::map<std::string, int> prefixMap;
    
    prefixMap.insert(std::pair<std::string, int>("",    0));       // none
    prefixMap.insert(std::pair<std::string, int>("da",  1));       // deca
    prefixMap.insert(std::pair<std::string, int>("h",   2));       // hecto
    prefixMap.insert(std::pair<std::string, int>("k",   3));       // kilo
    prefixMap.insert(std::pair<std::string, int>("M",   6));       // mega
    prefixMap.insert(std::pair<std::string, int>("G",   9));       // giga
    prefixMap.insert(std::pair<std::string, int>("T",  12));       // tera
    prefixMap.insert(std::pair<std::string, int>("P",  15));       // peta
    prefixMap.insert(std::pair<std::string, int>("E",  18));       // exa
    prefixMap.insert(std::pair<std::string, int>("Z",  21));       // zetta
    prefixMap.insert(std::pair<std::string, int>("Y",  24));       // yotta
    prefixMap.insert(std::pair<std::string, int>("d",  -1));       // deci
    prefixMap.insert(std::pair<std::string, int>("c",  -2));       // centi
    prefixMap.insert(std::pair<std::string, int>("m",  -3));       // milli
    prefixMap.insert(std::pair<std::string, int>("µ",  -6));       // micro
    prefixMap.insert(std::pair<std::string, int>("n",  -9));       // nano
    prefixMap.insert(std::pair<std::string, int>("p", -12));       // pico
    prefixMap.insert(std::pair<std::string, int>("f", -15));       // femto
    prefixMap.insert(std::pair<std::string, int>("a", -18));       // atto
    prefixMap.insert(std::pair<std::string, int>("z", -21));       // zepto
    prefixMap.insert(std::pair<std::string, int>("y", -24));       // yocto
    
    return prefixMap;
}







