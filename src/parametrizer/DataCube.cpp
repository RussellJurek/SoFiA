// Compilation: g++ -c DataCube.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <limits>
#include <typeinfo>

//#include <fitsio.h>
//#include <wcslib/wcshdr.h>

#include "helperFunctions.h"
#include "DataCube.h"



// -------------------------- //
// Constructor and Destructor //
// -------------------------- //

template <typename T> DataCube<T>::DataCube()
{
    numberPixels[0] = 1L;
    numberPixels[1] = 1L;
    numberPixels[2] = 1L;
    numberPixels[3] = 1L;
    
    data = 0;
    
    flagMemory = false;
    
    return;
}

template <typename T> DataCube<T>::~DataCube()
{
    this->clear();
    
    return;
}



// ---------------------------------- //
// Function to return pointer to data //
// ---------------------------------- //

template <typename T> T *DataCube<T>::dataPointer()
{
    return data;
}



// ---------------------------------------------------- //
// Function to return whether valid data set is defined //
// ---------------------------------------------------- //

template <typename T> bool DataCube<T>::isDefined()
{
    if(data != 0) return true;
    else return false;
}



// ------------------------------- //
// Function to create new data set //
// ------------------------------- //

template <typename T> int DataCube<T>::createNewCube(long dx, long dy, long dz)
{
    this->clear();
    
    if(typeid(T) != typeid(char) and typeid(T) != typeid(short) and typeid(T) != typeid(long) and typeid(T) != typeid(float) and typeid(T) != typeid(double))
    {
        std::cerr << "Error (DataCube): Unsupported data type of new cube;" << std::endl;
        std::cerr << "                  must be char, short, long, float, or double." << std::endl;
        return 1;
    }
    
    if(dx < 1L or dy < 1L or dz < 1L)
    {
        std::cerr << "Error (DataCube): Invalid array dimensions of new cube." << std::endl;
        return 1;
    }
    
    data = new T[dx * dy * dz];
    
    if(data == 0)
    {
        std::cerr << "Error (DataCube): Not enough memory to create data cube." << std::endl;
        this->clear();
        return 1;
    }
    
    flagMemory = true;
    
    //std::cout << "DataCube: Created new cube with dimensions " << dx << " x " << dy << " x " << dz << "." << std::endl;
    
    numberPixels[0] = dx;
    numberPixels[1] = dy;
    numberPixels[2] = dz;
    numberPixels[3] = 1L;
    
    this->floodFill(static_cast<T>(0));
    
    if(typeid(T)      == typeid(float))  this->setHeader("BITPIX", "-32");
    else if(typeid(T) == typeid(short))  this->setHeader("BITPIX",  "16");
    else if(typeid(T) == typeid(long))   this->setHeader("BITPIX",  "32");
    else if(typeid(T) == typeid(double)) this->setHeader("BITPIX", "-64");
    else if(typeid(T) == typeid(char))   this->setHeader("BITPIX",   "8");
    
    this->setHeader("NAXIS", "3");
    this->setHeader("NAXIS1", dx);
    this->setHeader("NAXIS2", dy);
    this->setHeader("NAXIS3", dz);
    
    this->setHeader("CTYPE1", "PIXEL");
    this->setHeader("CTYPE2", "PIXEL");
    this->setHeader("CTYPE3", "PIXEL");
    
    this->setHeader("CRPIX1", 0.0);
    this->setHeader("CRPIX2", 0.0);
    this->setHeader("CRPIX3", 0.0);
    
    this->setHeader("CRVAL1", 0.0);
    this->setHeader("CRVAL2", 0.0);
    this->setHeader("CRVAL3", 0.0);
    
    this->setHeader("CDELT1", 1.0);
    this->setHeader("CDELT2", 1.0);
    this->setHeader("CDELT3", 1.0);
    
    return 0;
}



// -------------------------------------------- //
// Function to create new data set from pointer //
// -------------------------------------------- //

