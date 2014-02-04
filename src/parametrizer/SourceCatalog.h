#ifndef SOURCECATALOG_H
#define SOURCECATALOG_H

#include <vector>

#include "Source.h"

class SourceCatalog
{
public:
    SourceCatalog();
    SourceCatalog(const SourceCatalog &sourceCatalog);
    ~SourceCatalog();
    
    int           readDuchampFile(const char *filename);
    int           readDuchampFile(const std::string &filename);
    int           insert(Source &source);
    int           update(unsigned long sourceID, Source &source);
    int           remove(unsigned long sourceID);
    Source       *getSource(unsigned long sourceID);
    unsigned long size();
    unsigned long first();
    unsigned long last();
    
    std::map<unsigned long,Source> & getSources()
    {
        return sources;
    }
    void setSources(std::map<unsigned long,Source> s)
    {
        sources=s;
    }
    void clear()
    {
        sources.clear();
    }
    
private:
    std::map<unsigned long, Source> sources;
};

#endif
