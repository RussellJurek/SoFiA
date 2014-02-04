// Compilation: g++ -c -o Source.o Source.cpp

#include <iostream>
#include <string>
#include <limits>
#include <map>

#include "Source.h"


// ----------- //
// Constructor //
// ----------- //

Source::Source()
{
    // Initialise parameters:
    sourceID   = 0L;
    sourceName = "";
    
    return;
}

Source::Source(const Source &s)
{
    sourceID   = s.sourceID;
    sourceName = s.sourceName;
    parameters = s.parameters;
    
    return;
}

// --------------------------------------- //
// Functions to check if source is defined //
// --------------------------------------- //

bool Source::isDefined()
{
    if(parameters.empty()) return false;
    else return true;
}


// ------------------------------------------------- //
// Functions to check if source parameter is defined //
// ------------------------------------------------- //

bool Source::parameterDefined(const std::string &name)
{
    if (parameters.find(name)==parameters.end())
        return false;
    else
        return true;
}




// ---------------------------------- //
// Functions to set source parameters //
// ---------------------------------- //

int Source::setParameter(Measurement<double> &measurement)
{
    parameters[measurement.getName()]=measurement;
    return 0;
}

int Source::setParameter(const std::string &parameter, double value, double uncertainty)
{
    std::string unit = "";
    return setParameter(parameter, value, uncertainty, unit);
}

int Source::setParameter(const std::string &parameter, double value, double uncertainty, std::string &unit)
{
    
    Measurement<double> tmp;
    if(tmp.set(parameter, value, uncertainty, unit) != 0)
    {
        std::cerr << "Error (Source): Failed to set source parameter." << std::endl;
        return 1;
    }
    parameters[tmp.getName()]=tmp;
    return 0;
}


// --------------------------------- //
// Function to get source parameters //
// --------------------------------- //

Measurement<double> Source::getParameterMeasurement(const std::string &parameter)
{
    
    if (parameters.find(parameter)==parameters.end())
    {
        std::cerr << "Error (Source): Source parameter \'" << parameter << "\' not found." << std::endl;
        Measurement<double> tmp;
        if(std::numeric_limits<double>::has_quiet_NaN)
        {
            double nan=std::numeric_limits<double>::quiet_NaN();         // Return NaN if available
            tmp.set("notfound",nan,nan,"");
            
        }
        else
        {
            tmp.set("notfound",0.,0.,"");                              // Otherwise return 0
        }
        return tmp;
    }
    else
    {
        return parameters[parameter];
    }
}

double Source::getParameter(const std::string &parameter)
{
    
    if (parameters.find(parameter)==parameters.end())
    {
        std::cerr << "Error (Source): Source parameter \'" << parameter << "\' not found." << std::endl;
        Measurement<double> tmp;
        if(std::numeric_limits<double>::has_quiet_NaN)
        {
            double nan=std::numeric_limits<double>::quiet_NaN();         // Return NaN if available
            tmp.set("notfound",nan,nan,"");
            
        }
        else
        {
            tmp.set("notfound",0.,0.,"");                              // Otherwise return 0
        }
        return tmp.getValue();
    }
    else
    {
        return parameters[parameter].getValue();
    }
}


// --------------------------------- //
// Function to set and get source ID //
// --------------------------------- //

int Source::setSourceID(unsigned long sid)
{
    sourceID = sid;
    
    return 0;
}

unsigned long Source::getSourceID()
{
    return sourceID;
}


// ----------------------------------- //
// Function to set and get source Name //
// ----------------------------------- //

int Source::setSourceName(const std::string &name)
{
    sourceName = name;
    
    return 0;
}

std::string Source::getSourceName()
{
    return sourceName;
}