template <typename T> int DataCube<T>::createNewCubeFromPointer(long dx, long dy, long dz, T *d)
{
    this->clear();
    
    if(typeid(T) != typeid(char) and typeid(T) != typeid(short) and typeid(T) != typeid(long) and typeid(T) != typeid(float) and typeid(T) != typeid(double))
    {
        std::cerr << "Error (DataCube): Unsupported data type of new cube;" << std::endl;
        std::cerr << "                  must be char, short, long, float, or double." << std::endl;
        return 1;
    }
    
    if(dx < 1L or dy < 1L or dz < 1L)
    {
        std::cerr << "Error (DataCube): Invalid array dimensions of new cube." << std::endl;
        return 1;
    }
    
    if(d == 0)
    {
        std::cerr << "Error (DataCube): Null pointer provided." << std::endl;
        return 1;
    }
    
    data = d;
    
    numberPixels[0] = dx;
    numberPixels[1] = dy;
    numberPixels[2] = dz;
    numberPixels[3] = 1L;
    
    if(typeid(T)      == typeid(float))  this->setHeader("BITPIX", "-32");
    else if(typeid(T) == typeid(short))  this->setHeader("BITPIX",  "16");
    else if(typeid(T) == typeid(long))   this->setHeader("BITPIX",  "32");
    else if(typeid(T) == typeid(double)) this->setHeader("BITPIX", "-64");
    else if(typeid(T) == typeid(char))   this->setHeader("BITPIX",   "8");
    
    this->setHeader("NAXIS", "3");
    this->setHeader("NAXIS1", dx);
    this->setHeader("NAXIS2", dy);
    this->setHeader("NAXIS3", dz);
    
    this->setHeader("CTYPE1", "PIXEL");
    this->setHeader("CTYPE2", "PIXEL");
    this->setHeader("CTYPE3", "PIXEL");
    
    this->setHeader("CRPIX1", 0.0);
    this->setHeader("CRPIX2", 0.0);
    this->setHeader("CRPIX3", 0.0);
    
    this->setHeader("CRVAL1", 0.0);
    this->setHeader("CRVAL2", 0.0);
    this->setHeader("CRVAL3", 0.0);
    
    this->setHeader("CDELT1", 1.0);
    this->setHeader("CDELT2", 1.0);
    this->setHeader("CDELT3", 1.0);
    
    return 0;
}



// ---------------------------------- //
// Function to clear current data set //
// ---------------------------------- //

template <typename T> int DataCube<T>::clear()
{
    if(data != 0)
    {
        if(flagMemory == true)
        {
            delete[] data;                       // Erase previously defined data set
            flagMemory = false;
        }
        data = 0;                                // and set pointer *data to null
    }
    
    if(header.isDefined()) header.clear();       // Clear all previously defined header entries
    
    numberPixels[0] = 1L;
    numberPixels[1] = 1L;
    numberPixels[2] = 1L;
    numberPixels[3] = 1L;
    
    //wcs.clear();                                 // Explicitly clear WCS information as well
    
    return 0;
}



// ------------------------------------------ //
// Overloaded functions to write to data cube //
// ------------------------------------------ //

template <typename T> int DataCube<T>::setData(T value, long x, long y, long z)
{
    if(x < 0L or x >= numberPixels[0] or y < 0L or y >= numberPixels[1] or z < 0L or z >= numberPixels[2] or data == 0)
    {
        return 1;
    }
    else
    {
        data[position(x, y, z)] = value;
        return 0;
    }
}

template <typename T> int DataCube<T>::addData(T value, long x, long y, long z)
{
    if(x < 0L or x >= numberPixels[0] or y < 0L or y >= numberPixels[1] or z < 0L or z >= numberPixels[2] or data == 0)
    {
        return 1;
    }
    else
    {
        data[position(x, y, z)] += value;
        return 0;
    }
}



// ------------------------------------------- //
// Overloaded functions to read from data cube //
// ------------------------------------------- //

template <typename T> T DataCube<T>::getData(long x, long y, long z)
{
    if(x < 0L or x >= numberPixels[0] or y < 0L or y >= numberPixels[1] or z < 0L or z >= numberPixels[2] or data == 0)
    {
        if(std::numeric_limits<T>::has_quiet_NaN == true)
        {
            // Return NaN if available:
            return std::numeric_limits<T>::quiet_NaN();
        }
        else
        {
            // Otherwise return 0:
            return static_cast<T>(0);
        }
    }
    else
    {
        return data[position(x, y, z)];
    }
}

template <typename T> int DataCube<T>::getDataR(T &value, long x, long y, long z)
{
    if(x < 0L or x >= numberPixels[0] or y < 0L or y >= numberPixels[1] or z < 0L or z >= numberPixels[2] or data == 0)
    {
        return 1;
    }
    else
    {
        value = data[position(x, y, z)];
        return 0;
    }
}



// --------------------------------- //
// Function to flood-fill data cube: //
// --------------------------------- //

template <typename T> int DataCube<T>::floodFill(T value)
{
    if(isDefined() == false) return 1;
    
    for(long x = 0L; x < numberPixels[0]; x++)
    {
        for(long y = 0L; y < numberPixels[1]; y++)
        {
            for(long z = 0L; z < numberPixels[2]; z++)
            {
                data[position(x, y, z)] = value;
            }
        }
    }
    
    dataMin = value;
    dataMax = value;
    
    return 0;
}



// ------------------------------- //
// Function to multiply data cube: //
// ------------------------------- //

