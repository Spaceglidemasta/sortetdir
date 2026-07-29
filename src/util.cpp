#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <vector>


#pragma once

char** strvecToCharPtrArr(const std::vector<std::string>& strs) {
    char** arr = new char*[strs.size()];
    for (size_t i = 0; i < strs.size(); ++i)
        arr[i] = const_cast<char*>(strs[i].c_str());
    return arr;
}