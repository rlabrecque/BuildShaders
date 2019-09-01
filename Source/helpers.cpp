#include <iostream>
#include <fstream>
//#include <sys/stat.h>
#include <io.h> // for _access
#include "helpers.h"
#include "main.h"

std::vector<std::string> LoadShaderListFile(const std::string filename) {
// 	my $inputbase = shift;
// 
// 	my @srcfiles;
// 	&MakeSureFileExists( "$inputbase.txt", 1, 0 );
	std::vector<std::string> srcfiles;
	if(FileExists(filename, true) == false) {
		std::cout << "Shader List File: \"" << filename << "\" does not exist\n";
		exit(EXIT_FAILURE);
	}

// 	open SHADERLISTFILE, "<$inputbase.txt" || die;
// 	my $line;
// 	while( $line = <SHADERLISTFILE> ) {
	std::ifstream shaderlistfile(filename);
	if(shaderlistfile.is_open()) {
		std::string line;
		while(shaderlistfile.good()) {
			getline(shaderlistfile, line);
// 			$line =~ s/\/\/.*$//;	# remove comments "//..."
// 			$line =~ s/^\s//;		# trim leading whitespace
// 			$line =~ s/\s*$//;		# trim trailing whitespace
// 			next if( $line =~ m/^\s*$/ );
			if(!line.empty()) {
				line.erase(0, line.find_first_not_of(" \t\f\v\n\r"));
				line.erase(line.find_last_not_of(" \t\f\v\n\r") + 1);
				if(line.substr(0, 2) != "//") {

// 					if( $line =~ m/\.fxc/ || $line =~ m/\.vsh/ || $line =~ m/\.psh/ ) {
// 					my $shaderbase = &LoadShaderListFile_GetShaderBase( $line );
					if(line.find(".fxc") != -1 || line.find(".vsh") != -1 || line.find(".psh") != -1) {
						std::string shaderbase = LoadShaderListFile_GetShaderBase(line);

// 						if( $ENV{"DIRECTX_FORCE_MODEL"} =~ m/^30$/i )	{# forcing all shaders to be ver. 30
// 							my $targetbase = $shaderbase;
// 							$targetbase =~ s/_ps2x/_ps30/i;
// 							$targetbase =~ s/_ps20b/_ps30/i;
// 							$targetbase =~ s/_ps20/_ps30/i;
// 							$targetbase =~ s/_vs20/_vs30/i;
// 							$targetbase =~ s/_vsxx/_vs30/i;
// 							push @srcfiles, ( $line . "-----" . $targetbase );
// 							} else {
						if(g_nDirectX_Force == 30) {
							std::string targetbase = shaderbase;
							if(targetbase.find("_ps2x") != -1) {
								size_t tmp = targetbase.find("_ps2x");
								targetbase.replace(tmp, tmp + 5, "_ps30");
							}
							else if(targetbase.find("_ps20") != -1) {
								size_t tmp = targetbase.find("_ps20");
								targetbase.replace(tmp, tmp + 5, "_ps30");
							}
							else if(targetbase.find("_ps20b") != -1) {
								size_t tmp = targetbase.find("_ps20b");
								targetbase.replace(tmp, tmp + 6, "_ps30");
							}
							else if(targetbase.find("_vs20") != -1) {
								size_t tmp = targetbase.find("_vs20");
								targetbase.replace(tmp, tmp + 5, "_vs30");
							}
							else if(targetbase.find("_vsxx") != -1) {
								size_t tmp = targetbase.find("_vsxx");
								targetbase.replace(tmp, tmp + 5, "_vs30");
							}
							//I think valve forgot _vs11

							srcfiles.push_back(line + "-----" + targetbase);
						}
						else {
// 							if( $shaderbase =~ m/_ps2x/i ) {
// 								my $targetbase = $shaderbase;
// 								$targetbase =~ s/_ps2x/_ps20/i;
// 								push @srcfiles, ( $line . "-----" . $targetbase );
// 
// 								$targetbase = $shaderbase;
// 								$targetbase =~ s/_ps2x/_ps20b/i;
// 								push @srcfiles, ( $line . "-----" . $targetbase );
// 							}
							if(shaderbase.find("_ps2x") != -1) {
								std::string targetbase;
								size_t tmp;

								targetbase = shaderbase;
								tmp = targetbase.find("_ps2x");

								targetbase.replace(tmp, tmp + 5, "_ps20");
								srcfiles.push_back(line + "-----" + targetbase);

								targetbase = shaderbase;
								targetbase.replace(tmp, tmp + 5, "_ps20");
								targetbase.push_back('b');
								srcfiles.push_back(line + "-----" + targetbase);
							}
// 							elsif( $shaderbase =~ m/_vsxx/i ) {
// 								my $targetbase = $shaderbase;
// 								$targetbase =~ s/_vsxx/_vs11/i;
// 								push @srcfiles, ( $line . "-----" . $targetbase );
// 
// 								$targetbase = $shaderbase;
// 								$targetbase =~ s/_vsxx/_vs20/i;
// 								push @srcfiles, ( $line . "-----" . $targetbase );
// 							} else {
// 								push @srcfiles, ( $line . "-----" . $shaderbase );
// 							}
							else if(shaderbase.find("_vsxx") != -1) {
								std::string targetbase;
								size_t tmp;

								targetbase = shaderbase;
								tmp = targetbase.find("_vsxx");

								targetbase.replace(tmp, tmp + 5, "_vs11");
								srcfiles.push_back(line + "-----" + targetbase);

								targetbase = shaderbase;
								targetbase.replace(tmp, tmp + 5, "_vs20");
								srcfiles.push_back(line + "-----" + targetbase);
							}
							else {
								srcfiles.push_back(line + "-----" + shaderbase);
							}
						}
					}
				}
			}
		}
// 		close SHADERLISTFILE;
// 		return @srcfiles;
// 		}
		shaderlistfile.close();
	}
	else {
		std::cout << "Could not open Shader List File: \"" << filename << "\"\n";
		exit(EXIT_FAILURE);
	}

	return srcfiles;
}

