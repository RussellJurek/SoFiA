#include <iostream>
#include <cmath>

#include "Measurement.h"



// Constant, static map of int and double-string pair defined as a member of a class template:
// (It probably can't get any more complicated than that!)

template <typename T> const std::map<unsigned int, std::pair<double, std::string> > Measurement<T>::conversionMap = Measurement<T>::createConversionMap();



// Constructors

template <typename T> Measurement<T>::Measurement()
{
    clear();
    
    return;
}

template <typename T> Measurement<T>::Measurement(const Measurement<T> &measurement)
{
    name        = measurement.name;
    value       = measurement.value;
    uncertainty = measurement.uncertainty;
    unit        = measurement.unit;
    
    return;
}



// Function to reset measurement:

template <typename T> void Measurement<T>::clear()
{
    name.clear();
    value = static_cast<T>(0.0);
    uncertainty = static_cast<T>(0.0);
    unit.clear();
    
    return;
}



// Functions to set parameters

template <typename T> void Measurement<T>::set(const std::string &newName, T newValue, T newUncertainty, const Unit &newUnit)
{
    if(newUncertainty < static_cast<T>(0.0)) newUncertainty *= static_cast<T>(-1.0);
    
    name        = newName;
    value       = newValue;
    uncertainty = newUncertainty;
    unit        = newUnit;
    
    return;
}

template <typename T> int Measurement<T>::set(const std::string &newName, T newValue, T newUncertainty, const std::string &newUnitStr)
{
    int  returnValue = 0;
    Unit newUnit;
    
    if(newUnit.set(newUnitStr) != 0) returnValue = 1;
    
    set(newName, newValue, newUncertainty, newUnit);
    
    return returnValue;
}

template <typename T> void Measurement<T>::setName(const std::string &newName)
{
    name = newName;
    return;
}

template <typename T> void Measurement<T>::setValue(T newValue)
{
    value = newValue;
    return;
}

template <typename T> void Measurement<T>::setUncertainty(T newUncertainty)
{
    if(newUncertainty < static_cast<T>(0.0)) newUncertainty *= static_cast<T>(-1.0);
    uncertainty = newUncertainty;
    return;
}

template <typename T> void Measurement<T>::setUnit(const Unit &newUnit)
{
    unit = newUnit;
    return;
}

template <typename T> int Measurement<T>::setUnit(const std::string &newUnitStr)
{
    if(unit.set(newUnitStr) != 0) return 1;
    else return 0;
}



// Function to set parameters with non-standard units

template <typename T> int Measurement<T>::set(const std::string &newName, T newValue, T newUncertainty, unsigned int mode)
{
    std::map<unsigned int, std::pair<double, std::string> >::const_iterator iter = conversionMap.find(mode);
    
    if(iter == conversionMap.end())
    {
        std::cerr << "Error (Measurement): Invalid conversion mode." << std::endl;
        return 1;
    }
    
    if(newUncertainty < static_cast<T>(0.0)) newUncertainty *= static_cast<T>(-1.0);
    
    double conversionFactor = (iter->second).first;
    newValue       *= static_cast<T>(conversionFactor);
    newUncertainty *= static_cast<T>(conversionFactor);
    
    Unit newUnit;
    newUnit.set((iter->second).second);
    
    this->set(newName, newValue, newUncertainty, newUnit);
    
    return 0;
}



// Functions to return name, value, uncertainty, and unit

template <typename T> std::string Measurement<T>::getName()
{
    return name;
}

template <typename T> T Measurement<T>::getValue()
{
    return value;
}

template <typename T> T Measurement<T>::getUncertainty()
{
    return uncertainty;
}

template <typename T> Unit Measurement<T>::getUnit()
{
    return unit;
}



// Function to return string with measurement
// ### This function is very basic right now, 
// ### but can be made much more complex and 
// ### powerful in the future.