template <typename T> int DataCube<T>::multiplyData(DataCube &factor)
{
    if(this->isDefined() == false or factor.isDefined() == false) return 1;
    
    for(int i = 0; i < 3; i++)
    {
        if(numberPixels[i] != factor.getSize(i))
        {
            std::cerr << "Error (DataCube): Cannot multiply cubes; sizes differ." << std::endl;
            return 1;
        }
    }
    
    T origValue = static_cast<T>(0);
    T newValue  = static_cast<T>(0);
    T mFac      = static_cast<T>(1);
    
    for(long x = 0L; x < numberPixels[0]; x++)
    {
        for(long y = 0L; y < numberPixels[1]; y++)
        {
            for(long z = 0L; z < numberPixels[2]; z++)
            {
                if(factor.getDataR(mFac, x, y, z))
                {
                    std::cerr << "Error (DataCube): Cannot multiply cubes; failed to read from cube." << std::endl;
                    return 1;
                }
                
                origValue = data[position(x, y, z)];
                newValue  = origValue * mFac;
                
                data[position(x, y, z)] = newValue;
                
                if(newValue < dataMin)      dataMin = newValue;
                else if(newValue > dataMax) dataMax = newValue;
            }
        }
    }
    
    return 0;
}


// ------------------------------------------- //
// Function to produce cube size in x, y, or z //
// ------------------------------------------- //

template <typename T> long DataCube<T>::getSize(int dimension)
{
    if(isDefined() == false or dimension < 0 or dimension > 2) return 0L;
    else return numberPixels[dimension];
}


// ------------------------------------------- //
// Function to return min. and max. data value //
// ------------------------------------------- //

template <typename T> T DataCube<T>::getDataMin()
{
    return dataMin;
}

template <typename T> T DataCube<T>::getDataMax()
{
    return dataMax;
}


// ------------------------------------- //
// Functions to return unit measurements //
// ------------------------------------- //

template <typename T> Measurement<double> DataCube<T>::getUnitFlux()
{
    return unitFlux;
}

template <typename T> Measurement<double> DataCube<T>::getUnitSpec()
{
    return unitSpec;
}



// ----------------------------------- //
// Functions to access WCS information //
// ----------------------------------- //

