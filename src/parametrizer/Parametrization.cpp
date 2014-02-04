#include <iostream>
#include <cmath>
#include <limits>

#include "helperFunctions.h"
#include "DataCube.h"
#include "Parametrization.h"
#include "BusyFit.h"
#include "Measurement.h"

Parametrization::Parametrization()
{
    dataCube = 0;
    maskCube = 0;
    source   = 0;
    
    centroidX            = 0.0;
    centroidY            = 0.0;
    centroidZ            = 0.0;
    lineWidthW20         = 0.0;
    lineWidthW50         = 0.0;
    peakFlux             = 0.0;
    integratedFlux       = 0.0;
    totalFlux            = 0.0;
    busyFitSuccess       = 0;
    busyFunctionChi2     = 0.0;
    busyFunctionCentroid = 0.0;
    busyFunctionW20      = 0.0;
    busyFunctionW50      = 0.0;
    busyFunctionFpeak    = 0.0;
    busyFunctionFint     = 0.0;
    
    for(size_t i = 0; i < BUSYFIT_FREE_PARAM; i++)
    {
        busyFitParameters[i]    = 0.0;
        busyFitUncertainties[i] = 0.0;
    }
    
    return;
}

int Parametrization::parametrize(DataCube<float> *d, DataCube<short> *m, Source *s, bool doBF)
{
    doBusyFunction = doBF;
    
    if(loadData(d, m, s) != 0)
    {
        std::cerr << "Error (Parametrization): Source parametrisation failed." << std::endl;
        return 1;
    }
    
    /*if(createMomentZeroMap() != 0)
    {
        std::cerr << "Error (Parametrization): Failed to create moment-zero map." << std::endl;
        return 1;
    }*/
    
    if(createIntegratedSpectrum() != 0)
    {
        std::cerr << "Error (Parametrization): Failed to create integrated spectrum." << std::endl;
        return 1;
    }
    
    if(measureCentroid() != 0)
    {
        std::cerr << "Error (Parametrization): Failed to measure source centroid." << std::endl;
        return 1;
    }
    
    if(measureFlux() != 0)
    {
        std::cerr << "Error (Parametrization): Source flux measurement failed." << std::endl;
        return 1;
    }
    
    if(doBusyFunction == true)
    {
        if(fitBusyFunction() != 0)
        {
            std::cerr << "Error (Parametrization): Failed to fit Busy Function." << std::endl;
            return 1;
        }
    }
    
    if(writeParameters() != 0)
    {
        std::cerr << "Error (Parametrization): Failed to write parameters to source." << std::endl;
        return 1;
    }
    
    return 0;
}

