
#include <fstream>
#include "main.h"
#include "updateshaders.h"
#include "fxc_prep.h"
#include "helpers.h"
#include "copyshaderincfiles.h"

//-list "C:\Users\Riley\Documents\Code\The Sandbox\src\materialsystem\stdshaders\stdshader_dx9_30.txt" -dx9_30 -force30 -game "C:\Games\Steam\steamapps\sourcemods\thesandbox" -source "C:\Users\Riley\Documents\Code\The Sandbox\src" -sdk "C:\Games\Steam\steamapps\dafox1\sourcesdk\bin\source2007\bin"
//-list "C:\Users\Riley\Desktop\orangebox\materialsystem\stdshaders\stdshader_dx9_30.txt" -source "C:\Users\Riley\Desktop\orangebox" -game "C:\Users\Riley\Desktop\orangebox\gamedir" -sdk "C:\Games\Steam\steamapps\dafox1\sourcesdk\bin\source2007\bin" -dx9_30 -force30
//-list "F:\Code\Slide\src\materialsystem\stdshaders\stdshader_dx9_30.txt" -source "F:\Code\Slide\src" -game "C:\Games\Steam\steamapps\sourcemods\slide" -sdk "C:\Games\Steam\steamapps\common\alien swarm\bin" -dx9_30 -force30

std::vector<std::string> g_launchArgs;
bool g_bDynamicCompile;
short g_nDirectX_Force;
std::string g_strShaderPath;
std::string g_strGameDir;

//todo: -v[erbose]

