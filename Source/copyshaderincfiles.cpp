#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <io.h>
#include "copyshaderincfiles.h"
#include "helpers.h"
#include "main.h"


void copyshaderincfiles(std::string txtfilename, const bool x360) {
	/*my $txtfilename = shift;
	my $arg = shift;

	my $is360 = 0;
	my $platformextension = "";
	if( $arg =~ m/-x360/i )
	{
		$is360 = 1;
		$platformextension = ".360";
	}*/

	std::string platformextension = "";
	if(x360) {
		platformextension = ".360";
	}

	/*open TXTFILE, "<$txtfilename";

	my $src;
	my $dst;*/
	std::ifstream TXTFILE(txtfilename);

	if(TXTFILE.is_open()) {
		std::string line;
		std::string dst;
		while(TXTFILE.good()) {
			getline(TXTFILE, line);
			if(line.find("_tmp") != -1) {
				line.erase(0, 1);
				line.erase(line.end() - 1, line.end());
				dst = line;
				dst.erase(dst.find("_tmp"), 4);

				
				// Does the destination exist?
				//std::cout << "dst: ";
				bool dstexists = FileExists(dst, false);
				//std::cout << "src: ";
				bool srcexists = FileExists(line, false);
				if(!srcexists) {
					std::cout << "Source file does not exist: " << line << "\n";
					//getchar(); //todo: re-enable
					continue;
				}

				// What are the time stamps for the src and dst
				struct stat attrib;
				stat(line.c_str(), &attrib);
				time_t srcmodtime = attrib.st_mtime;
				stat(dst.c_str(), &attrib);
				time_t dstmodtime = attrib.st_mtime;

				if(!dstexists || (srcmodtime != dstmodtime)) {
					// Make the target writable if it exists
					if(dstexists) {
						//chmod to 777
						_chmod(dst.c_str(), (_S_IREAD | _S_IWRITE));
						SetFileAttributesA(dst.c_str(), FILE_ATTRIBUTE_NORMAL);
					}
					else {
						std::string dir = dst;			
						dir.erase(dir.find_last_of('\\'), dir.length());
						CreateDirectoryA(dir.c_str(), NULL);
						SetFileAttributesA(dir.c_str(), FILE_ATTRIBUTE_NORMAL);
					}

				}

				//todo: we want to see STDERR here if there is an error.
				if(!CopyFileA(line.c_str(), dst.c_str(), false)) {
					std::cout << "line: " << line << "\ndst: " << dst << "\n";
					std::cout << "CopyFile Failed! (" << GetLastError() << ")\n";
				}
				else {
					if(!SetFileAttributesA(dst.c_str(), FILE_ATTRIBUTE_READONLY)) {
						std::cout << "SetFileAttributes Failed! (" << GetLastError() << ")\n";
					}
				}
			}
		}
	}

	TXTFILE.close();
}