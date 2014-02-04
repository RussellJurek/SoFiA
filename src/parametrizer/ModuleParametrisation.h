#ifndef MODULEPARAMETRISATION_H
#define MODULEPARAMETRISATION_H


#include <string>
#include <map>

#include "SourceCatalog.h"
#include "DataCube.h"

class ModuleParametrisation
{
public:
    ModuleParametrisation();
    ~ModuleParametrisation();
    
    int run(float *d, short *m, long dx, long dy, long dz, std::map<std::string, std::string> &fitsHeader, SourceCatalog &initCatalog);
    
    SourceCatalog getCatalog()
    {
        return catalog;
    }
    
    void setFlags(bool doMO, bool doBF)
    {
        doMaskOptimisation = doMO;
        doBusyFunction     = doBF;
    }
    
    
private:
    //std::map<std::string, std::string> parameters;
    
    DataCube<float> dataCube;
    DataCube<short> maskCube;
    SourceCatalog   catalog;
    
    bool doMaskOptimisation;
    bool doBusyFunction;
    
    // Make copy constructor and assignment operator private, so they can't be called:
    ModuleParametrisation(const ModuleParametrisation&);
    ModuleParametrisation &operator=(const ModuleParametrisation&);
};

#endif
