#ifndef MASKOPTIMIZATION_H
#define MASKOPTIMIZATION_H

#include "DataCube.h"
#include "Source.h"


#define MASKOPTIMIZATION_DEFAULT_SPATIAL_RADIUS  30L
#define MASKOPTIMIZATION_DEFAULT_SPECTRAL_RADIUS 30L
#define MASKOPTIMIZATION_ITERATIONS              40
#define MASKOPTIMIZATION_INITIAL_ELLIPSE_SIZE    2.0
#define MASKOPTIMIZATION_ELLIPSE_GROWTH          1.0


class MaskOptimization
{
public:
    MaskOptimization(long dx = 0L, long dy = 0L, long dz = 0L);
    
    int optimize(DataCube<float> *d, DataCube<short> *m, Source *s);
    
private:
    DataCube<float> *dataCube;
    DataCube<short> *maskCube;
    Source          *source;
    DataCube<float>  momentMap;
    
    double theta;
    double ellA;
    double ellB;
    
    long searchRadiusX;
    long searchRadiusY;
    long searchRadiusZ;
    
    long subRegionX1;
    long subRegionX2;
    long subRegionY1;
    long subRegionY2;
    long subRegionZ1;
    long subRegionZ2;
    
    int loadData(DataCube<float> *d, DataCube<short> *m, Source *s);
    int createMomentMap();
    int fitEllipse();
    int growEllipse();
};

#endif