/*template <typename T> bool DataCube<T>::wcsIsDefined()
{
    return wcs.isDefined();
}

template <typename T> int DataCube<T>::readWcsFromHeader(const std::string &headerString)
{
    if(headerString.empty()) return 1;
    
    int n = 0;
    
    if(getHeader("NAXIS", n)) return 1;
    if(n <= 0) return 1;
    
    std::vector<char> headerVector(headerString.size() + 1);
    std::copy(headerString.begin(), headerString.end(), headerVector.begin());
    
    return wcs.readFitsHeader(&headerVector[0], n, &numberPixels[0]);
}

template <typename T> int DataCube<T>::wcsPixelToWorld(double x, double y, double z, double &longitude, double &latitude, double &spectral)
{
    if(wcs.isDefined())
    {
        wcs.convertPixelToSky(x, y, z, longitude, latitude, spectral);
        return 0;
    }
    else
    {
        return 1;
    }
}

template <typename T> int DataCube<T>::wcsWorldToPixel(double &x, double &y, double &z, double longitude, double latitude, double spectral)
{
    if(wcs.isDefined())
    {
        wcs.convertSkyToPixel(x, y, z, longitude, latitude, spectral);
        return 0;
    }
    else
    {
        return 1;
    }
    
}

template <typename T> int DataCube<T>::wcsChannelToSpectral(double z, double &spectral)
{
    if(wcs.isDefined())
    {
        wcs.convertChannelToSpectral(z, spectral);
        return 0;
    }
    else
    {
        return 1;
    }
}



// -------------------------------------- //
// Overloaded functions to read FITS file //
// -------------------------------------- //

template <typename T> int DataCube<T>::readFitsFile(const std::string &fileName)
{
    if(fileName.empty()) return 1;
    else                 return readFitsFile(fileName.c_str());
}

template <typename T> int DataCube<T>::readFitsFile(const char *fileName)
{
    this->clear();                               // Clear current data set
    
    if(fileName == 0)       return 1;
    if(fileName[0] == '\0') return 1;
    
    int       status          = 0;               // CFITSIO status variable MUST be set to 0!
    int       bitpixOrig      = 0;               // Original data type
    int       bitpixEquiv     = 0;               // Data type actually required after BZERO/BSCALE scaling
    int       dimension       = 0;
    int       numberKeys      = 0;
    long      firstPixel[4]   = {1, 1, 1, 1};    // This MUST be set to 1!
    char      headerKey[FLEN_KEYWORD];
    char      headerValue[FLEN_VALUE];
    char      headerComment[FLEN_COMMENT];
    fitsfile *filePointer;                       // CFITSIO file pointer
    
    // Allowed (and supported) FITS axis types:
    
    std::map<std::string, std::string> typeSpec;
    std::map<std::string, std::string>::iterator iterSpec;
    typeSpec.insert(std::pair<std::string, std::string>("FREQ", "Hz"));
    typeSpec.insert(std::pair<std::string, std::string>("ENER", "J"));
    typeSpec.insert(std::pair<std::string, std::string>("WAVN", "1/m"));
    typeSpec.insert(std::pair<std::string, std::string>("VRAD", "m/s"));
    typeSpec.insert(std::pair<std::string, std::string>("WAVE", "m"));
    typeSpec.insert(std::pair<std::string, std::string>("VOPT", "m/s"));
    typeSpec.insert(std::pair<std::string, std::string>("ZOPT", ""));
    typeSpec.insert(std::pair<std::string, std::string>("AWAV", "m"));
    typeSpec.insert(std::pair<std::string, std::string>("VELO", "m/s"));
    typeSpec.insert(std::pair<std::string, std::string>("BETA", ""));
    
    std::map<std::string, unsigned int> typeSpat;
    std::map<std::string, unsigned int>::iterator iterSpat;
    typeSpat.insert(std::pair<std::string, unsigned int>("RA",    UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("DEC",   UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("GLON",  UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("GLAT",  UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("ELON",  UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("ELAT",  UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("LL",    UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("MM",    UNIT_DEG));
    typeSpat.insert(std::pair<std::string, unsigned int>("PIXEL", UNIT_NONE));
    typeSpat.insert(std::pair<std::string, unsigned int>("",      UNIT_NONE));
    
    // Open FITS file and point *filePointer to it:
    
    std::cout << "DataCube: Reading FITS file " << fileName << ":" << std::endl;
    
    if(fits_open_file(&filePointer, fileName, READONLY, &status))
    {
        fits_report_error(stderr, status);
        return 1;
    }
    
    // Determine data types:
    
    fits_get_img_type(filePointer, &bitpixOrig, &status);
    
    if(status != 0)
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    fits_get_img_equivtype(filePointer, &bitpixEquiv, &status);
    
    if(status != 0)
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    // Check whether the required FITS data format is supported:
    
    switch(bitpixEquiv)
    {
        case BYTE_IMG:
            if(typeid(char) != typeid(T))
            {
                std::cerr << "Warning (DataCube): Type mismatch between data and FITS file (8 bit int)." << std::endl;
                std::cerr << "                    Automatic type casting will be performed." << std::endl;
            }
            break;
            
        case SHORT_IMG:
            if(typeid(short) != typeid(T))
            {
                std::cerr << "Warning (DataCube): Type mismatch between data and FITS file (16 bit int)." << std::endl;
                std::cerr << "                    Automatic type casting will be performed." << std::endl;
            }
            break;
            
        case LONG_IMG:
            if(typeid(long) != typeid(T))
            {
                std::cerr << "Warning (DataCube): Type mismatch between data and FITS file (32 bit int)." << std::endl;
                std::cerr << "                    Automatic type casting will be performed." << std::endl;
            }
            break;
            
        case FLOAT_IMG:
            if(typeid(float) != typeid(T))
            {
                std::cerr << "Warning (DataCube): Type mismatch between data and FITS file (32 bit float)." << std::endl;
                std::cerr << "                    Automatic type casting will be performed." << std::endl;
            }
            break;
            
        case DOUBLE_IMG:
            if(typeid(double) != typeid(T))
            {
                std::cerr << "Warning (DataCube): Type mismatch between data and FITS file (64 bit float)." << std::endl;
                std::cerr << "                    Automatic type casting will be performed." << std::endl;
            }
            break;
            
        default:
            std::cerr << "Error (DataCube): Unsupported FITS data type (" << bitpixEquiv << ")." << std::endl;
            fits_close_file(filePointer, &status);
            return 1;
    }
    
    // Determine dimensions of the cube:
    
    if(fits_get_img_dim(filePointer, &dimension, &status))
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    if(dimension < 1 or dimension > 4)
    {
        fits_close_file(filePointer, &status);
        std::cerr << "Error (DataCube): FITS file must have 1, 2, or 3 dimensions." << std::endl;
        return 1;
    }
    
    // Determine size of each dimension:
    
    if(fits_get_img_size(filePointer, 4, numberPixels, &status))
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    if(numberPixels[0] < 1 or numberPixels[1] < 1 or numberPixels[2] < 1 or numberPixels[3] < 1)
    {
        fits_close_file(filePointer, &status);
        std::cerr << "Error (DataCube): Illegal cube dimensions encountered." << std::endl;
        return 1;
    }
    
    if(numberPixels[3] > 1)
    {
        fits_close_file(filePointer, &status);
        std::cerr << "Error (DataCube): FITS file must have 1, 2, or 3 dimensions." << std::endl;
        return 1;
    }
    
    // Read in header entries:
    
    if(fits_get_hdrspace(filePointer, &numberKeys, NULL, &status))
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    for(int i = 0; i < numberKeys; i++)
    {
        if(fits_read_keyn(filePointer, i, headerKey, headerValue, headerComment, &status)) break;
        
        std::string headerKeyString(headerKey);
        std::string headerValueString(headerValue);
        
        stringTrim(headerKeyString);             // Trim key string
        stringToUpper(headerKeyString);          // and convert to upper case, just to be sure
        
        // Ignore all history items:
        if(headerKeyString.compare(0, 7, std::string("HISTORY")) != 0)
        {
            stringTrim(headerValueString);       // Trim value string
            
            // Determine data type and write header entry:
            if(headerValueString.size() > 0)
            {
                if(headerValueString.substr(0, 1) == "\'" and headerValueString.substr(headerValueString.length() - 1, 1) == "\'")
                {
                    // String?
                    headerValueString = headerValueString.substr(1, headerValueString.length() - 2);         // Remove quotation marks
                    stringTrim(headerValueString, TRIM_RIGHT);       // Leading spaces are significant according to FITS standard!
                    
                    header.addEntry(headerKeyString, headerValueString);
                }
                else if(headerValueString == "T")
                {
                    // Boolean 'true'?
                    header.addEntry(headerKeyString, "1", METADATA_BOOL);
                }
                else if(headerValueString == "F")
                {
                    // Boolean 'false'?
                    header.addEntry(headerKeyString, "0", METADATA_BOOL);
                }
                else if(headerValueString.find_first_of(".ED") != std::string::npos)
                {
                    // Floating point number?
                    header.addEntry(headerKeyString, headerValueString, METADATA_DOUBLE);
                }
                else if(headerValueString.find_first_not_of("+-0123456789") == std::string::npos)
                {
                    // Integer number?
                    header.addEntry(headerKeyString, headerValueString, METADATA_LONG);
                }
                else
                {
                    // Defaults to string:
                    header.addEntry(headerKeyString, headerValueString);
                }
            }
        }
    }
    
    //if(status == END_OF_FILE) status = 0;
    
    if(status != 0)
    {
        status = 0;
        fits_close_file(filePointer, &status);
        fits_report_error(stderr, status);
        return 1;
    }
    
    // Checking for specifications of physical units:
    
    std::string keywordString = "";
    std::string unitString    = "";
    
    if(getHeader("BUNIT", unitString) != 0)
    {
        std::cerr << "Warning (DataCube): Keyword BUNIT missing; brightness unit unknown." << std::endl;
        unitFlux.set("brightness", 1.0, 0.0, UNIT_NONE);
    }
    else
    {
        stringReplace(unitString, "/BEAM", "");
        stringReplace(unitString, "/Beam", "");
        stringReplace(unitString, "/beam", "");
        
        if(unitString == "JY" or unitString == "Jy") unitFlux.set("brightness", 1.0, 0.0, UNIT_JY);
        else if(unitString == "mJy")                 unitFlux.set("brightness", 1.0e-3, 0.0, UNIT_JY);
        else if(unitString == "MJy")                 unitFlux.set("brightness", 1.0e+6, 0.0, UNIT_JY);
        else
        {
            if(unitFlux.set("brightness", 1.0, 0.0, unitString) != 0)
            {
                std::cerr << "Warning (DataCube): Unknown brightness unit \'" << unitString <<  "\'." << std::endl;
            }
        }
    }
    
    if(getHeader("CTYPE1", keywordString) != 0) keywordString = "PIXEL";
    
    if(getHeader("CUNIT1", unitString) != 0)
    {
        // No unit information found:
        iterSpat = typeSpat.find(keywordString.substr(0, keywordString.find_first_of('-')));
        
        if(iterSpat != typeSpat.end())
        {
            // Valid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT1 missing; using FITS default." << std::endl;
            unitLon.set("longitude", 1.0, 0.0, iterSpat->second);
        }
        else
        {
            // Invalid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT1 missing and CTYPE1 unknown." << std::endl;
            unitLon.set("longitude", 1.0, 0.0, UNIT_NONE);
        }
    }
    else
    {
        // Unit information found:
        if(unitString == "deg" or unitString == "DEG")
        {
            unitLon.set("longitude", 1.0, 0.0, UNIT_DEG);
        }
        else
        {
            if(unitLon.set("longitude", 1.0, 0.0, unitString) != 0)
            {
                std::cerr << "Warning (DataCube): Unknown longitude unit \'" << unitString <<  "\'." << std::endl;
            }
        }
    }
    
    if(getHeader("CTYPE2", keywordString) != 0) keywordString = "PIXEL";
    
    if(getHeader("CUNIT2", unitString) != 0)
    {
        // No unit information found:
        iterSpat = typeSpat.find(keywordString.substr(0, keywordString.find_first_of('-')));
        
        if(iterSpat != typeSpat.end())
        {
            // Valid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT2 missing; using FITS default." << std::endl;
            unitLat.set("latitude", 1.0, 0.0, iterSpat->second);
        }
        else
        {
            // Invalid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT2 missing and CTYPE2 unknown." << std::endl;
            unitLat.set("latitude", 1.0, 0.0, UNIT_NONE);
        }
    }
    else
    {
        // Unit information found:
        if(unitString == "deg" or unitString == "DEG")
        {
            unitLat.set("latitude", 1.0, 0.0, UNIT_DEG);
        }
        else
        {
            if(unitLat.set("latitude", 1.0, 0.0, unitString) != 0)
            {
                std::cerr << "Warning (DataCube): Unknown latitude unit \'" << unitString <<  "\'." << std::endl;
            }
        }
    }
    
    if(getHeader("CTYPE3", keywordString) != 0) keywordString = "PIXEL";
    
    if(getHeader("CUNIT3", unitString) != 0)
    {
        // No unit information found:
        iterSpec = typeSpec.find(keywordString.substr(0, keywordString.find_first_of('-')));
        
        if(iterSpec != typeSpec.end())
        {
            // Valid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT3 missing; using FITS default." << std::endl;
            unitSpec.set("spectral", 1.0, 0.0, iterSpec->second);
        }
        else
        {
            // Invalid type keyword:
            std::cerr << "Warning (DataCube): Keyword CUNIT3 missing and CTYPE3 unknown." << std::endl;
            unitSpec.set("spectral", 1.0, 0.0, UNIT_NONE);
        }
    }
    else
    {
        // Unit information found:
        stringReplace(unitString, ".", " ");
        
        if(unitSpec.set("spectral", 1.0, 0.0, unitString) != 0)
        {
            std::cerr << "Warning (DataCube): Unknown spectral unit \'" << unitString <<  "\'." << std::endl;
        }
    }
    
    std::cout << "DataCube: Brightness unit: " << unitFlux.print() << std::endl;
    std::cout << "DataCube: Longitude unit:  " << unitLon.print() << std::endl;
    std::cout << "DataCube: Latitude unit:   " << unitLat.print() << std::endl;
    std::cout << "DataCube: Spectral unit:   " << unitSpec.print() << std::endl;*/
    
    /*for(unsigned int i = 0; i < header.size(); i++)
    {
        std::string key;
        std::string value;
        int         type;
        header.getKey(i, key);
        header.getValue(i, value);
        header.getType(i, type);
        std::cout << key << '\t' << value << '\t' << type << std::endl;
    }*/
    
    // Allocate memory for data:
    
    /*T *dataRow = new T[numberPixels[0]];           // Temporary memory for one row
    
    if(dataRow == 0)
    {
        fits_close_file(filePointer, &status);
        std::cerr << "Error (DataCube): Not enough memory to read FITS file." << std::endl;
        return 1;
    }
    
    data = new T[numberPixels[0] * numberPixels[1] * numberPixels[2]];
    
    if(data == 0)
    {
        delete[] dataRow;
        fits_close_file(filePointer, &status);
        std::cerr << "Error (DataCube): Not enough memory to read FITS file." << std::endl;
        return 1;
    }
    
    flagMemory = true;
    
    // Read data from FITS file:
    
    dataMin = std::numeric_limits<T>::max();
    dataMax = std::numeric_limits<T>::min();
    
    std::cout << "DataCube: Progress: 0%" << std::flush;
    
    for(firstPixel[2] = 1L; firstPixel[2] <= numberPixels[2]; firstPixel[2]++)
    {
        //std::cout << "DataCube: Progress: " << ((100L * firstPixel[2]) / numberPixels[2]) << "%\r" << std::flush;
        
        if((50L * firstPixel[2]) / numberPixels[2] > (50L * (firstPixel[2] - 1)) / numberPixels[2])
        {
            std::cout << "\rDataCube: Progress: " << ((100L * firstPixel[2]) / numberPixels[2]) << "%" << std::flush;
        }
        
        for(firstPixel[1] = 1L; firstPixel[1] <= numberPixels[1]; firstPixel[1]++)
        {
            // Check actual type T to point to the correct data type in fits_read_pix():
            if(typeid(T) == typeid(char))
            {
                if(fits_read_pix(filePointer, TBYTE, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else if(typeid(T) == typeid(short))
            {
                if(fits_read_pix(filePointer, TSHORT, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else if(typeid(T) == typeid(int))
            {
                if(fits_read_pix(filePointer, TINT, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else if(typeid(T) == typeid(long))
            {
                if(fits_read_pix(filePointer, TLONG, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else if(typeid(T) == typeid(float))
            {
                if(fits_read_pix(filePointer, TFLOAT, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else if(typeid(T) == typeid(double))
            {
                if(fits_read_pix(filePointer, TDOUBLE, firstPixel, numberPixels[0], NULL, dataRow, NULL, &status)) break;
            }
            else
            {
                std::cerr << "Error (DataCube): Unsupported data type (" << typeid(T).name() << ")." << std::endl;
                fits_close_file(filePointer, &status);
                delete[] dataRow;
                this->clear();
                return 1;
            }
            
            for(long i = 0L; i < numberPixels[0]; i++)
            {
                data[position(i, firstPixel[1] - 1L, firstPixel[2] - 1L)] = dataRow[i] ? dataRow[i] : static_cast<T>(0);
                
                if(dataRow[i] < dataMin)      dataMin = dataRow[i];
                else if(dataRow[i] > dataMax) dataMax = dataRow[i];
            }
        }
    }
    
    std::cout << std::endl;
    
    if(status != 0)
    {
        fits_report_error(stderr, status);
        status = 0;
        fits_close_file(filePointer, &status);
        delete[] dataRow;
        this->clear();
        return 1;
    }
    
    // Release temporary memory and close FITS file:
    
    delete[] dataRow;
    
    fits_close_file(filePointer, &status);
    
    // Lastly, try to read WCS from FITS file:
    
    if(wcs.readFitsFile(fileName)) std::cerr << "Warning (DataCube): No valid WCS definition found." << std::endl;
    else                           std::cout << "DataCube: Valid WCS definition found." << std::endl;
    
    return 0;
}



// --------------------------- //
// Function to write FITS file //
// --------------------------- //

// WARNING: This is a very crude implementation and will need to be cleaned up and made fail-safe!
// WARNING: One thing to check is whether the array size is correct according to the header.

template <typename T> int DataCube<T>::writeFitsFile(const std::string &fileName, const std::string &fileNameHeader)
{
    if(fileName.empty())
    {
        std::cerr << "Error (DataCube): Cannot create output file; no file name specified." << std::endl;
        return 1;
    }
    
    if(!this->isDefined())
    {
        std::cerr << "Error (DataCube): Cannot create output file; no data present." << std::endl;
        return 1;
    }
    
    int       status = 0;              // CFITSIO status variable MUST be set to 0!
    fitsfile *fptr;                    // CFITSIO file pointer
    fitsfile *hfptr;
    
    std::cout << "DataCube: Writing FITS file " << fileName << "." << std::endl;
    
    // Open header file:
    fits_open_file(&hfptr, fileNameHeader.c_str(), READONLY, &status);
    
    if(status != 0)
    {
        std::cerr << "Error (DataCube): Opening of FITS file " << fileNameHeader << " failed." << std::endl;
        fits_report_error(stderr, status);
        return 1;
    }
    
    // Create and open output file:
    fits_create_file(&fptr, fileName.c_str(), &status);
    
    if(status != 0)
    {
        std::cerr << "Error (DataCube): Creation of FITS file " << fileName << " failed." << std::endl;
        fits_report_error(stderr, status);
        status = 0;
        fits_close_file(hfptr, &status);
        return 1;
    }
    
    // Copy header from header file to output file:
    fits_copy_header(hfptr, fptr, &status);
    
    if(status != 0)
    {
        std::cerr << "Error (DataCube): Failed to copy header to output FITS file." << std::endl;
        fits_report_error(stderr, status);
        status = 0;
        fits_close_file(fptr, &status);
        status = 0;
        fits_close_file(hfptr, &status);
        return 1;
    }
    
    // Close header file (no longer needed):
    fits_close_file(hfptr, &status);
    status = 0;
    
    long  firstpix[3] = {1L, 1L, 1L};  // This MUST be initialised to 1!
    T     pix[numberPixels[0]];
    
    // Write data into output file:
    //std::cout.setf(std::ios::unitbuf); // Flush std::cout after every inserting operation
    std::cout << "DataCube: Progress: 0%" << std::flush;
    
    for(firstpix[2] = 1L; firstpix[2] <= numberPixels[2]; firstpix[2]++)
    {
        if((50L * firstpix[2]) / numberPixels[2] > (50L * (firstpix[2] - 1)) / numberPixels[2])
        {
            std::cout << "\rDataCube: Progress: " << ((100L * firstpix[2]) / numberPixels[2]) << "%" << std::flush;
        }
        
        for(firstpix[1] = 1L; firstpix[1] <= numberPixels[1]; firstpix[1]++)
        {
            for(long i = 0; i < numberPixels[0]; i++)
            {
                pix[i] = data[position(i, firstpix[1] - 1L, firstpix[2] - 1L)];
            }
            
            if(     typeid(T) == typeid(char))   fits_write_pix(fptr, TBYTE,   firstpix, numberPixels[0], pix, &status);
            else if(typeid(T) == typeid(short))  fits_write_pix(fptr, TSHORT,  firstpix, numberPixels[0], pix, &status);
            else if(typeid(T) == typeid(int))    fits_write_pix(fptr, TINT,    firstpix, numberPixels[0], pix, &status);
            else if(typeid(T) == typeid(long))   fits_write_pix(fptr, TLONG,   firstpix, numberPixels[0], pix, &status);
            else if(typeid(T) == typeid(float))  fits_write_pix(fptr, TFLOAT,  firstpix, numberPixels[0], pix, &status);
            else if(typeid(T) == typeid(double)) fits_write_pix(fptr, TDOUBLE, firstpix, numberPixels[0], pix, &status);
            else
            {
                std::cerr << "Error (DataCube): Unsupported data type (" << typeid(T).name() << ")." << std::endl;
                fits_close_file(fptr, &status);
                return 1;
            }
        }
    }
    
    std::cout << std::endl;
    
    fits_close_file(fptr, &status);
    
    return 0;
}*/