int Parametrization::loadData(DataCube<float> *d, DataCube<short> *m, Source *s)
{
    dataCube = 0;
    maskCube = 0;
    source = 0;
    
    data.clear();            // Clear all previously defined data.
    
    if(d == 0 or m == 0 or s == 0)
    {
        std::cerr << "Error (Parametrization): Cannot load data; invalid pointer provided." << std::endl;
        return 1;
    }
    
    if(!d->isDefined() or !m->isDefined() or !s->isDefined())
    {
        std::cerr << "Error (Parametrization): Cannot load data; source or data cube undefined." << std::endl;
        return 1;
    }
    
    if(d->getSize(0) != m->getSize(0) or d->getSize(1) != m->getSize(1) or d->getSize(2) != m->getSize(2))
    {
        std::cerr << "Error (Parametrization): Mask and data cube have different sizes." << std::endl;
        return 1;
    }
    
    double posX = s->getParameter("X");
    double posY = s->getParameter("Y");
    double posZ = s->getParameter("Z");
    
    if(posX < 0.0 or posY < 0.0 or posZ < 0.0 or posX >= static_cast<double>(d->getSize(0)) or posY >= static_cast<double>(d->getSize(1)) or posZ >= static_cast<double>(d->getSize(2)))
    {
        std::cerr << "Error (Parametrization): Source position outside cube range." << std::endl;
        return 1;
    }
    
    dataCube = d;
    maskCube = m;
    source   = s;
    
    // Define sub-region to operate on:
    
    if(source->parameterDefined("BBOX_X_MIN") and source->parameterDefined("BBOX_X_MAX"))
    {
        searchRadiusX = static_cast<long>(source->getParameter("BBOX_X_MAX") - source->getParameter("BBOX_X_MIN"));
    }
    else
    {
        searchRadiusX = PARAMETRIZATION_DEFAULT_SPATIAL_RADIUS;
        std::cerr << "Warning (MaskOptimization): No bounding box defined; using default search radius\n";
        std::cerr << "                            in the spatial domain instead." << std::endl;
    }
    
    if(source->parameterDefined("BBOX_Y_MIN") and source->parameterDefined("BBOX_Y_MAX"))
    {
        searchRadiusY = static_cast<long>(source->getParameter("BBOX_Y_MAX") - source->getParameter("BBOX_Y_MIN"));
    }
    else
    {
        searchRadiusY = PARAMETRIZATION_DEFAULT_SPATIAL_RADIUS;
        std::cerr << "Warning (MaskOptimization): No bounding box defined; using default search radius\n";
        std::cerr << "                            in the spatial domain instead." << std::endl;
    }
    
    if(source->parameterDefined("BBOX_Z_MIN") and source->parameterDefined("BBOX_Z_MAX"))
    {
        searchRadiusZ = static_cast<long>(0.6 * (source->getParameter("BBOX_Z_MAX") - source->getParameter("BBOX_Z_MIN")));
    }
    else
    {
        searchRadiusZ = PARAMETRIZATION_DEFAULT_SPECTRAL_RADIUS;
        std::cerr << "Warning (MaskOptimization): No bounding box defined; using default search radius\n";
        std::cerr << "                            in the spectral domain instead." << std::endl;
    }
    
    subRegionX1 = static_cast<long>(posX) - searchRadiusX;
    subRegionX2 = static_cast<long>(posX) + searchRadiusX;
    subRegionY1 = static_cast<long>(posY) - searchRadiusY;
    subRegionY2 = static_cast<long>(posY) + searchRadiusY;
    subRegionZ1 = static_cast<long>(posZ) - searchRadiusZ;
    subRegionZ2 = static_cast<long>(posZ) + searchRadiusZ;
    
    if(subRegionX1 < 0L) subRegionX1 = 0L;
    if(subRegionY1 < 0L) subRegionY1 = 0L;
    if(subRegionZ1 < 0L) subRegionZ1 = 0L;
    if(subRegionX2 >= dataCube->getSize(0)) subRegionX2 = dataCube->getSize(0) - 1L;
    if(subRegionY2 >= dataCube->getSize(1)) subRegionY2 = dataCube->getSize(1) - 1L;
    if(subRegionZ2 >= dataCube->getSize(2)) subRegionZ2 = dataCube->getSize(2) - 1L;
    
    for(long x = subRegionX1; x <= subRegionX2; x++)
    {
        for(long y = subRegionY1; y <= subRegionY2; y++)
        {
            for(long z = subRegionZ1; z <= subRegionZ2; z++)
            {
                // Add only those pixels that are masked as being part of the source:
                if(static_cast<unsigned short>(maskCube->getData(x, y, z)) == source->getSourceID())
                {
                    struct DataPoint dataPoint;
                    
                    dataPoint.x = x;
                    dataPoint.y = y;
                    dataPoint.z = z;
                    dataPoint.value = dataCube->getData(x, y, z);
                    // ### WARNING: A std::bad_alloc exception occurs later on when dataPoint.value is set to a constant of 1.0! No idea why...
                    
                    data.push_back(dataPoint);
                }
            }
        }
    }
    
    if(data.empty() == true)
    {
        std::cerr << "Error (Parametrization): No data found for source " << source->getSourceID() << "." << std::endl;
        return 1;
    }
    
    return 0;
}



// Create moment-0 map:

/*int Parametrization::createMomentZeroMap()
{
    if(data.empty())
    {
        std::cerr << "Error (Parametrization): No data loaded." << std::endl;
        return 1;
    }
    
    DataCube<float> moment0;
    
    if(moment0.createNewCube(subRegionX2 - subRegionX1 + 1, subRegionY2 - subRegionY1 + 1, subRegionZ2 - subRegionZ1 + 1) != 0)
    {
        std::cerr << "Error (Parametrization): Failed to create moment map." << std::endl;
        return 1;
    }
    
    moment0.floodFill(0.0);
    
    for(long x = 0; x <= subRegionX2 - subRegionX1; x++)
    {
        for(long y = 0; y <= subRegionY2 - subRegionY1; y++)
        {
            for(long z = 0; z <= subRegionZ2 - subRegionZ1; z++)
            {
                moment0.addData(dataCube->getData(x + subRegionX1, y + subRegionY1, z + subRegionZ1), x, y, z);
            }
        }
    }
    
    return 0;
}*/



