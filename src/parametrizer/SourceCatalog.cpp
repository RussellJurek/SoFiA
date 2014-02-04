// Compilation: g++ -c SourceCatalog.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "helperFunctions.h"
#include "SourceCatalog.h"


// ----------- //
// Constructor //
// ----------- //

SourceCatalog::SourceCatalog()
{
    return;
}

SourceCatalog::SourceCatalog(const SourceCatalog &sC)
{
    sources = sC.sources;
    
    return;
}

// ---------- //
// Destructor //
// ---------- //

SourceCatalog::~SourceCatalog()
{
    return;
}


// ---------------------------------------------- //
// Function to return number of catalogue sources //
// ---------------------------------------------- //

unsigned long SourceCatalog::size()
{
    return sources.size();
}


// ------------------------------------- //
// Function to return ID of first source //
// ------------------------------------- //

unsigned long SourceCatalog::first()
{
    std::map<unsigned long, Source>::iterator iter = sources.begin();
    
    if(iter == sources.end())
    {
        std::cerr << "Error (Catalog): No sources found in catalogue." << std::endl;
        return 1;
    }
    
    return iter->first;
}


// ------------------------------------ //
// Function to return ID of last source //
// ------------------------------------ //

unsigned long SourceCatalog::last()
{
    std::map<unsigned long, Source>::iterator iter = sources.end();
    
    if(iter == sources.end())
    {
        std::cerr << "Error (Catalog): No sources found in catalogue." << std::endl;
        return 1;
    }
    
    return iter->first;
}


// ------------------------------- //
// Function to insert a new source //
// ------------------------------- //

int SourceCatalog::insert(Source &source)
{
    unsigned long sourceID = source.getSourceID();
    
    // Wow, this looks complicated:
    std::pair<std::map<unsigned long, Source>::iterator, bool> ret = sources.insert(std::pair<unsigned long, Source>(sourceID, source));
    
    if(ret.second == false)
    {
        std::cerr << "Warning (Catalog): Source already exists; replacing current entry." << std::endl;
        update(sourceID, source);
        
        return 1;
    }
    
    return 0;
}


// ------------------------------------- //
// Function to update an existing source //
// ------------------------------------- //

int SourceCatalog::update(unsigned long sourceID, Source &source)
{
    std::map<unsigned long, Source>::iterator iter = sources.find(sourceID);
    
    if(iter == sources.end())
    {
        std::cerr << "Warning (Catalog): Source does not exist; creating new entry." << std::endl;
        insert(source);
        
        return 1;
    }
    
    iter->second = source;
    
    return 0;
}


// ------------------------------------- //
// Function to remove an existing source //
// ------------------------------------- //

int SourceCatalog::remove(unsigned long sourceID)
{
    std::map<unsigned long, Source>::iterator iter = sources.find(sourceID);
    
    if(iter == sources.end())
    {
        std::cerr << "Error (Catalog): Source ID not found; failed to remove source." << std::endl;
        
        return 1;
    }
    
    sources.erase(iter);
    
    return 0;
}


// ------------------------------------- //
// Function to return a catalogue source //
// ------------------------------------- //

Source *SourceCatalog::getSource(unsigned long sourceID)
{
    std::map<unsigned long, Source>::iterator iter = sources.find(sourceID);
    
    if(iter == sources.end())
    {
        std::cerr << "Error (Catalog): Source ID not found." << std::endl;
        return 0;
    }
    else
    {
        return &(iter->second);
    }
}


// ---------------------------------------- //
// Functions to read Duchamp catalogue file //
// ---------------------------------------- //

int SourceCatalog::readDuchampFile(const std::string &filename)
{
    return readDuchampFile(filename.c_str());
}

int SourceCatalog::readDuchampFile(const char *filename)
{
    std::ifstream  fp;
    std::string    readline;
    
    // Open catalogue file:
    
    std::cout << "Catalog: Reading catalogue file " << filename << "." << std::endl;
    
    // Read in catalogue:
    
    fp.open(filename);
    
    if(!fp.is_open())
    {
        std::cerr << "Error (Catalog): Opening of catalogue " << filename << " failed." << std::endl;
        return 1;
    }
    
    // Define parameter names to be read in, in the correct column order:
    std::vector<std::string> parameterNames;
    parameterNames.push_back("ID");
    parameterNames.push_back("Name");
    parameterNames.push_back("X");
    parameterNames.push_back("Y");
    parameterNames.push_back("Z");
    parameterNames.push_back("RA");
    parameterNames.push_back("DEC");
    parameterNames.push_back("FREQ");
    parameterNames.push_back("w_RA");
    parameterNames.push_back("w_DEC");
    parameterNames.push_back("w_50");
    parameterNames.push_back("w_20");
    parameterNames.push_back("w_FREQ");
    parameterNames.push_back("F_int");
    parameterNames.push_back("F_tot");
    parameterNames.push_back("F_peak");
    
    while(fp.good())
    {
        getline(fp, readline);
        
        std::vector<std::string> tokens;
        
        if(readline.length() > 200)
        {
            stringTok(readline, tokens, " \t\n");
            
            if(tokens.size() >= parameterNames.size())
            {
                unsigned long sourceID = stringToNumber<unsigned long>(tokens.at(0));
                
                if(sourceID > 0)
                {
                    Source source;
                    source.setSourceID(sourceID);
                    source.setSourceName(tokens.at(1));
                    
                    for(unsigned int i = 2; i < parameterNames.size(); i++)
                    {
                        if(parameterNames.at(i) == "RA" or parameterNames.at(i) == "DEC")
                        {
                            std::vector<std::string> tokens2;
                            
                            stringTok(tokens.at(i), tokens2, ":");
                            
                            if(tokens2.size() == 3)
                            {
                                int    value1 = stringToNumber<int>(tokens2.at(0));
                                int    value2 = stringToNumber<int>(tokens2.at(1));
                                double value3 = stringToNumber<double>(tokens2.at(2));
                                
                                double value  = static_cast<double>(mathSgn(value1)) * (static_cast<double>(std::abs(value1)) + (static_cast<double>(value2) / 60.0) + (value3 / 3600.0));
                                
                                source.setParameter(parameterNames.at(i), value);
                            }
                            else
                            {
                                std::cerr << "Warning (Catalog): Failed to read RA/Dec from Duchamp file." << std::endl;
                                std::cerr << "                   Parameter will be omitted." << std::endl;
                            }
                        }
                        else
                        {
                            source.setParameter(parameterNames.at(i), stringToNumber<double>(tokens.at(i)));
                        }
                    }
                    
                    // Add new source to catalogue:
                    sources.insert(std::pair<unsigned long, Source>(sourceID, source));
                }
            }
        }
    }
    
    fp.close();
    
    return 0;
}
