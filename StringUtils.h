#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <iostream>
#include <vector>
#include <string.h>

using std::string;
using std::vector;



///
/// Class to provide abstracted functionality for strings
///
class StringUtils {

private:



public:

    StringUtils() { };

    vector<string> &split(const string &s, const char delim, vector<string> &elems);
    vector<string> split(const string &s, const char delim);


};

#endif
