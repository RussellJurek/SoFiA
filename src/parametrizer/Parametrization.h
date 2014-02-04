#ifndef PARAMETRIZATION_H
#define PARAMETRIZATION_H

#define PARAMETRIZATION_DEFAULT_SPATIAL_RADIUS  50L
#define PARAMETRIZATION_DEFAULT_SPECTRAL_RADIUS 50L

#include <vector>

#include "BusyFit.h"
#include "DataCube.h"
#include "Source.h"

class Parametrization
{
public:
    Parametrization();
    
    int parametrize(DataCube<float> *d, DataCube<short> *m, Source *s, bool doBF = true);
    
private:
    int loadData(DataCube<float> *d, DataCube<short> *m, Source *s);
    //int createMomentZeroMap();
    int measureCentroid();
    int measureFlux();
    int createIntegratedSpectrum();
    int fitBusyFunction();
    int writeParameters();
    
    bool doBusyFunction;
    
    DataCube<float> *dataCube;
    DataCube<short> *maskCube;
    Source          *source;
    
    long searchRadiusX;
    long searchRadiusY;
    long searchRadiusZ;
    
    long subRegionX1;
    long subRegionX2;
    long subRegionY1;
    long subRegionY2;
    long subRegionZ1;
    long subRegionZ2;
    
    struct DataPoint
    {
        long x;
        long y;
        long z;
        float value;
    };
    
    std::vector<struct DataPoint> data;
    
    std::vector<double> spectrum;
    
    double centroidX;
    double centroidY;
    double centroidZ;
    double lineWidthW20;
    double lineWidthW50;
    double peakFlux;
    double integratedFlux;
    double totalFlux;
    int    busyFitSuccess;
    double busyFunctionChi2;
    double busyFitParameters[BUSYFIT_FREE_PARAM];
    double busyFitUncertainties[BUSYFIT_FREE_PARAM];
    double busyFunctionCentroid;
    double busyFunctionW20;
    double busyFunctionW50;
    double busyFunctionFpeak;
    double busyFunctionFint;
};

#endif
