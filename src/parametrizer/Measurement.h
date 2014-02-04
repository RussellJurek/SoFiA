#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#define MEASUREMENT_DEFAULT 0
#define MEASUREMENT_COMPACT 1
#define MEASUREMENT_FULL    2
#define MEASUREMENT_UNIT    3

#include <string>

#include "helperFunctions.h"
#include "Unit.h"

template <typename T> class Measurement
{
public:
                    Measurement();
                    Measurement(const Measurement<T> &measurement);
    
    void            clear();
    
    void            set(const std::string &newName, T newValue, T newUncertainty, const Unit &newUnit);
    int             set(const std::string &newName, T newValue, T newUncertainty, const std::string &newUnitStr);
    int             set(const std::string &newName, T newValue, T newUncertainty, unsigned int mode);
    void            setName(const std::string &newName);
    void            setValue(T newValue);
    void            setUncertainty(T newValue);
    void            setUnit(const Unit &newUnit);
    int             setUnit(const std::string &newUnitStr);
    
    std::string     getName();
    T               getValue();
    T               getUncertainty();
    Unit            getUnit();
    
    std::string     print(unsigned int mode = MEASUREMENT_DEFAULT, int decimals = -1, bool scientific = false);
    
    int             convert(T &newValue, T &newUncertainty, unsigned int mode);
    
    int             invert();
    
    bool            operator == (Measurement<T> &measurement);
    bool            operator != (Measurement<T> &measurement);
    bool            operator <= (Measurement<T> &measurement);
    bool            operator >= (Measurement<T> &measurement);
    bool            operator <  (Measurement<T> &measurement);
    bool            operator >  (Measurement<T> &measurement);
    Measurement<T> &operator =  (const Measurement<T> &measurement);
    Measurement<T> &operator += (Measurement<T> &measurement);
    Measurement<T> &operator -= (Measurement<T> &measurement);
    Measurement<T> &operator *= (const Measurement<T> &measurement);
    Measurement<T> &operator /= (const Measurement<T> &measurement);
    Measurement<T>  operator +  (Measurement<T> &measurement);
    Measurement<T>  operator -  (Measurement<T> &measurement);
    Measurement<T>  operator *  (const Measurement<T> &measurement);
    Measurement<T>  operator /  (const Measurement<T> &measurement);
    Measurement<T>  operator -  ();
    
private:
    std::string     name;
    T               value;
    T               uncertainty;
    Unit            unit;
    
    
    
    // Static members that store some basic information for conversion of units:
    
    static std::map<unsigned int, std::pair<double, std::string> > createConversionMap()
    {
        std::map<unsigned int, std::pair<double, std::string> > conversionMap;
        
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_JY,     std::pair<double, std::string>(1.0e-26,                 "W/m2/Hz"))); // Jansky
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_MJY,    std::pair<double, std::string>(1.0e-29,                 "W/m2/Hz"))); // mJy
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_DEG,    std::pair<double, std::string>(MATH_CONST_PI / 180.0,   "rad")));     // degree
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_ARCMIN, std::pair<double, std::string>(MATH_CONST_PI / 1.08e+4, "rad")));     // arcmin
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_ARCSEC, std::pair<double, std::string>(MATH_CONST_PI / 6.48e+5, "rad")));     // arcsec
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_MAS,    std::pair<double, std::string>(MATH_CONST_PI / 6.48e+8, "rad")));     // mas
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_G,      std::pair<double, std::string>(1.0e-4,                  "T")));       // Gauß
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_PC,     std::pair<double, std::string>(3.0856775814671900e+16,  "m")));       // parsec
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_KPC,    std::pair<double, std::string>(3.0856775814671900e+19,  "m")));       // kpc
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_MPC,    std::pair<double, std::string>(3.0856775814671900e+22,  "m")));       // Mpc
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_LY,     std::pair<double, std::string>(9.460730472580800e+15,   "m")));       // lightyear
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_AU,     std::pair<double, std::string>(1.49597870700e+11,       "m")));       // AU
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_MIN,    std::pair<double, std::string>(60.0,                    "s")));       // minute
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_H,      std::pair<double, std::string>(3600.0,                  "s")));       // hour
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_A,      std::pair<double, std::string>(3.1557600e+7,            "s")));       // year
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_ERG,    std::pair<double, std::string>(1.0e-7,                  "J")));       // erg
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_DYN,    std::pair<double, std::string>(1.0e-5,                  "N")));       // dyne
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_C,      std::pair<double, std::string>(299792458,               "m/s")));     // light sp.
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_E,      std::pair<double, std::string>(1.602176565e-19,         "C")));       // elem. ch.
        conversionMap.insert(std::pair<unsigned int, std::pair<double, std::string> >(UNIT_NONE,   std::pair<double, std::string>(1.0,                     "")));        // no unit
        
        return conversionMap;
    }
    
    static const std::map<unsigned int, std::pair<double, std::string> > conversionMap;
};

#endif
