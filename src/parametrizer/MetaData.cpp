#include <iostream>

#include "helperFunctions.h"
#include "MetaData.h"



MetaData::MetaData()
{
    return;
}



// Function to return whether header is defined

bool MetaData::isDefined()
{
    if(header.empty()) return false;
    else               return true;
}



// Function to clear header

void MetaData::clear()
{
    if(!header.empty()) header.clear();
    
    return;
}



// Function to return header size

unsigned int MetaData::size()
{
    return header.size();
}



// Function to find header entry and return its position
// (returns header.size() if element not found)

unsigned int MetaData::findEntry(const std::string &key)
{
    if(header.empty() or key.empty()) return header.size();
    
    unsigned int position = 0;
    bool found = false;
    
    while(position < header.size() and found == false)
    {
        if(header.at(position).key == key) found = true;
        else position++;
    }
    
    if(found == false) return header.size();
    else return position;
}



// Functions to return key, value, and type at given position

int MetaData::getKey(unsigned int position, std::string &result)
{
    if(header.empty() or position < 0 or position >= header.size())
    {
        std::cerr << "Error (MetaData): Invalid header position." << std::endl;
        return 1;
    }
    else
    {
        result = header.at(position).key;
        return 0;
    }
}

int MetaData::getValue(unsigned int position, std::string &result)
{
    if(header.empty() or position < 0 or position >= header.size())
    {
        std::cerr << "Error (MetaData): Invalid header position." << std::endl;
        return 1;
    }
    else
    {
        result = header.at(position).value;
        return 0;
    }
}

int MetaData::getType(unsigned int position, int &result)
{
    if(header.empty() or position < 0 or position >= header.size())
    {
        std::cerr << "Error (MetaData): Invalid header position." << std::endl;
        return 1;
    }
    else
    {
        result = header.at(position).type;
        return 0;
    }
}



// Functions to add header entry

int MetaData::addEntry(const std::string &newKey, const std::string &newValue, int type)
{
    if(newKey.empty() or newValue.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key or value missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = newValue;
    entry.unit  = "";
    entry.type  = type;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, float newValue)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = "";
    entry.type  = METADATA_FLOAT;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, double newValue)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = "";
    entry.type  = METADATA_DOUBLE;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, int newValue)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = "";
    entry.type  = METADATA_INT;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, long newValue)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = "";
    entry.type  = METADATA_LONG;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, bool newValue)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    if(newValue == true) entry.value = "1";
    else entry.value = "0";
    entry.unit  = "";
    entry.type  = METADATA_BOOL;
    
    header.push_back(entry);
    
    return 0;
}



// Same, but including unit information

int MetaData::addEntry(const std::string &newKey, float newValue, std::string &unitString, int type)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = unitString;
    entry.type  = type;
    
    header.push_back(entry);
    
    return 0;
}

int MetaData::addEntry(const std::string &newKey, double newValue, std::string &unitString, int type)
{
    if(newKey.empty())
    {
        std::cerr << "Error (MetaData): Cannot add header entry; key missing." << std::endl;
        return 1;
    }
    
    struct Entry entry;
    
    entry.key   = newKey;
    entry.value = numberToString(newValue);
    entry.unit  = unitString;
    entry.type  = type;
    
    header.push_back(entry);
    
    return 0;
}



// Function to modify header entry
// (this will create a new entry if the key does not yet exist)

int MetaData::modifyEntry(const std::string &key, const std::string &value)
{
    if(key.empty() or value.empty())
    {
        std::cerr << "Error (MetaData): Cannot modify header entry; key or value missing." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Warning (MetaData): Key \'" << key << "\' not found; creating new header entry." << std::endl;
        if(addEntry(key, value) != 0) return 1;
    }
    else
    {
        header.at(position).value = value;
    }
    
    return 0;
}



// Functions to read header entry

int MetaData::getEntry(const std::string &key, int &value)
{
    if(key.empty())
    {
        std::cerr << "Error (MetaData): Cannot get header entry; no key specified." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Error (MetaData): Header entry \'" << key << "\' not found." << std::endl;
        return 1;
    }
    
    value = stringToNumber<int>(header.at(position).value);
    
    return 0;
}

int MetaData::getEntry(const std::string &key, long &value)
{
    if(key.empty())
    {
        std::cerr << "Error (MetaData): Cannot get header entry; no key specified." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Error (MetaData): Header entry \'" << key << "\' not found." << std::endl;
        return 1;
    }
    
    value = stringToNumber<long>(header.at(position).value);
    
    return 0;
}

int MetaData::getEntry(const std::string &key, float &value)
{
    if(key.empty())
    {
        std::cerr << "Error (MetaData): Cannot get header entry; no key specified." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Error (MetaData): Header entry \'" << key << "\' not found." << std::endl;
        return 1;
    }
    
    value = stringToNumber<float>(header.at(position).value);
    
    return 0;
}

int MetaData::getEntry(const std::string &key, double &value)
{
    if(key.empty())
    {
        std::cerr << "Error (MetaData): Cannot get header entry; no key specified." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Error (MetaData): Header entry \'" << key << "\' not found." << std::endl;
        return 1;
    }
    
    value = stringToNumber<double>(header.at(position).value);
    
    return 0;
}

int MetaData::getEntry(const std::string &key, std::string &value)
{
    if(key.empty())
    {
        std::cerr << "Error (MetaData): Cannot get header entry; no key specified." << std::endl;
        return 1;
    }
    
    unsigned int position = findEntry(key);
    
    if(position == header.size())
    {
        std::cerr << "Error (MetaData): Header entry \'" << key << "\' not found." << std::endl;
        return 1;
    }
    
    value = header.at(position).value;
    
    return 0;
}
