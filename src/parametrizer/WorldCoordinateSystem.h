#ifndef WORLDCOORDINATESYSTEM_H
#define WORLDCOORDINATESYSTEM_H

#include <vector>
#include <string>

class WorldCoordinateSystem
{
public:
    WorldCoordinateSystem();
    ~WorldCoordinateSystem();
    
    bool isDefined();
    int  clear();
    
    int  readFitsFile(const char *fileName);
    int  readFitsFile(const std::string &fileName);
    int  readFitsHeader(char *headerString, int numberAxes, long *dimAxes = 0);
    
    int  convertToWorld(std::vector<double> &coordPixel, std::vector<double> &coordWorld);
    int  convertToPixel(std::vector<double> &coordPixel, std::vector<double> &coordWorld);
    
    int  convertPixelToSky(double x, double y, double z, double &longitude, double &latitude, double &spectral);
    int  convertSkyToPixel(double &x, double &y, double &z, double longitude, double latitude, double spectral);
    int  convertChannelToSpectral(double z, double &spectral);
    
private:
    bool validWcsDefined;
    struct wcsprm *wcsParameters;
    int  numberWcsRep;
};

#endif