/*
set SrcDirBase=..\..
set SDKArgs=
set SHADERINCPATH=vshtmp9/... fxctmp9/...
set DIRECTX_SDK_VER=pc09.00
set DIRECTX_SDK_BIN_DIR=dx9sdk\utilities

if("-dx9_30") {
set DIRECTX_SDK_VER=pc09.30
set DIRECTX_SDK_BIN_DIR=dx10sdk\utilities\dx9_30
*/
int main(int argc, char* argv[]) {
	std::cout << "****************\n";
	std::cout << "DaFox's Shader Compiler\n";
	std::cout << "****************\n";
	
	for (int i = 0; i < argc; ++i) {
		std::cout << "Arg " << i + 1 << " = " << argv[i] << "\n";
		g_launchArgs.push_back(argv[i]);
	}
	std::cout << '\n';

	std::string szDirectX_SDK = "pc09.00";
	std::string szDirectX_SDK_Bin_Dir = "\\dx9sdk\\utilities";
	if(FindArg("-dx9_30")) {
		szDirectX_SDK = "pc09.30";
		szDirectX_SDK_Bin_Dir = "\\dx10sdk\\utilities\\dx9_30";
	}
	if(FindArg("-force30")) {
		g_nDirectX_Force = 30;
	}

	bool bDynamicCompile = FindArg("-dyn"); // Use dynamic shaders to build .inc files only
	//bool bNvidia = FindArg("-nv3x"); //only fxc_prep uses this right now 
	bool bps2a = FindArg("-ps20a");
	bool bX360 = FindArg("-x360");
	bool bProduceCompiledVcs = !FindArg("-novcs"); //todo: backwards...
	bool bProduceCppClasses = !FindArg("-nocpp");

	if(FindArg("-fxc_prep")) {
		if(FindArgStr("-shader") == "" || FindArgStr("-shaderdir") == "") {
			std::cout << "ERROR: fxc_prep failed. fxc_prep must be called with valid -shader and -shaderdir params\n";
				return EXIT_FAILURE;
		}

		fxc_prep(FindArgStr("-shader"), FindArg("-nv3x"), bps2a, bX360, bProduceCompiledVcs, bProduceCppClasses, FindArgStr("-shaderdir"));
		//g_srcdir,
		return EXIT_SUCCESS;
	}
	else if(FindArg("-psh_prep")) {
		return EXIT_SUCCESS;
	}
	else if(FindArg("-vsh_prep")) {
		return EXIT_SUCCESS;
	}

	std::string strShaderProjectName = FindArgStr("-list"); //stdshader_dx9_30 //This should really be JUST the filename, not the path. But it's both.
	if(strShaderProjectName == "" || !FileExists(strShaderProjectName, false)) {
		std::cout << "ERROR: You must specify a valid -list file.\n";
		std::cout << "ERROR: Path: \"" << strShaderProjectName << "\"\n";
		Usage();
		//todo: Launch our GUI window here
	}
	g_strShaderPath = FindArgStr("-list");
	g_strShaderPath.erase(g_strShaderPath.find_last_of('\\')); //Really this should have the \\ at the end, but it doesn't.


	std::string strSourceDir = FindArgStr("-source");
	if(strSourceDir == "") { //todo: give a default szSrcDir at some point. ..\.. was the default in the old scripts. Maybe ShaderPath + ..\..
		std::cout << "ERROR: You must specify a valid -source dir.\n";
		std::cout << "ERROR: Path: \"" << strSourceDir << "\"\n";
		Usage();
	}

	g_strGameDir = FindArgStr("-game");
	if(g_strGameDir == "" || !FileExists((g_strGameDir + "\\gameinfo.txt").c_str(), false)) {
		std::cout << "ERROR: You must specify a valid -game dir. (gameinfo.txt not found)\n";
		std::cout << "ERROR: Path: \"" << g_strGameDir << "\"\n";
		Usage();
	}
	
	std::string strSDKBinDir = FindArgStr("-sdk");
	if(strSDKBinDir == "" || !FileExists((strSDKBinDir + "\\shadercompile.exe").c_str(), false)) {
		std::cout << "ERROR: You must specify a valid SDK Binary Directory with -sdk (shadercompile.exe not found)\n";
		std::cout << "ERROR: Path: \"" << strSDKBinDir << "\"\n";
		Usage();
	}
		
	std::string strShadersOutDir = g_strShaderPath + "\\shaders"; //.\shaders, redefined with -dir
	if(FindArgStr("-dir") != "") {
		strShadersOutDir = /*szShaderPath + '\\' + */FindArgStr("-dir"); //todo: This could be moved to the declaration once we get fallback args going.
	}

	if(!FileExists(strShadersOutDir, false)) {
		CreateDirectoryA(strShadersOutDir.c_str(), nullptr);
	}
	std::string strTempDir = strShadersOutDir + "\\fxc";
	if(!FileExists(strShadersOutDir, false)) {
		CreateDirectoryA(strTempDir.c_str(), nullptr);
	}
	strTempDir = strShadersOutDir + "\\vsh";
	if(!FileExists(strTempDir, false)) {
		CreateDirectoryA(strTempDir.c_str(), nullptr);
	}
	strTempDir = strShadersOutDir + "\\psh";
	if(!FileExists(strTempDir, false)) {
		CreateDirectoryA(strTempDir.c_str(), nullptr);
	}

	//todo: I don't know why we are deleting these instead of just overwriting them.
	//todo: should we also be deleting .makefile, .makefile.copy
	strTempDir = g_strShaderPath + "\\filelist.txt";
	if(FileExists(strTempDir.c_str(), false)) {
		remove(strTempDir.c_str());
	}
	strTempDir = g_strShaderPath + "\\filestocopy.txt";
	if(FileExists(strTempDir.c_str(), false)) {
		remove(strTempDir.c_str());
	}
	strTempDir = g_strShaderPath + "\\filelistgen.txt";
	if(FileExists(strTempDir.c_str(), false)) {
		remove(strTempDir.c_str());
	}
	strTempDir = g_strShaderPath + "\\inclist.txt";
	if(FileExists(strTempDir.c_str(), false)) {
		remove(strTempDir.c_str());
	}
	strTempDir = g_strShaderPath + "\\vcslist.txt";
	if(FileExists(strTempDir.c_str(), false)) {
		remove(strTempDir.c_str());
	}

	updateshaders(strShaderProjectName, bX360);

	//Run the makefile, generating minimal work/build list for fxc files, go ahead and compile vsh and psh files.
	std::cout << "\nBuilding inc files, asm vcs files, and VMPI worklist for " << strShaderProjectName << ".makefile...\n\n";
	system("call \"%VS100COMNTOOLS%vsvars32.bat\""); //todo: not needed I don't believe?
	system(("call nmake.exe /B /C /S -f \"" + strShaderProjectName + ".makefile\"").c_str());
#if 0
	STARTUPINFOA si = {sizeof(STARTUPINFOA)};
	PROCESS_INFORMATION pi = {};

	std::string str1 = /*"\"C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\nmake.exe\"*/"/B /C /S -f \"" + strShaderProjectName + ".makefile\""; //no longer works without /B! I don't think that vsvars32.bat is holding here.
	std::vector<char> CmdLine(str1.begin(), str1.end()); //haha hacky.
	CmdLine.push_back('\0');

	std::cout << "NMake Args: " << &CmdLine[0] << "\n";
	
	if(CreateProcessA("C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\VC\\bin\\nmake.exe", &CmdLine[0], NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD ExitCode;
		GetExitCodeProcess(pi.hProcess, &ExitCode);
		std::cout << "NMAKE Ended with Return Code: " << ExitCode << "\n";
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		std::cout << "ERROR: CreateProcess failed! Code: " << GetLastError() << "\n";
		return EXIT_FAILURE;
	}
#endif

	/*if exist "inclist.txt" (
	echo Publishing shader inc files to target...
	perl %SrcDirBase%\devtools\bin\copyshaderincfiles.pl inclist.txt
	)*/
	if(FileExists((g_strShaderPath + "\\inclist.txt").c_str(), false)) {
		std::cout << "Publishing shader inc files to target...\n";
		copyshaderincfiles((g_strShaderPath + "\\inclist.txt").c_str(), bX360);
	}
	else {
		std::cout << "ERROR: The inclist.txt file does not exist!\n";
		return EXIT_FAILURE;
	}

	/*REM ****************
	REM Add the executables to the worklist.
	REM ****************
	if /i "%DIRECTX_SDK_VER%" == "pc09.00" (
	rem echo "Copy extra files for dx 9 std
	)
	if /i "%DIRECTX_SDK_VER%" == "pc09.30" (
	echo %SrcDirBase%\devtools\bin\d3dx9_33.dll >> filestocopy.txt
	)*/
	FILE* pFilesToCopy;
	if(fopen_s(&pFilesToCopy, (g_strShaderPath + "\\filestocopy.txt").c_str(), "a") != 0) {
		std::cout << "ERROR: The file filestocopy.txt was not opened!\n";
		return EXIT_FAILURE;
	}

	std::string tmpoutput = strSourceDir;
	tmpoutput.erase(0, 3);
	if(szDirectX_SDK == "pc09.00") {
		//todo: "Copy extra files for dx 9 std
	}
	else if(szDirectX_SDK == "pc09.30") {
		fprintf(pFilesToCopy, "%s", (strSourceDir + "\\devtools\\bin\\d3dx9_33.dll\n").c_str());
	}

	fprintf(pFilesToCopy, "%s", (strSourceDir + szDirectX_SDK_Bin_Dir + "\\dx_proxy.dll\n").c_str());
	tmpoutput = strSDKBinDir;
	tmpoutput.erase(0, 3);
	fprintf(pFilesToCopy, "%s", (strSDKBinDir + "\\shadercompile.exe\n").c_str());
	fprintf(pFilesToCopy, "%s", (strSDKBinDir + "\\shadercompile_dll.dll\n").c_str());
	fprintf(pFilesToCopy, "%s", (strSDKBinDir + "\\vstdlib.dll\n").c_str());
	fprintf(pFilesToCopy, "%s", (strSDKBinDir + "\\tier0.dll").c_str());

	fclose(pFilesToCopy);
	

	/*REM ****************
	REM Cull duplicate entries in work/build list
	REM ****************
	if exist filestocopy.txt type filestocopy.txt | perl "%SrcDirBase%\devtools\bin\uniqifylist.pl" > uniquefilestocopy.txt
	if exist filelistgen.txt if not "%dynamic_shaders%" == "1" (
	echo Generating action list...
	copy filelistgen.txt filelist.txt >nul
	)*/

	if(FileExists((g_strShaderPath + "\\filestocopy.txt").c_str(), false)) { //todo: reevaluate this algorithm.
		std::ifstream TXTFILE((g_strShaderPath + "\\filestocopy.txt").c_str());
		std::vector<std::string> uniquestrings;
		if(TXTFILE.is_open()) {
			std::string line;
			bool bFound;
			while(TXTFILE.good()) {
				getline(TXTFILE, line);
				bFound = false;
				for(unsigned i = 0; i < uniquestrings.size(); ++i) {
					if(uniquestrings[i] == line) {
						bFound = true;
						break;
					}
				}
				if(!bFound && line != "") {
					uniquestrings.push_back(line);
				}
			}
		}
		TXTFILE.close();

		FILE * pUniqueFiles;
		if(fopen_s(&pUniqueFiles, (g_strShaderPath + "\\uniquefilestocopy.txt").c_str(), "w") != 0) {
			std::cout << "ERROR: The file uniquefilestocopy.txt was not opened!\n";
			return EXIT_FAILURE;
		}
		for(unsigned i = 0; i < uniquestrings.size(); ++i) {
			fprintf(pUniqueFiles, "%s\n", uniquestrings[i].c_str());
		}
		fclose(pUniqueFiles);
	}
	else {
		std::cout << "Could not open filestocopy.txt!\n";
		return EXIT_FAILURE;
	}

	if(FileExists((g_strShaderPath + "\\filelistgen.txt").c_str(), false)) {
		if(!bDynamicCompile) {
			std::cout << "Generating action list...\n";
			if(!CopyFileA((g_strShaderPath + "\\filelistgen.txt").c_str(), (g_strShaderPath + "\\filelist.txt").c_str(), false)) {
				std::cout << "Src: " << (g_strShaderPath + "\\filelistgen.txt").c_str() << "\nDst: " << (g_strShaderPath + "\\filelist.txt").c_str() << "\n";
				std::cout << "CopyFile Failed! (" << GetLastError() << ")\n";
			}
		}
	}
	else {
		std::cout << "The file 'filelistgen.txt' does not exist!\n";
		return EXIT_FAILURE;
	}

	//todo: delete all txt files here, find out which ones shadercompile.exe actually needs.

	/*REM ****************
	REM Execute distributed process on work/build list
	REM ****************

	set shader_path_cd=%cd%
	if exist "filelist.txt" if exist "uniquefilestocopy.txt" if not "%dynamic_shaders%" == "1" (
	echo Running distributed shader compilation...

	cd /D %ChangeToDir%
	%shadercompilecommand% %SDKArgs% -shaderpath "%shader_path_cd:/=\%" -allowdebug
	cd /D %shader_path_cd%
	)*/
	if(FileExists((g_strShaderPath + "\\filelist.txt").c_str(), false) && FileExists((g_strShaderPath + "\\uniquefilestocopy.txt").c_str(), false) && !bDynamicCompile) {
		std::cout << "Running distributed shader compilation...\n";

		STARTUPINFOA startupinfo;
		PROCESS_INFORMATION processinfo;
		ZeroMemory(&startupinfo, sizeof(startupinfo));
		startupinfo.cb = sizeof(startupinfo);
		ZeroMemory(&processinfo, sizeof(processinfo));

		std::string str1 = "\"" + strSDKBinDir + "\\shadercompile.exe\" -nompi -nop4 -game \"" + g_strGameDir + "\" -shaderpath \"" + g_strShaderPath + "\" -allowdebug";
		std::vector<char> CmdLine(str1.begin(), str1.end()); //haha hacky.
		CmdLine.push_back('\0');
		std::cout << "shadercompile.exe Args: " << &CmdLine[0] << "\n";

		if(CreateProcessA((strSDKBinDir + "\\shadercompile.exe").c_str(), &CmdLine[0], NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, strSDKBinDir.c_str(), &startupinfo, &processinfo)) {
			WaitForSingleObject(processinfo.hProcess, INFINITE);
			DWORD ExitCode;
			GetExitCodeProcess(processinfo.hProcess, &ExitCode);
			std::cout << "OKAY: CreateProcess Ended, Return Code: " << ExitCode << "\n";
			CloseHandle(processinfo.hProcess);
			CloseHandle(processinfo.hThread);
		}
		else {
			std::cout << "ERROR: CreateProcess failed! Code: " << GetLastError() << "\n";
			return EXIT_FAILURE;
		}
	}

	/*REM ****************
	REM PC Shader copy
	REM Publish the generated files to the output dir using XCOPY
	REM This batch file may have been invoked standalone or slaved (master does final smart mirror copy)
	REM ****************
	:DoXCopy
	if not "%dynamic_shaders%" == "1" (
	if not exist "%targetdir%" md "%targetdir%"
	if not "%targetdir%"=="%shaderDir%" xcopy %shaderDir%\*.* "%targetdir%" /e /y
	)
	goto end*/
	system("pause");
	return EXIT_SUCCESS;
}