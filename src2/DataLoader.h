#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <string>
#include "LogData.h"

using namespace std;

EventType getEventType(const string& typeStr);
Location getLocation(const string& locStr);
void loadCSVData(const string& filename, LogPool& pool);

#endif