// --------------------------------------- //
// Overloaded function to set header entry //
// --------------------------------------- //

template <typename T> int DataCube<T>::setHeader(const std::string &keyName, const std::string &value)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    unsigned int position = header.findEntry(searchKey);
    
    if(position == header.size()) header.addEntry(searchKey, value);
    else header.modifyEntry(searchKey, value);
    
    return 0;
}

template <typename T> int DataCube<T>::setHeader(const std::string &keyName, int value)
{
    std::stringstream s;
    s << value;
    std::string valueString = s.str();
    
    return setHeader(keyName, valueString);
}

template <typename T> int DataCube<T>::setHeader(const std::string &keyName, long value)
{
    std::stringstream s;
    s << value;
    std::string valueString = s.str();
    
    return setHeader(keyName, valueString);
}

template <typename T> int DataCube<T>::setHeader(const std::string &keyName, float value)
{
    std::stringstream s;
    s << value;
    std::string valueString = s.str();
    
    return setHeader(keyName, valueString);
}

template <typename T> int DataCube<T>::setHeader(const std::string &keyName, double value)
{
    std::stringstream s;
    s << value;
    std::string valueString = s.str();
    
    return setHeader(keyName, valueString);
}

template <typename T> int DataCube<T>::setHeader(const char *keyName, const char *value)
{
    return setHeader(std::string(keyName), std::string(value));
}