// Measure centroid:

int Parametrization::measureCentroid()
{
    if(data.empty() or spectrum.empty())
    {
        std::cerr << "Error (Parametrization): No data loaded." << std::endl;
        return 1;
    }
    
    double sum = 0.0;
    centroidX  = 0.0;
    centroidY  = 0.0;
    centroidZ  = 0.0;
    
    for(size_t i = 0; i < data.size(); i++)
    {
        centroidX += data[i].value * data[i].x;
        centroidY += data[i].value * data[i].y;
        centroidZ += data[i].value * data[i].z;
        sum       += data[i].value;
    }
    
    centroidX /= sum;
    centroidY /= sum;
    centroidZ /= sum;
    
    return 0;
}



// Measure peak flux and integrated flux:

int Parametrization::measureFlux()
{
    if(data.empty() == true)
    {
        std::cerr << "Error (Parametrization): No data loaded." << std::endl;
        return 1;
    }
    
    totalFlux = 0.0;
    peakFlux  = -std::numeric_limits<double>::max();
    
    // Sum over all pixels:
    for(unsigned long i = 0L; i < data.size(); i++)
    {
        totalFlux += static_cast<double>(data[i].value);
        
        if(peakFlux < static_cast<double>(data[i].value)) peakFlux = static_cast<double>(data[i].value);
    }
    
    // Multiply with channel width:
    double cdelt3 = 1.0;
    
    if(dataCube->getHeader("CDELT3", cdelt3) != 0)
    {
        std::cerr << "Warning (Parametrization): No spectral channel width information found." << std::endl;
        std::cerr << "                           Flux measurement will not be adjusted." << std::endl;
    }
    
    integratedFlux = totalFlux * cdelt3;
    
    // Correct for beam size:
    double bmaj = sqrt(4.0 * log(2.0) / MATH_CONST_PI);
    double bmin = bmaj;
    
    if(dataCube->getHeader("BMAJ", bmaj) != 0)
    {
        std::cerr << "Warning (Parametrization): No beam information found." << std::endl;
        std::cerr << "                           Flux measurement will not be adjusted." << std::endl;
    }
    else if(dataCube->getHeader("BMIN", bmin) != 0)
    {
        std::cerr << "Warning (Parametrization): No beam minor axis information found." << std::endl;
        std::cerr << "                           Assuming circular beam for flux measurement." << std::endl;
        bmin = bmaj;
    }
    
    double cdelt1 = 1.0;
    double cdelt2 = 1.0;
    
    if(dataCube->getHeader("CDELT1", cdelt1) != 0)
    {
        std::cerr << "Warning (Parametrization): No longitude pixel size information found." << std::endl;
        std::cerr << "                           Flux measurement will not be adjusted." << std::endl;
    }
    else if(dataCube->getHeader("CDELT2", cdelt2) != 0)
    {
        std::cerr << "Warning (Parametrization): No latitude pixel size information found." << std::endl;
        std::cerr << "                           Assuming square pixels." << std::endl;
        cdelt2 = cdelt1;
    }
    
    cdelt1 = mathAbs(cdelt1);
    cdelt2 = mathAbs(cdelt2);
    
    if(mathAbs(cdelt1 - cdelt2) > cdelt1 * 0.01)
    {
        // Make sure that pixels are square-shaped within 1% accuracy:
        std::cerr << "Warning (Parametrization): Pixels deviate from square shape by more than 1%." << std::endl;
        std::cerr << "                           Flux measurement will not be adjusted." << std::endl;
        
        cdelt1 = 1.0;
        cdelt2 = 1.0;
    }
    else
    {
        // Average pixel sizes:
        cdelt1 = 0.5 * (cdelt1 + cdelt2);
    }
    
    double beamCorr = MATH_CONST_PI * (bmaj / cdelt1) * (bmin / cdelt1) / (4.0 * log(2.0));
    
    if(beamCorr < 0.5 or beamCorr > 50.0)
    {
        std::cerr << "Warning (Parametrization): Unusual beam correction factor of " << beamCorr << " found." << std::endl;
        std::cerr << "                           Values normally range from about 1 to 20." << std::endl;
    }
    
    if(beamCorr == 0.0)
    {
        // In C++ the above should be true for both +0 and -0.
        std::cerr << "Warning (Parametrization): Beam correction factor is zero." << std::endl;
        std::cerr << "                           Flux measurement will not be adjusted." << std::endl;
        beamCorr = 1.0;
    }
    
    integratedFlux /= beamCorr;
    
    //std::cout << cdelt1 << '\t' << cdelt2 << '\t' << cdelt3 << '\t' << bmaj << '\t' << bmin << std::endl;
    
    // WARNING: There is no proper treatment of different units here, and all parameters are implicitly 
    // WARNING: assumed to have matching default units. This will need to be improved at some point.
    // WARNING: Also, there is no calculation of uncertainties yet!
    
    return 0;
}



