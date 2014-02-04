// Compilation: g++ -c WorldCoordinateSystem.cpp

#include <iostream>
#include <vector>

#include <fitsio.h>
#include <wcslib/wcs.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsfix.h>

#include "WorldCoordinateSystem.h"



// -------------------------- //
// Constructor and Destructor //
// -------------------------- //

WorldCoordinateSystem::WorldCoordinateSystem()
{
    wcsParameters = 0;
    numberWcsRep  = 0;
    
    validWcsDefined = false;
    
    return;
}

WorldCoordinateSystem::~WorldCoordinateSystem()
{
    if(wcsParameters != 0)
    {
        wcsvfree(&numberWcsRep, &wcsParameters);
        wcsfree(wcsParameters);
        free(wcsParameters);
    }
    
    return;
}



// ----------------------------------------------- //
// Function to return whether valid WCS is defined //
// ----------------------------------------------- //

bool WorldCoordinateSystem::isDefined()
{
    if(validWcsDefined == true and wcsParameters != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}



// ---------------------------------------- //
// Function to clear current WCS definition //
// ---------------------------------------- //

int WorldCoordinateSystem::clear()
{
    validWcsDefined = false;
    
    if(wcsParameters != 0)                       // Delete any previous WCS definition
    {
        wcsvfree(&numberWcsRep, &wcsParameters);
        wcsfree(wcsParameters);
        free(wcsParameters);
    }
    
    wcsParameters = 0;                           // Set data pointers to null again
    numberWcsRep  = 0;
    
    return 0;
}



// --------------------------------------------------- //
// Function to convert from pixel to world coordinates //
// --------------------------------------------------- //

int WorldCoordinateSystem::convertToWorld(std::vector<double> &coordPixel, std::vector<double> &coordWorld)
{
    unsigned int numberAxes = wcsParameters->naxis;
    
    if(validWcsDefined == false or wcsParameters == 0)
    {
        std::cerr << "Error (WorldCoordinateSystem): Cannot convert coordinates, no WCS defined." << std::endl;
        return 1;
    }
    
    if(coordPixel.size() != coordWorld.size())
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate vector sizes inconsistent (" << coordPixel.size() << " vs " << coordWorld.size() << ")." << std::endl;
        return 1;
    }
    
    if(coordWorld.size() != numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate vector size (" << coordWorld.size() << ") inconsistent with cube dimension (" << numberAxes << ")." << std::endl;
        return 1;
    }
    
    double *coordPixel2 = new double[numberAxes];
    double *tmpWorld    = new double[numberAxes];
    double  phi;
    double  theta;
    double *coordWorld2 = new double[numberAxes];
    int     stat;
    
    for(unsigned int i = 0; i < numberAxes; i++)
    {
        coordPixel2[i] = coordPixel[i] + 1.0;    // Caution: WCS pixel arrays are 1-based!!!
    }
    
    int statusWcs = 0;
    
    statusWcs = wcsp2s(wcsParameters, 1, numberAxes, coordPixel2, tmpWorld, &phi, &theta, coordWorld2, &stat);
    
    if(statusWcs)
    {
        delete[] coordPixel2;
        delete[] tmpWorld;
        delete[] coordWorld2;
        
        std::cerr << "Error (WorldCoordinateSystem): wcslib error " << statusWcs << ": " << wcs_errmsg[statusWcs] << std::endl;
        
        return 1;
    }
    
    for(unsigned int i = 0; i < numberAxes; i++)
    {
        coordWorld[i] = coordWorld2[i];
    }
    
    delete[] coordPixel2;
    delete[] tmpWorld;
    delete[] coordWorld2;
    
    return 0;
}



// --------------------------------------------------- //
// Function to convert from world to pixel coordinates //
// --------------------------------------------------- //

