#include <fstream>
#include <iostream>
#include <string>

#include "helperFunctions.h"
#include "ModuleParametrisation.h"
#include "MaskOptimization.h"
#include "Parametrization.h"



// -------------------------- //
// Constructor and destructor //
// -------------------------- //

ModuleParametrisation::ModuleParametrisation()
{
    doMaskOptimisation = true;
    doBusyFunction     = true;
    
    return;
}

ModuleParametrisation::~ModuleParametrisation()
{
    return;
}



// -------------------------------------- //
// Function to run parametrisation module //
// -------------------------------------- //

int ModuleParametrisation::run(float *d, short *m, long dx, long dy, long dz, std::map<std::string, std::string> &fitsHeader, SourceCatalog &initCatalog)
{
    // Check data
    
    if(d == 0 or m == 0)
    {
        std::cerr << "Error (ModParam): Invalid data pointer provided." << std::endl;
        return 1;
    }
    
    if(dx <= 0 or dy <= 0 or dz <= 0)
    {
        std::cerr << "Error (ModParam): Invalid data cube dimensions." << std::endl;
        return 1;
    }
    
    dataCube.createNewCubeFromPointer(dx, dy, dz, d);
    maskCube.createNewCubeFromPointer(dx, dy, dz, m);
    
    for(std::map<std::string, std::string>::iterator it = fitsHeader.begin(); it != fitsHeader.end(); it++)
    {
        dataCube.setHeader(it->first, it->second);
    }
    
    catalog = initCatalog;
    
    for(std::map<unsigned long, Source>::iterator it = catalog.getSources().begin(); it != catalog.getSources().end(); it++)
    {
        long i = it->first;
        Source *source = catalog.getSource(i);
        
        if(source == 0)
        {
            std::cerr << "Error (ModParam): Failed to find requested catalogue source " << i << "." << std::endl;
            return 1;
        }
        
        // Pipeline: Run mask optimisation algorithm:
        if(doMaskOptimisation == true)
        {
            std::cout << "Mask optimisation of source " << i << std::endl;
            
            MaskOptimization maskOptimization;
            
            if(maskOptimization.optimize(&dataCube, &maskCube, source) != 0)
            {
                std::cerr << "Error (ModParam): Mask optimisation failed for source " << source->getSourceID() << "." << std::endl;
            }
        }
        
        // Pipeline: Run parametrisation algorithm:
        std::cout << "Parametrisation of source " << i << std::endl;
        
        Parametrization parametrization;
        
        if(parametrization.parametrize(&dataCube, &maskCube, source, doBusyFunction) != 0)
        {
            std::cerr << "Error (ModParam): Parametrisation failed for source " << source->getSourceID() << "." << std::endl;
        }
    }
    
    return 0;
}