// Create integrated spectrum:

int Parametrization::createIntegratedSpectrum()
{
    if(data.empty() == true)
    {
        std::cerr << "Error (Parametrization): No data loaded." << std::endl;
        return 1;
    }
    
    spectrum.clear();
    
    for(long i = 0; i <= subRegionZ2 - subRegionZ1; i++)
    {
        spectrum.push_back(0.0);
    }
    
    for(size_t i = 0; i < data.size(); i++)
    {
        spectrum[data[i].z - subRegionZ1] += static_cast<double>(data[i].value);
    }
    
    //for(size_t i = 0; i <= subRegionZ2 - subRegionZ1; i++)
    //{
    //    std::cout << i << '\t' << spectrum[i] << std::endl;
    //}
    
    return 0;
}



// Fit Busy Function:

int Parametrization::fitBusyFunction()
{
    if(data.empty() or spectrum.empty())
    {
        std::cerr << "Error (Parametrization): No data loaded." << std::endl;
        return 1;
    }
    
    std::vector<double> uncertainties(subRegionZ2 - subRegionZ1 + 1, 0.1);
    
    BusyFit busyFit;
    busyFit.setup(spectrum.size(), &spectrum[0], &uncertainties[0], 2, true, false);
    
    busyFitSuccess = busyFit.fit();
    
    busyFit.getResult(&busyFitParameters[0], &busyFitUncertainties[0], busyFunctionChi2);
    busyFit.getParameters(busyFunctionCentroid, busyFunctionW50, busyFunctionW20, busyFunctionFpeak, busyFunctionFint);
    
    return 0;
}



// Assign results to source

int Parametrization::writeParameters()
{
    /*Measurement<double> measurementFlux = dataCube->getUnitFlux();
    Measurement<double> measurementSpec = dataCube->getUnitSpec();
    
    // WARNING: This will later need to be set manually in DataCube when reading data from pointer, using header information!
    measurementFlux.set("Flux", 1.0, 0.0, UNIT_JY);
    measurementSpec.set("Velocity", 1.0, 0.0, "km/s");
    
    Unit unitFlux    = measurementFlux.getUnit();
    Unit unitSpec    = measurementSpec.getUnit();
    Unit unitIntFlux = unitFlux * unitSpec;
    Unit unitBfB     = unitSpec;
    unitBfB.invert();
    Unit unitBfC     = unitSpec * unitSpec;
    unitBfC.invert();*/
    
    source->setParameter("ID",         source->getSourceID());
    
    source->setParameter("X",          centroidX);
    source->setParameter("Y",          centroidY);
    source->setParameter("Z",          centroidZ);
    
    source->setParameter("F_PEAK",     peakFlux);
    source->setParameter("F_INT",      integratedFlux);
    source->setParameter("F_TOT",      totalFlux);
    
    if(doBusyFunction == true)
    {
        source->setParameter("BF_FLAG",    busyFitSuccess);
        source->setParameter("BF_CHI2",    busyFunctionChi2);
        source->setParameter("BF_A",       busyFitParameters[0]);
        source->setParameter("BF_B1",      busyFitParameters[1]);
        source->setParameter("BF_B2",      busyFitParameters[2]);
        source->setParameter("BF_C",       busyFitParameters[3]);
        source->setParameter("BF_XE0",     busyFitParameters[4]);
        source->setParameter("BF_XP0",     busyFitParameters[5]);
        source->setParameter("BF_W",       busyFitParameters[6]);
        source->setParameter("BF_Z",       busyFunctionCentroid);
        source->setParameter("BF_W20",     busyFunctionW20);
        source->setParameter("BF_W50",     busyFunctionW50);
        source->setParameter("BF_F_PEAK",  busyFunctionFpeak);
        source->setParameter("BF_F_INT",   busyFunctionFint);
    }
    
    return 0;
}