int WorldCoordinateSystem::convertToPixel(std::vector<double> &coordPixel, std::vector<double> &coordWorld)
{
    unsigned int numberAxes = wcsParameters->naxis;
    
    if(validWcsDefined == false or wcsParameters == 0)
    {
        std::cerr << "Error (WorldCoordinateSystem): Cannot convert coordinates, no WCS defined." << std::endl;
        return 1;
    }
    
    if(coordPixel.size() != coordWorld.size())
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate vector sizes inconsistent (" << coordPixel.size() << " vs " << coordWorld.size() << ")." << std::endl;
        return 1;
    }
    
    if(coordPixel.size() != numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate vector size (" << coordPixel.size() << ") inconsistent with cube dimension (" << numberAxes << ")." << std::endl;
        return 1;
    }
    
    double *coordPixel2 = new double[numberAxes];
    double *tmpWorld    = new double[numberAxes];
    double  phi;
    double  theta;
    double *coordWorld2 = new double[numberAxes];
    int     stat;
    
    for(unsigned int i = 0; i < numberAxes; i++)
    {
        coordWorld2[i] = coordWorld[i];
    }
    
    int statusWcs = 0;
    
    statusWcs = wcss2p(wcsParameters, 1, numberAxes, coordWorld2, &phi, &theta, tmpWorld, coordPixel2, &stat);
    if(statusWcs)
    {
        delete[] coordPixel2;
        delete[] tmpWorld;
        delete[] coordWorld2;
        
        std::cerr << "Error (WorldCoordinateSystem): wcslib error " << statusWcs << ": " << wcs_errmsg[statusWcs] << std::endl;
        
        return 1;
    }
    
    for(unsigned int i = 0; i < numberAxes; i++)
    {
        coordPixel[i] = coordPixel2[i] - 1.0;    // Caution: WCS pixel arrays are 1-based!!!
    }
    
    delete[] coordPixel2;
    delete[] tmpWorld;
    delete[] coordWorld2;
    
    return 0;
}



// ---------------------------------------------------------------------- //
// Function to convert pixel to spatial coordinates (uses convertToWorld) //
// ---------------------------------------------------------------------- //

int WorldCoordinateSystem::convertPixelToSky(double x, double y, double z, double &longitude, double &latitude, double &spectral)
{
    if(validWcsDefined == false or wcsParameters == 0)
    {
        std::cerr << "Error (WorldCoordinateSystem): Cannot convert coordinates, no WCS defined." << std::endl;
        return 1;
    }
    
    int numberAxes    = wcsParameters->naxis;
    int axisLongitude = wcsParameters->lng;
    int axisLatitude  = wcsParameters->lat;
    int axisSpectral  = wcsParameters->spec;
    
    if(axisLongitude < 0 or axisLongitude >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No longitude axis found." << std::endl;
        return 1;
    }
    
    if(axisLatitude < 0 or axisLatitude >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No latitude axis found." << std::endl;
        return 1;
    }
    
    if(axisSpectral < 0 or axisSpectral >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No spectral axis found." << std::endl;
        return 1;
    }
    
    std::vector<double> coordPixel;
    std::vector<double> coordWorld;
    
    for(int i = 0; i < numberAxes; i++)
    {
        if(i == axisLongitude)     coordPixel.push_back(x);
        else if(i == axisLatitude) coordPixel.push_back(y);
        else if(i == axisSpectral) coordPixel.push_back(z);
        else                       coordPixel.push_back(0.0);
        
        coordWorld.push_back(0.0);
    }
    
    if(convertToWorld(coordPixel, coordWorld))
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate conversion failed." << std::endl;
        return 1;
    }
    
    longitude = coordWorld[axisLongitude];
    latitude  = coordWorld[axisLatitude];
    spectral  = coordWorld[axisSpectral];
    
    return 0;
}



// ---------------------------------------------------------------------- //
// Function to convert spatial to pixel coordinates (uses convertToPixel) //
// ---------------------------------------------------------------------- //

int WorldCoordinateSystem::convertSkyToPixel(double &x, double &y, double &z, double longitude, double latitude, double spectral)
{
    if(validWcsDefined == false or wcsParameters == 0)
    {
        std::cerr << "Error (WorldCoordinateSystem): Cannot convert coordinates, no WCS defined." << std::endl;
        return 1;
    }
    
    int numberAxes    = wcsParameters->naxis;
    int axisLongitude = wcsParameters->lng;
    int axisLatitude  = wcsParameters->lat;
    int axisSpectral  = wcsParameters->spec;
    
    if(axisLongitude < 0 or axisLongitude >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No longitude axis found." << std::endl;
        return 1;
    }
    
    if(axisLatitude < 0 or axisLatitude >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No latitude axis found." << std::endl;
        return 1;
    }
    
    if(axisSpectral < 0 or axisSpectral >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No spectral axis found." << std::endl;
        return 1;
    }
    
    std::vector<double> coordPixel;
    std::vector<double> coordWorld;
    
    for(int i = 0; i < numberAxes; i++)
    {
        if(i == axisLongitude)     coordWorld.push_back(longitude);
        else if(i == axisLatitude) coordWorld.push_back(latitude);
        else if(i == axisSpectral) coordWorld.push_back(spectral);
        else                       coordWorld.push_back(0.0);
        
        coordPixel.push_back(0.0);
    }
    
    if(convertToPixel(coordPixel, coordWorld))
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate conversion failed." << std::endl;
        return 1;
    }
    
    x = coordPixel[axisLongitude];
    y = coordPixel[axisLatitude];
    z = coordPixel[axisSpectral];
    
    return 0;
}