std::string LoadShaderListFile_GetShaderBase(std::string shadername) {
	if(shadername.find("-----") != -1) {
		return shadername.substr(shadername.find("-----") + 5); //find a new shader name ex: SDK_Shadername.psh-----SDK_NewShadername
	}
	else {
		std::string shadertype = LoadShaderListFile_GetShaderType(shadername);
		assert(shadertype != "");
		shadername.erase(shadername.find("." + shadertype), shadername.length());
		return shadername;
	}
}

std::string LoadShaderListFile_GetShaderType(const std::string shadername) {
	if (shadername.find(".vsh") != -1) {
		return "vsh";
	}
	else if(shadername.find(".psh") != -1) {
		return "psh";
	}
	else if(shadername.find(".fxc") != -1) {
		return "fxc";
	}
	return "";
}

std::string LoadShaderListFile_GetShaderSrc(const std::string shadername) {
	if(shadername.find("-----") != -1) {
		return shadername.substr(0, shadername.find("-----"));
	}
	else {
		return shadername;
	}
}

bool FileExists(const std::string filename, const bool testwrite) {
	//struct stat stFileInfo;
	//int Stat = stat(filename.c_str(), &stFileInfo);
	std::ifstream file(filename);
	if(file.is_open()) {
		if(testwrite) {
			if (_access(filename.c_str(), 2) == -1) {
				std::cout << "File " << filename << " is not writable!\n";
				return false;
			}
		}

		//std::cout << "File " << filename << " exists!\n"; //todo: verbose
		return true;
	}
	else {
		//std::cout << "File " << filename << " does not exists! (" << Stat << ")\n";
		return false;
	}
}

bool DirectoryExists(const std::string filename) {
	DWORD dwAttrib = GetFileAttributesA(filename.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}