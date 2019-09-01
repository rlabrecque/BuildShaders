#ifndef _HELPERS_H
#define _HELPERS_H
#include <string>
#include <vector>

std::vector<std::string> LoadShaderListFile(const std::string filename);
std::string LoadShaderListFile_GetShaderBase(std::string shadername);
std::string LoadShaderListFile_GetShaderType(const std::string shadername);
std::string LoadShaderListFile_GetShaderSrc(const std::string shadername);
bool FileExists(const std::string filename, bool testwrite);
bool DirectoryExists(const std::string filename);

#endif //_HELPERS_H