template <typename T> std::string Measurement<T>::print(unsigned int mode, int decimals, bool scientific)
{
    if(mode < 0 or mode > 3) mode = 0;
    
    std::string result;
    
    if(mode == MEASUREMENT_FULL) result.append(name + " = ");
    
    if(mode != MEASUREMENT_UNIT) result.append(numberToString<T>(value, decimals, scientific));
    
    if(uncertainty != 0.0 and mode != MEASUREMENT_COMPACT and mode != MEASUREMENT_UNIT) result.append(" ± " + numberToString<T>(uncertainty, decimals, scientific));
    
    if(unit.isDefined())
    {
        if(mode != MEASUREMENT_UNIT) result.append(" ");
        if(unit.getPrefix() == 0)    result.append(unit.printString(UNIT_EXP));
        else                         result.append("× " + unit.printString(UNIT_EXP));
    }
    
    return result;
}



// Function to export parameters to non-standard units

template <typename T> int Measurement<T>::convert(T &newValue, T &newUncertainty, unsigned int mode)
{
    std::map<unsigned int, std::pair<double, std::string> >::const_iterator iter = conversionMap.find(mode);
    
    if(iter == conversionMap.end())
    {
        std::cerr << "Error (Measurement): Invalid conversion mode." << std::endl;
        return 1;
    }
    
    Unit newUnit;
    newUnit.set((iter->second).second);
    
    // Check that unit is correct:
    if(this->unit != newUnit)
    {
        std::cerr << "Error (Measurement): Wrong dimension encountered; cannot convert." << std::endl;
        return 1;
    }
    
    double conversionFactor = (iter->second).first;
    
    newValue       = static_cast<T>(value       * pow(10.0, (this->unit).getPrefix()) / conversionFactor);
    newUncertainty = static_cast<T>(uncertainty * pow(10.0, (this->unit).getPrefix()) / conversionFactor);
    
    return 0;
}



// Function to invert measurement

template <typename T> int Measurement<T>::invert()
{
    if(this->value == static_cast<T>(0.0))
    {
        std::cerr << "Error (Measurement): Value is zero; cannot invert." << std::endl;
        return 1;
    }
    
    this->uncertainty /= this->value * this->value;
    this->value        = static_cast<T>(1.0) / this->value;
    (this->unit).invert();
    this->name = this->name + "⁻¹";
    
    return 0;
}



// Assignment operator (hopefully exception-safe):

template <typename T> Measurement<T> &Measurement<T>::operator = (const Measurement<T> &measurement)
{
    this->name        = measurement.name;
    this->value       = measurement.value;
    this->uncertainty = measurement.uncertainty;
    this->unit        = measurement.unit;
    
    return *this;
}

// Comparison operators:

template <typename T> bool Measurement<T>::operator == (Measurement<T> &measurement)
{
    if(this->value * static_cast<T>(pow(10.0, (this->unit).getPrefix())) == measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix())) and this->unit == measurement.unit) return true;
    else return false;
}

template <typename T> bool Measurement<T>::operator != (Measurement<T> &measurement)
{
    return !(*this == measurement);
}

template <typename T> bool Measurement<T>::operator <= (Measurement<T> &measurement)
{
    if(this->value * static_cast<T>(pow(10.0, (this->unit).getPrefix())) <= measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix())) and this->unit == measurement.unit) return true;
    else return false;
}

template <typename T> bool Measurement<T>::operator >= (Measurement<T> &measurement)
{
    if(this->value * static_cast<T>(pow(10.0, (this->unit).getPrefix())) >= measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix())) and this->unit == measurement.unit) return true;
    else return false;
}

template <typename T> bool Measurement<T>::operator < (Measurement<T> &measurement)
{
    if(this->value * static_cast<T>(pow(10.0, (this->unit).getPrefix())) < measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix())) and this->unit == measurement.unit) return true;
    else return false;
}

