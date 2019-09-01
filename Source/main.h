#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <iostream>
#include <assert.h>

extern std::vector<std::string> g_launchArgs;
extern bool g_bDynamicCompile; // Use dynamic shaders to build .inc files only
extern short g_nDirectX_Force; //-force30
extern std::string g_strShaderPath;
extern std::string g_strGameDir;

inline void Usage() {
	std::cout << "Usage:\n";
	std::cout << "buildshaders -list <shaderProjectName> -game <GameDir> -source <SourceDir>\n";
	std::cout << "gameDir is where gameinfo.txt is (where it will store the compiled shaders).\n";
	std::cout << "sourceDir is where the source code is (where it will find scripts and compilers).\n";
	//std::cout << "ex   : buildshaders myshaders\n";
	std::cout << "ex: buildshaders -list stdshader_dx9_30.txt -game c:/steam/steamapps/sourcemods/mymod -source c:/mymod/src\n";
	system("pause");
	exit(EXIT_FAILURE);
}

inline bool FindArg(const std::string szArg) {
	for(unsigned short nArg = 0; nArg < g_launchArgs.size(); ++nArg) {
		//std::cout << lastshader << " depends on " << dep[dependancy] << "\n";
		if(g_launchArgs[nArg] == szArg) {
			return true;
		}
	}
	return false;
}

inline std::string FindArgStr(const std::string szArg) {
	for(unsigned char nArg = 0; nArg < g_launchArgs.size(); ++nArg) {
		if(g_launchArgs[nArg] == szArg && nArg != (g_launchArgs.size() - 1) && g_launchArgs[nArg + 1].at(0) != '-') {
			return g_launchArgs[nArg + 1];
		}
	}
	return "";
}