// ------------------------------------------------------------- //
// Function to convert channel to spectral (uses convertToPixel) //
// ------------------------------------------------------------- //

int WorldCoordinateSystem::convertChannelToSpectral(double z, double &spectral)
{
    if(validWcsDefined == false or wcsParameters == 0)
    {
        std::cerr << "Error (WorldCoordinateSystem): Cannot convert coordinates, no WCS defined." << std::endl;
        return 1;
    }
    
    int numberAxes   = wcsParameters->naxis;
    int axisSpectral = wcsParameters->spec;
    
    if(axisSpectral < 0 or axisSpectral >= numberAxes)
    {
        std::cerr << "Error (WorldCoordinateSystem): No spectral axis found." << std::endl;
        return 1;
    }
    
    std::vector<double> coordPixel;
    std::vector<double> coordWorld;
    
    for(int i = 0; i < numberAxes; i++)
    {
        if(i == axisSpectral) coordPixel.push_back(z);
        else                  coordPixel.push_back(0.0);
        
        coordWorld.push_back(0.0);
    }
    
    if(convertToWorld(coordPixel, coordWorld))
    {
        std::cerr << "Error (WorldCoordinateSystem): Coordinate conversion failed." << std::endl;
        return 1;
    }
    
    spectral = coordWorld[axisSpectral];
    
    return 0;
}



// ---------------------------------------------------------- //
// Overloaded function to read WCS information from FITS file //
// ---------------------------------------------------------- //

int readFitsFile(const std::string &fileName)
{
    if(fileName.empty()) return 1;
    else                 return readFitsFile(fileName.c_str());
}

int WorldCoordinateSystem::readFitsFile(const char *fileName)
{
    this->clear();                               // Delete any previous WCS definition
    
    if(fileName == 0)       return 1;
    if(fileName[0] == '\0') return 1;
    
    int statusFits = 0;                          // CFITSIO status variable, MUST be set to 0!
    fitsfile *filePointer = 0;                   // CFITSIO file pointer
    char *header = 0;
    int numberAxes = 0;
    long *dimAxes = 0;
    
    int statusWcs  = 0;                          // Some variables needed by wcslib
    int numberKeys = 0;
    int numberRejected = 0;
    
    // Open FITS file:
    if(fits_open_file(&filePointer, fileName, READONLY, &statusFits)) goto Error;
    
    // Get number of dimensions:
    if(fits_get_img_dim(filePointer, &numberAxes, &statusFits))
    {
        fits_close_file(filePointer, &statusFits);
        goto Error;
    }
    
    // Get the size of each dimension:
    dimAxes = new long[numberAxes];
    for(int i = 0; i < numberAxes; i++) dimAxes[i] = 1L;
    if(fits_get_img_size(filePointer, numberAxes, dimAxes, &statusFits))
    {
        fits_close_file(filePointer, &statusFits);
        goto Error;
    }
    
    // Read header into memory and point *header to it:
    if(fits_hdr2str(filePointer, 1, NULL, 0, &header, &numberKeys, &statusFits))
    {
        fits_close_file(filePointer, &statusFits);
        goto Error;
    }
    
    // Close FITS file:
    fits_close_file(filePointer, &statusFits);
    
    // Allocate memory for wcsprm struct:
    wcsParameters = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
    wcsParameters->flag = -1;
    
    // Initialise wcsprm struct:
    statusWcs = wcsini(true, numberAxes, wcsParameters);
    if(statusWcs) goto Error;
    
    // Parse the FITS header to fill in the wcsprm struct:
    statusWcs = wcspih(header, numberKeys, 1, 2, &numberRejected, &numberWcsRep, &wcsParameters);
    if(statusWcs) goto Error;
    
    // Apply all necessary corrections to wcsprm struct
    // (missing cards, non-standard units or spectral types, etc.):
    int stat[NWCSFIX];
    statusWcs = wcsfix(1, (const int*)dimAxes, wcsParameters, stat);
    if(statusWcs) goto Error;
    
    // Set up additional parameters in wcsprm struct derived imported data:
    statusWcs = wcsset(wcsParameters);
    if(statusWcs) goto Error;
    
    // Redo the corrections to account for things like NCP projections:
    statusWcs = wcsfix(1, (const int*)dimAxes, wcsParameters, stat);
    if(statusWcs) goto Error;
    
    // Clean up and exit upon success:
    if(header != 0) free(header);
    if(dimAxes != 0) delete[] dimAxes;
    
    validWcsDefined = true;
    
    return 0;
    
    // Exit point on error:
    
    // I've got no choice but to do this because fitsio and wcslib are written in C,
    // and this is the easiest way to avoid a total mess in the code.
    
    Error:
    
    if(statusWcs != 0)  std::cerr << "Error (WorldCoordinateSystem): wcslib error " << statusWcs << ": " << wcs_errmsg[statusWcs] << std::endl;
    
    if(statusFits != 0) fits_report_error(stderr, statusFits);
    
    if(wcsParameters != 0)
    {
        // Warning: There are some serious bugs in wcslib that lead to a total crash when 
        // wcsvfree() or wcsfree() is called and wcspih() failed and messed some pointers 
        // up before. Apparently, wcsvfree() and wcsfree() will attempt to free illegal 
        // memory blocks, which leads to a complete crash of all software using this class. 
        // There's nothing I can do about this other than not calling wcsvfree() and 
        // wcsfree() at all, but this would create a memory leak which is not ideal either.
        wcsvfree(&numberWcsRep, &wcsParameters);
        wcsfree(wcsParameters);
        free(wcsParameters);
    }
    
    if(header != 0) free(header);
    
    if(dimAxes != 0) delete[] dimAxes;
    
    return 1;
}



