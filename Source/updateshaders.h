#ifndef _UPDATESHADERS_H
#define _UPDATESHADERS_H
#include <string>
#include <vector>

void updateshaders(std::string filename, bool x360);
std::vector<std::string> GetAsmShaderDependencies_R(std::string shadername, std::vector<std::string> dep);

#endif //_UPDATESHADERS_H