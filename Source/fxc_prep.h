#ifndef _FXC_PREP_H
#define _FXC_PREP_H
#include <string>
#include <vector>
#include <sstream>

void fxc_prep(std::string shader, bool nvidia, bool ps2a, bool x360, bool produceCompiledVcs, bool produceCppClasses, const std::string shaderpath);

std::vector<std::string> ReadInputFile(const std::string shaderpath, std::string filename);
std::string GetShaderType(const std::string shadername, const bool debug, const bool ps2a);
std::string WriteHelperVar(const std::string name, const int min, const int max);
inline std::string AppendInt(const int derp) {
	std::ostringstream out;
	out << derp;
	return out.str();
}
#endif //_FXC_PREP_H