template <typename T> int DataCube<T>::setHeader(const char *keyName, int value)
{
    return setHeader(std::string(keyName), value);
}

template <typename T> int DataCube<T>::setHeader(const char *keyName, long value)
{
    return setHeader(std::string(keyName), value);
}

template <typename T> int DataCube<T>::setHeader(const char *keyName, float value)
{
    return setHeader(std::string(keyName), value);
}

template <typename T> int DataCube<T>::setHeader(const char *keyName, double value)
{
    return setHeader(std::string(keyName), value);
}



// ------------------------------------------ //
// Overloaded function to return header entry //
// ------------------------------------------ //

template <typename T> int DataCube<T>::getHeader(const std::string &keyName, std::string &result)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    if(header.getEntry(searchKey, result) != 0)
    {
        std::cerr << "Warning (DataCube): Header key \'" << searchKey << "\' not found." << std::endl;
        return 1;
    }
    
    return 0;
}

template <typename T> int DataCube<T>::getHeader(const std::string &keyName, int &result)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    if(header.getEntry(searchKey, result) != 0)
    {
        std::cerr << "Warning (DataCube): Header key \'" << searchKey << "\' not found." << std::endl;
        return 1;
    }
    
    return 0;
}

template <typename T> int DataCube<T>::getHeader(const std::string &keyName, long &result)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    if(header.getEntry(searchKey, result) != 0)
    {
        std::cerr << "Warning (DataCube): Header key \'" << searchKey << "\' not found." << std::endl;
        return 1;
    }
    
    return 0;
}

