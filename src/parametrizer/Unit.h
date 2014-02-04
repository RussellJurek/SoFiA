#ifndef UNIT_HEADER
#define UNIT_HEADER
// (UNIT_H is needed further down for 'hour')

#define UNIT_NUMBER_BASE_UNITS  7

#define UNIT_STD 0
#define UNIT_EXP 1

// The following are non-standard units that can be used 
// to support import and export functions to/from the SI.
// They are not specifically treated or supported here,
// but the separate class "Measurement" uses them.
#define UNIT_NONE    0
#define UNIT_JY      1
#define UNIT_MJY     2
#define UNIT_DEG     3
#define UNIT_ARCMIN  4
#define UNIT_ARCSEC  5
#define UNIT_MAS     6
#define UNIT_G       7
#define UNIT_PC      8
#define UNIT_KPC     9
#define UNIT_MPC    10
#define UNIT_AU     11
#define UNIT_LY     12
#define UNIT_MIN    13
#define UNIT_H      14
#define UNIT_A      15
#define UNIT_ERG    16
#define UNIT_DYN    17
#define UNIT_C      18
#define UNIT_E      19

#include <string>
#include <map>
#include <vector>

class Unit
{
public:
                     Unit();
                     Unit(const Unit &unit);
                     Unit(const std::string &value);
    
    int              getPrefix();
    
    int              set(const std::string &value);
    std::string      printString(const unsigned int mode = UNIT_STD);
    
    void             invert();
    
    void             clear();
    bool             isEmpty();
    bool             isDefined();
    
    Unit &           operator =  (const Unit &unit);
    bool             operator == (const Unit &cmpUnit);
    bool             operator == (const std::string &cmpUnitStr);
    bool             operator != (const Unit &cmpUnit);
    bool             operator != (const std::string &cmpUnitStr);
    Unit &           operator *= (const Unit &factor);
    Unit &           operator *= (const std::string &factorStr);
    Unit             operator *  (const Unit &factor);
    Unit             operator *  (const std::string &factorStr);
    
private:
    int              prefixes;
    std::vector<int> units;
    
    
    
    // List of base units:
    
    static std::vector<std::string> createBaseUnitList();
    static const std::vector<std::string> baseUnitList;
    
    // Function for definition of named units:
    
    static std::map<std::string, std::vector<int> > createUnitMap();
    static const std::map<std::string, std::vector<int> > unitMap;
    
    // Function for definition of named prefixes:
    
    static std::map<std::string, int> createPrefixMap();
    static const std::map<std::string, int> prefixMap;
};

#endif
