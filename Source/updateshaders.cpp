#include "updateshaders.h"
#include <iostream>
#include <fstream>
#include <io.h>
#include <sstream>
#include "main.h"
#include "helpers.h"

void updateshaders(std::string filename, const bool x360) {
	std::cout << "--------------------------------\n" << "Update Shaders\n" << filename << '\n' << "--------------------------------\n";
	std::string tmpfolder = "_tmp";
	std::string vcsext = ".vcs";
	if(x360) {
		tmpfolder = "_360_tmp";
		vcsext = ".360.vcs";
	}
	
	/*if(srcdir == "") { //todo: ????
		//Do not use SourceDir
	}*/
	
	std::vector<std::string> srcfiles = LoadShaderListFile(filename);

	FILE* makefile;
	FILE* copyfile;
	FILE* inclist;
	FILE* vcslist;

	//std::string tmp = "makefile." + filename;
	std::string tmp = filename + ".makefile";
	if(fopen_s(&makefile, tmp.c_str(), "w") != 0) {
		std::cout << "The makefile was not opened! \"" << tmp << "\"\n";
		exit(EXIT_FAILURE);
	}
	tmp.append(".copy");
	if(fopen_s(&copyfile, tmp.c_str(), "w") != 0) {
		std::cout << "The makefile copy was not opened! \"" << tmp << "\"\n";
		exit(EXIT_FAILURE);
	}
	if(fopen_s(&inclist, (g_strShaderPath + "\\inclist.txt").c_str(), "w") != 0) {
		std::cout << "The inclist was not opened! \"" << (g_strShaderPath + "\\inclist.txt").c_str() << "\"\n";
		exit(EXIT_FAILURE);
	}
	if(fopen_s(&vcslist, (g_strShaderPath + "\\vcslist.txt").c_str(), "w") != 0) {
		std::cout << "The vcslist was not opened! \"" << (g_strShaderPath + "\\inclist.txt").c_str() << "\"\n";
		exit(EXIT_FAILURE);
	}

	// make a default dependency that depends on all of the shaders.
	fputs("default: ", makefile);

	for(unsigned shader = 0; shader < srcfiles.size(); ++shader) {
		
		std::string shadertype = LoadShaderListFile_GetShaderType(srcfiles[shader]);
		std::string shaderbase = LoadShaderListFile_GetShaderBase(srcfiles[shader]);
		std::string shadersrc = LoadShaderListFile_GetShaderSrc(srcfiles[shader]);

		if(shadertype == "fxc" || shadertype == "vsh") {
			//std::string incFileName = " " + shadertype + "tmp9" + tmpfolder + "\\" + shaderbase + ".inc";
			std::string tmppath = g_strShaderPath;
			tmppath.erase(0, 3);
			std::string incFileName = "\"" + g_strShaderPath + '\\' + shadertype + "tmp9" + tmpfolder + "\\" + shaderbase + ".inc\"";
			fputs((" " + incFileName).c_str(), makefile);
			//incFileName.erase(0,1);
			incFileName.append("\n");
			fputs(incFileName.c_str(), inclist);
		}

		std::string vcsfile = shaderbase + vcsext;

		bool compilevcs = true;
		if(shadertype == "fxc" && g_bDynamicCompile) {
			compilevcs = false;
		}
		if(x360 && shaderbase.find("_ps20") != -1) {
			compilevcs = false;
		}
		/*if( $compilevcs )
		{
		my $vcsFileName = "..\\..\\..\\game\\hl2\\shaders\\$shadertype\\$shaderbase" . $g_vcsext;
		# We want to check for perforce operations even if the crc matches in the event that a file has been manually reverted and needs to be checked out again.
		&output_vcslist_line( "$vcsFileName\n" );  
		$shadercrcpass{$shader} = &CheckCRCAgainstTarget( $shadersrc, $vcsFileName, 0 );
		  if( $shadercrcpass{$shader} )
		  {
		  $compilevcs = 0;
		  }
		}*/
		if(compilevcs) {
			std::string vcsFileName = g_strGameDir + "\\shaders\\" + shadertype + "\\" + shaderbase + vcsext + "\n";
			// We want to check for perforce operations even if the crc matches in the event that a file
			// has been manually reverted and needs to be checked out again.
			
			fputs(vcsFileName.c_str(), vcslist);
			//CheckCRCAgainstTarget //todo: we do no CRC checking yet
			/*if(shadercrcpass) {
			compilevcs = false
			}*/
		}

		if(compilevcs) {
			//std::string tmppath = szShaderPath;
			//tmppath.erase(0, 3);
			//std::string tmp = " \"..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\..\\" + tmppath + "\\shaders\\" + shadertype + "\\" + vcsfile + "\"";
			//fputs(tmp.c_str(), makefile); //todo: causes each part of the makefile to run twice
			// emit a list of vcs files to copy to the target since we want to build them.
			tmp = shadersrc + "-----" + shaderbase + "\n";
			fputs(tmp.c_str(), copyfile);
		}
	}

	fputs("\n\n", makefile);
	
	//Insert all of our vertex shaders and depencencies
	std::string lastshader = "";
	std::vector<std::string> dep;
	for(unsigned shader = 0; shader < srcfiles.size(); ++shader) {
		std::string shadername = LoadShaderListFile_GetShaderSrc(srcfiles[shader]);
		if(lastshader != shadername) {
			lastshader = shadername;
			dep.clear();
			dep = GetAsmShaderDependencies_R(lastshader, dep);
			//for(unsigned dependancy = 0; dependancy < dep.size(); ++dependancy) {
			//	std::cout << lastshader << " depends on " << dep[dependancy] << "\n";
			//}
		}
		//DoAsmShader( shader );
		std::string shaderbase = LoadShaderListFile_GetShaderBase(srcfiles[shader]);
		std::string shadertype = LoadShaderListFile_GetShaderType(srcfiles[shader]);
		std::string incfile;

		if(shadertype == "fxc" || shadertype == "vsh") {
			//incfile =  szShaderPath + '\\' + shadertype + "tmp9" + tmpfolder + "\\" + shaderbase + ".inc ";
			std::string tmppath = g_strShaderPath;
			tmppath.erase(0, 3);
			incfile = "\"" + g_strShaderPath + '\\' + shadertype + "tmp9" + tmpfolder + "\\" + shaderbase + ".inc\" ";
		}
		
		std::string vcsfile = shaderbase + vcsext;
		bool bWillCompileVcs = true;
		if (shadertype == "fxc" && g_bDynamicCompile)	{
			bWillCompileVcs = false;
		}
		//if(shadercrcpass(shader)) { bWillCompileVcs = false; } //todo:

		if(bWillCompileVcs)	{
			std::string tmppath = g_strShaderPath;
			tmppath.erase(0, 3);
			std::string tmp = incfile + "\"" + g_strShaderPath + "\\shaders\\" + shadertype + "\\" + vcsfile + "\": " + "\"" + g_strShaderPath + '\\' + shadername + '\"';
			for(unsigned dependancy = 0; dependancy < dep.size(); ++dependancy) {
				tmp.append((" \"" + g_strShaderPath + '\\' + dep[dependancy] + '\"').c_str());
			}
			tmp.append("\n");
			fputs(tmp.c_str(), makefile);
		}
		else
		{
			// psh files don't need a rule at this point since they don't have inc files and we aren't compiling a vcs.
			if(shadertype == "fxc" || shadertype == "vsh") {
				//&output_makefile_line( $incfile . ":  $shadername @dep\n") ;
				std::string tmppath = g_strShaderPath;
				tmppath.erase(0, 3);
				std::string tmp = incfile + ": " + "\"" + g_strShaderPath + '\\' + shadername + '\"';
				for(unsigned dependancy = 0; dependancy < dep.size(); ++dependancy) {
					tmp.append((" \"" + g_strShaderPath + '\\' + dep[dependancy] + '\"').c_str());
				}
				tmp.append("\n");
				fputs(tmp.c_str(), makefile);
			}
		}

		std::string x360switch = "";
		std::string moreswitches = "";

		if(!bWillCompileVcs && shadertype == "fxc")	{
			moreswitches = "-novcs ";
		}
		if(x360) {
			x360switch = "-x360 ";

			if(bWillCompileVcs && shaderbase.find("_ps20") != -1) {
				moreswitches = "-novcs ";
				bWillCompileVcs = false;
			}
		}

		// if we are psh and we are compiling the vcs, we don't need this rule.
		if(!(shadertype == "psh" && !bWillCompileVcs)) {
			char tmpmodulename[MAX_PATH];
			GetModuleFileNameA(NULL, tmpmodulename, MAX_PATH);
			std::string ModuleFileName = tmpmodulename;
			fputs(("\t\"" + ModuleFileName + "\" -" + shadertype + "_prep " + moreswitches + x360switch + "-shader \"" + srcfiles[shader] + "\" -shaderdir \"" + g_strShaderPath + "\"\n").c_str(), makefile);
		}
		if(bWillCompileVcs) {
			// shadername relative to the Shaderlist's path.
			fputs(("\techo " + g_strShaderPath + "\\" + shadername + ">> \"" + g_strShaderPath + "\\filestocopy.txt\"\n").c_str(), makefile);
			for(unsigned dependancy = 0; dependancy < dep.size(); ++dependancy) {
				fputs(("\techo " + g_strShaderPath + "\\" + dep[dependancy] + ">> \"" + g_strShaderPath + "\\filestocopy.txt\"\n").c_str(), makefile);
			}
		}
		fputs("\n", makefile);
	}

	fclose(makefile);
	fclose(copyfile);
	fclose(inclist);
	fclose(vcslist);
}

std::vector<std::string> GetAsmShaderDependencies_R(std::string shadername, std::vector<std::string> dep) {
	std::string tmp = g_strShaderPath + '\\' + shadername;
	std::ifstream SHADER(tmp);

	if(SHADER.is_open()) {
		std::string line;
		bool bfound = false;
		while(SHADER.good()) {
			getline(SHADER, line);
			if(!line.empty() && line.find("#include") != -1 && line.find('"') != -1 && line.substr(line.find_first_not_of(" \t\f\v\n\r"), line.find_first_not_of(" \t\f\v\n\r") + 2) != "//") { //todo: check if this is correct again...
				std::string tmp2 = line.substr(line.find_first_of('"') + 1, line.find_last_of('"') - (line.find_first_of('"') + 1));
				bfound = false;
				for(unsigned dependancy = 0; dependancy < dep.size(); ++dependancy) {
					if(tmp2 == dep[dependancy]) {
						bfound = true;
					}
				}
				if(bfound == false) {
					dep.push_back(tmp2);
					dep = GetAsmShaderDependencies_R(tmp2, dep);
				}
			}
		}
	}
	SHADER.close();
	return dep;
}