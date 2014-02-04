#ifndef DATACUBE_H
#define DATACUBE_H

#include <string>
#include <map>

//#include "WorldCoordinateSystem.h"
#include "MetaData.h"
#include "Measurement.h"

template <typename T> class DataCube
{
public:
    DataCube();
    ~DataCube();
    
    bool   isDefined();
    int    clear();
    
    int    createNewCube(long dx, long dy, long dz);
    int    createNewCubeFromPointer(long dx, long dy, long dz, T *d);
    
    //int    readFitsFile(const char *fileName);
    //int    readFitsFile(const std::string &fileName);
    
    //int    writeFitsFile(const std::string &fileName, const std::string &fileNameHeader);
    
    int    setHeader(const std::string &keyName, const std::string &value);
    int    setHeader(const std::string &keyName, int value);
    int    setHeader(const std::string &keyName, long value);
    int    setHeader(const std::string &keyName, float value);
    int    setHeader(const std::string &keyName, double value);
    int    setHeader(const char *keyName, const char *value);
    int    setHeader(const char *keyName, int value);
    int    setHeader(const char *keyName, long value);
    int    setHeader(const char *keyName, float value);
    int    setHeader(const char *keyName, double value);
    
    int    getHeader(const std::string &keyName, std::string &result);
    int    getHeader(const std::string &keyName, int &result);
    int    getHeader(const std::string &keyName, long &result);
    int    getHeader(const std::string &keyName, float &result);
    int    getHeader(const std::string &keyName, double &result);
    int    getHeader(const char *keyName, std::string &result);
    int    getHeader(const char *keyName, int &result);
    int    getHeader(const char *keyName, long &result);
    int    getHeader(const char *keyName, float &result);
    int    getHeader(const char *keyName, double &result);
    
    int    setData(T value, long x, long y, long z);
    int    addData(T value, long x, long y, long z);
    int    multiplyData(DataCube &factor);
    
    T      getData(long x, long y = 0L, long z = 0L);
    int    getDataR(T &value, long x, long y = 0L, long z = 0L);
    
    int    floodFill(T value = static_cast<T>(0));
    
    long   getSize(int dimension);
    
    Measurement<double> getUnitFlux();
    Measurement<double> getUnitSpec();
    
    T      getDataMin();
    T      getDataMax();
    
    T     *dataPointer();
    
    //bool   wcsIsDefined();
    //int    readWcsFromHeader(const std::string &headerString);
    //int    wcsPixelToWorld(double x, double y, double z, double &longitude, double &latitude, double &spectral);
    //int    wcsWorldToPixel(double &x, double &y, double &z, double longitude, double latitude, double spectral);
    //int    wcsChannelToSpectral(double z, double &spectral);
    
private:
    long   numberPixels[4];
    
    MetaData header;
    
    T     *data;
    T      dataMin;
    T      dataMax;
    
    bool   flagMemory;
    
    Measurement<double> unitFlux;
    Measurement<double> unitLon;
    Measurement<double> unitLat;
    Measurement<double> unitSpec;
    
    //WorldCoordinateSystem wcs;
    
    long   position(long x, long y, long z);
    
    // Make copy constructor and assignment operator private, so they can't be called:
    // (Maybe I should rather write sensible ones at some point...)
    DataCube(const DataCube&);
    DataCube &operator=(const DataCube&);
};

#endif
