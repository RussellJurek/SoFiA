#ifndef METADATA_H
#define METADATA_H

#define METADATA_STRING 0
#define METADATA_INT    1
#define METADATA_LONG   2
#define METADATA_FLOAT  3
#define METADATA_DOUBLE 4
#define METADATA_BOOL   5

#include <string>
#include <vector>

class MetaData
{
public:
    MetaData();
    
    int addEntry(const std::string &newKey, bool newValue);
    int addEntry(const std::string &newKey, int newValue);
    int addEntry(const std::string &newKey, long newValue);
    int addEntry(const std::string &newKey, float newValue);
    int addEntry(const std::string &newKey, double newValue);
    int addEntry(const std::string &newKey, const std::string &newValue, int type = METADATA_STRING);
    
    int addEntry(const std::string &newKey, float newValue, std::string &unitString, int type = METADATA_STRING);
    int addEntry(const std::string &newKey, double newValue, std::string &unitString, int type = METADATA_STRING);
    
    int modifyEntry(const std::string &key, const std::string &value);
    
    int getEntry(const std::string &key, int &value);
    int getEntry(const std::string &key, long &value);
    int getEntry(const std::string &key, float &value);
    int getEntry(const std::string &key, double &value);
    int getEntry(const std::string &key, std::string &value);
    
    int getKey(unsigned int position, std::string &result);
    int getValue(unsigned int position, std::string &result);
    int getType(unsigned int position, int &result);
    
    unsigned int findEntry(const std::string &key);
    unsigned int size();
    
    bool isDefined();
    void clear();
    
private:
    struct Entry
    {
        std::string key;
        std::string value;
        std::string unit;
        int         type;
    };
    
    std::vector<struct Entry> header;
};

#endif