template <typename T> bool Measurement<T>::operator > (Measurement<T> &measurement)
{
    if(this->value * static_cast<T>(pow(10.0, (this->unit).getPrefix())) > measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix())) and this->unit == measurement.unit) return true;
    else return false;
}

// Arithmetic operators:

// Addition:
template <typename T> Measurement<T> &Measurement<T>::operator += (Measurement<T> &measurement)
{
    if(this->unit != measurement.unit)
    {
        std::cerr << "Error (Measurement): Cannot add measurements; dimensions differ." << std::endl;
        return *this;
    }
    
    this->value += measurement.value * static_cast<T>(pow(10.0, measurement.unit.getPrefix() - (this->unit).getPrefix()));
    
    T unc1 = this->uncertainty;
    T unc2 = measurement.uncertainty * static_cast<T>(pow(10.0, measurement.unit.getPrefix() - (this->unit).getPrefix()));
    this->uncertainty = sqrt(unc1 * unc1 + unc2 * unc2);
    
    // Note: name and unit are left unchanged.
    
    return *this;
}

template <typename T> Measurement<T> Measurement<T>::operator + (Measurement<T> &measurement)
{
    Measurement newMeasurement(*this);
    
    return newMeasurement += measurement;
}

// Subtraction:
template <typename T> Measurement<T> &Measurement<T>::operator -= (Measurement<T> &measurement)
{
    if(this->unit != measurement.unit)
    {
        std::cerr << "Error (Measurement): Cannot subtract measurements; dimensions differ." << std::endl;
        return *this;
    }
    
    this->value -= (measurement.value * pow(10.0, measurement.unit.getPrefix() - (this->unit).getPrefix()));
    
    T unc1 = this->uncertainty;
    T unc2 = measurement.uncertainty * pow(10.0, measurement.unit.getPrefix() - (this->unit).getPrefix());
    this->uncertainty = sqrt(unc1 * unc1 + unc2 * unc2);
    
    // Note: name and unit are left unchanged.
    
    return *this;
}

template <typename T> Measurement<T> Measurement<T>::operator - (Measurement<T> &measurement)
{
    Measurement newMeasurement(*this);
    
    return newMeasurement -= measurement;
}

// Multiplication:
template <typename T> Measurement<T> &Measurement<T>::operator *= (const Measurement<T> &measurement)
{
    T unc1 = this->uncertainty;
    T unc2 = measurement.uncertainty;
    this->uncertainty = sqrt(measurement.value * measurement.value * unc1 * unc1 + this->value * this->value * unc2 * unc2);
    
    this->value *= measurement.value;
    
    this->unit *= measurement.unit;
    
    (this->name).append(" × " + measurement.name);
    
    return *this;
}

template <typename T> Measurement<T> Measurement<T>::operator * (const Measurement<T> &measurement)
{
    Measurement newMeasurement(*this);
    
    return newMeasurement *= measurement;
}

// Division:
template <typename T> Measurement<T> &Measurement<T>::operator /= (const Measurement<T> &measurement)
{
    Measurement divisor(measurement);
    
    if(divisor.invert() != 0)
    {
        std::cerr << "Error (Measurement): Cannot divide measurements; divisor is zero." << std::endl;
        return *this;
    }
    
    *this *= divisor;
    
    return *this;
}

template <typename T> Measurement<T> Measurement<T>::operator / (const Measurement<T> &measurement)
{
    Measurement newMeasurement(*this);
    
    return newMeasurement /= measurement;
}

// Additive inverse:
template <typename T> Measurement<T> Measurement<T>::operator - ()
{
    Measurement newMeasurement(*this);
    
    newMeasurement.value *= static_cast<T>(-1.0);
    
    return newMeasurement;
}



// ########################################################## //
// Instantiate desired templates so they can be pre-compiled: //
// ########################################################## //

template class Measurement<char>;
template class Measurement<short>;
template class Measurement<int>;
template class Measurement<long>;
template class Measurement<float>;
template class Measurement<double>;
