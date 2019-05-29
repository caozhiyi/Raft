#ifndef HEADER_UNIXOS
#define HEADER_UNIXOS
#include <string>

// enable dump file
void SetCoreFileUnlimit();

// get local ip
std::string GetOsIp();

#endif