// ------------------------------------------------------------------- //
// Overloaded function to read WCS information from FITS header string //
// ------------------------------------------------------------------- //

int WorldCoordinateSystem::readFitsHeader(char *headerString, int numberAxes, long *dimAxes)
{
    this->clear();                               // Delete any previous WCS definition
    
    if(headerString == 0)       return 1;
    
    int statusWcs  = 0;                          // Some variables needed by wcslib
    int numberKeys = 0;
    int numberRejected = 0;
    
    // Allocate memory for wcsprm struct:
    wcsParameters = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
    wcsParameters->flag = -1;
    
    // Initialise wcsprm struct:
    statusWcs = wcsini(true, numberAxes, wcsParameters);
    if(statusWcs) goto Error;
    
    // Parse the FITS header to fill in the wcsprm struct:
    statusWcs = wcspih(headerString, numberKeys, 1, 2, &numberRejected, &numberWcsRep, &wcsParameters);
    if(statusWcs) goto Error;
    
    if(dimAxes != 0)
    {
        // Apply all necessary corrections to wcsprm struct
        // (missing cards, non-standard units or spectral types, etc.):
        int stat[NWCSFIX];
        statusWcs = wcsfix(1, (const int*)dimAxes, wcsParameters, stat);
        if(statusWcs) goto Error;
        
        // Set up additional parameters in wcsprm struct derived imported data:
        statusWcs = wcsset(wcsParameters);
        if(statusWcs) goto Error;
        
        // Redo the corrections to account for things like NCP projections:
        statusWcs = wcsfix(1, (const int*)dimAxes, wcsParameters, stat);
        if(statusWcs) goto Error;
    }
    
    validWcsDefined = true;
    
    return 0;
    
    // Exit point on error:
    
    // I've got no choice but to do this because fitsio and wcslib are written in C,
    // and this is the easiest way to avoid a total mess in the code.
    
    Error:
    
    if(statusWcs != 0)  std::cerr << "Error (WorldCoordinateSystem): wcslib error " << statusWcs << ": " << wcs_errmsg[statusWcs] << std::endl;
    
    if(wcsParameters != 0)
    {
        // Warning: There are some serious bugs in wcslib that lead to a total crash when 
        // wcsvfree() or wcsfree() is called and wcspih() failed and messed some pointers 
        // up before. Apparently, wcsvfree() and wcsfree() will attempt to free illegal 
        // memory blocks, which leads to a complete crash of all software using this class. 
        // There's nothing I can do about this other than not calling wcsvfree() and 
        // wcsfree() at all, but this would create a memory leak which is not ideal either.
        wcsvfree(&numberWcsRep, &wcsParameters);
        wcsfree(wcsParameters);
        free(wcsParameters);
    }
    
    return 1;
}