template <typename T> int DataCube<T>::getHeader(const std::string &keyName, float &result)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    if(header.getEntry(searchKey, result) != 0)
    {
        std::cerr << "Warning (DataCube): Header key \'" << searchKey << "\' not found." << std::endl;
        return 1;
    }
    
    return 0;
}

template <typename T> int DataCube<T>::getHeader(const std::string &keyName, double &result)
{
    std::string searchKey(keyName);
    stringToUpper(searchKey);
    
    if(header.getEntry(searchKey, result) != 0)
    {
        std::cerr << "Warning (DataCube): Header key \'" << searchKey << "\' not found." << std::endl;
        return 1;
    }
    
    return 0;
}

template <typename T> int DataCube<T>::getHeader(const char *keyName, std::string &result)
{
    return getHeader(std::string(keyName), result);
}

template <typename T> int DataCube<T>::getHeader(const char *keyName, int &result)
{
    return getHeader(std::string(keyName), result);
}

template <typename T> int DataCube<T>::getHeader(const char *keyName, long &result)
{
    return getHeader(std::string(keyName), result);
}

template <typename T> int DataCube<T>::getHeader(const char *keyName, float &result)
{
    return getHeader(std::string(keyName), result);
}

template <typename T> int DataCube<T>::getHeader(const char *keyName, double &result)
{
    return getHeader(std::string(keyName), result);
}



// ------------------------------------------------------- //
// Function to convert x, y, and z into 1-D array position //
// ------------------------------------------------------- //

template <typename T> long DataCube<T>::position(long x, long y, long z)
{
    //return z + numberPixels[2] * (y + numberPixels[1] * x);
    return x + numberPixels[0] * (y + numberPixels[1] * z);
}



// ############################################################ //
// Instantiate different templates so they can be pre-compiled: //
// ############################################################ //

template class DataCube<char>;
template class DataCube<short>;
template class DataCube<int>;
template class DataCube<long>;
template class DataCube<float>;
template class DataCube<double>;
