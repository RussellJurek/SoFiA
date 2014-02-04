#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <map>

#include "Measurement.h"

class Source
{
public:
    Source();
    Source(const Source &s);
    
    bool          isDefined();
    bool          parameterDefined(const std::string &name);
//     unsigned int  findParameter(const std::string &name);
    
    int           setParameter(const std::string &parameter, double value, double uncertainty, std::string &unit);
    int           setParameter(const std::string &parameter, double value, double uncertainty = 0.0);
    int           setParameter(Measurement<double> &measurement);
    Measurement<double> getParameterMeasurement(const std::string &parameter);
    double        getParameter(const std::string &parameter);
    
    int           setSourceID(unsigned long sid);
    unsigned long getSourceID();
    
    int           setSourceName(const std::string &name);
    std::string   getSourceName();
    
    std::map<std::string,Measurement<double> > getParameters()
    {
        return parameters;
    }
    void setParameters(std::map<std::string,Measurement<double> > params)
    {
        parameters=params;
    }
    void clear()
    {
        parameters.clear();
    }
    
    
private:
    unsigned long sourceID;
    std::string   sourceName;
    std::map<std::string,Measurement<double> > parameters;      // Space after first '>' required, otherwise g++ will complain
};

#endif
