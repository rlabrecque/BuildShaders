#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include "fxc_prep.h"
#include "helpers.h"
#include "main.h"

//std::string szShaderPath;

void fxc_prep(std::string shader, bool nvidia, bool ps2a, bool x360, bool produceCompiledVcs, bool produceCppClasses, const std::string shaderpath) {
	//$generateListingFile = 0;
	//$spewCombos = 0;
	//$g_produceCppClasses = 1;
	//$g_produceCompiledVcs = 1;
	//bool generateListingFile = false;
	bool spewCombos = false;

	std::cout << "Running fxc_prep on \"" << shaderpath << "\" \"" << shader << '\n';

	if(shaderpath == "" || !DirectoryExists(shaderpath)) {
		std::cout << "You require a valid shader path! \"" + shaderpath + "\"\n";
		exit(EXIT_FAILURE);
	}
	if(shader == "") {
		std::cout << "You require a valid shader! " + shader + "\n";
		exit(EXIT_FAILURE);
	}

	std::cout << "fxc_prep running on " << shader << "\n";
	
	/*$argstring = $fxc_filename;
	$fxc_basename = $fxc_filename;
	$fxc_basename =~ s/^.*-----//; //DF; string replaces everything before and including the dashes.
	$fxc_filename =~ s/-----.*$//;*/

	std::string fxc_filename = shader.substr(0, shader.find("-----"));
	std::string fxc_basename = shader.substr(shader.find("-----") + 5, shader.size());


	//$debug = 0;
	//$forcehalf = 0;
	//#ifdef _DEBUG
	//bool debug = true;
	//#else
	bool debug = false;
	//#endif // _DEBUG
	//bool forcehalf = false;

	/*if ( $g_x360 )
	{
		$fxctmp = "fxctmp9_360_tmp";
	}
	else
	{
		$fxctmp = "fxctmp9_tmp";
	}*/

	std::string fxctmp;

	if(x360) {
		fxctmp = "fxctmp9_360_tmp";
	}
	else {
		fxctmp = "fxctmp9_tmp";
	}

	/*if( !stat $fxctmp )
	{
		mkdir $fxctmp, 0777 || die $!;
	}*/

	if(!FileExists(shaderpath + '\\' + fxctmp, false)) {
		CreateDirectoryA((shaderpath + '\\' + fxctmp).c_str(), NULL);
	}

	//@fxc = ReadInputFile( $fxc_filename );
	std::vector<std::string> fxc = ReadInputFile(shaderpath, fxc_filename);

	std::vector<std::string> staticDefineNames;
	std::vector<int> staticDefineMin;
	std::vector<int> staticDefineMax;
	std::map<std::string, std::string> staticDefineInit;

	std::vector<std::string> dynamicDefineNames;
	std::vector<int> dynamicDefineMin;
	std::vector<int> dynamicDefineMax;

	std::string perlskipcode;
	std::vector<std::string> perlskipcodeindividual;

	std::map<std::string, unsigned> centroidEnable;
	int centroidMask = 0;

	std::string outputHeader;

	/*# READ THE TOP OF THE FILE TO FIND SHADER COMBOS
	foreach $line ( @fxc )*/
	for(unsigned line = 0; line < fxc.size(); ++line) {
		//$line="" if ($g_x360 && ($line=~/\[PC\]/));										# line marked as [PC] when building for x360
		//$line="" if (($g_x360 == 0)  && ($line=~/\[XBOX\]/));							# line marked as [XBOX] when building for pc
		if(x360 && fxc[line].find("[PC]") != -1) { //line marked as [PC] when building for x360
			fxc[line] = "";
		}
		if(!x360 && fxc[line].find("[XBOX]") != -1) { //line marked as [XBOX] when building for pc
			fxc[line] = "";
		}
		/*if ( $fxc_basename =~ m/_ps(\d+\w?)$/i )
		{
			my $psver = $1;
			$line="" if (($line =~/\[ps\d+\w?\]/i) && ($line!~/\[ps$psver\]/i));	# line marked for a version of compiler and not what we build
		}
		if ( $fxc_basename =~ m/_vs(\d+\w?)$/i )
		{
			my $vsver = $1;
			$line="" if (($line =~/\[vs\d+\w?\]/i) && ($line!~/\[vs$vsver\]/i));	# line marked for a version of compiler and not what we build
		}*/
		if(fxc_basename.find("_ps") != -1) { //todo: turn this into a function accept "ps"/"vs" return a bool.
			std::string psver = fxc_basename.substr(fxc_basename.find("_ps") + 1, fxc_basename.size());

			std::vector<std::string> vecvar;
			int startpos = 0;
			bool bFoundSMvar = false;

			while(startpos != -1) {
				if(fxc[line].find("[ps", startpos) != -1) {
					std::string psvartmp;
					bFoundSMvar = true;
					//std::cout << "line: " << fxc[line] << "\n";
					for(unsigned i = fxc[line].find("[ps", startpos) + 1; i < fxc[line].size(); ++i) {
						if(fxc[line].at(i) == ']') {
							break;
						}
						psvartmp.push_back(fxc[line].at(i));
					}
					vecvar.push_back(psvartmp);
					startpos = fxc[line].find("[ps", startpos) + 3;
				}
				else {
					startpos = -1;
				}
			}

			bool bFound = false;
			for(unsigned linevariable = 0; linevariable < vecvar.size(); ++linevariable) {
				//std::cout << linevariable << " Does " << psver << " equal to " << vecvar[linevariable] << "?\n";
				if(psver == vecvar[linevariable]) {
					bFound = true;
					break;
				}
			}

			if(bFoundSMvar && !bFound) {
				fxc[line] = "";
			}
		}
		if(fxc_basename.find("_vs") != -1) {
			std::string psver = fxc_basename.substr(fxc_basename.find("_vs") + 1, fxc_basename.size());

			std::vector<std::string> vecvar;
			int startpos = 0;
			bool bFoundSMvar = false;

			while(startpos != -1) {
				if(fxc[line].find("[vs", startpos) != -1) {
					std::string vsvartmp;
					bFoundSMvar = true;
					//std::cout << "line: " << fxc[line] << "\n";
					for(unsigned i = fxc[line].find("[vs", startpos) + 1; i < fxc[line].size(); ++i) {
						if(fxc[line].at(i) == ']') {
							break;
						}
						vsvartmp.push_back(fxc[line].at(i));
					}
					vecvar.push_back(vsvartmp);
					startpos = fxc[line].find("[vs", startpos) + 3;
				}
				else {
					startpos = -1;
				}
			}

			bool bFound = false;
			for(unsigned linevariable = 0; linevariable < vecvar.size(); ++linevariable) {
				//std::cout << linevariable << " Does " << psver << " equal to " << vecvar[linevariable] << "?\n";
				if(psver == vecvar[linevariable]) {
					bFound = true;
					break;
				}
			}

			if(bFoundSMvar && !bFound) {
				fxc[line] = "";
			}
		}

		/*my $init_expr;
		$init_expr = $1 if ( $line=~/\[\=([^\]]+)\]/);	# parse default init expression for combos*/
		std::string init_expr;
		if(fxc[line].find("[= ") != -1) {
			for(unsigned i = fxc[line].find("[= ") + 3; i < fxc[line].size(); ++i) { //this might not leave a space in front unlike the perl equivilent
				if(fxc[line].at(i) == ']') {
					break;
				}
				init_expr.push_back(fxc[line].at(i));
			}
		}

		/*	$line=~s/\[[^\[\]]*\]//;
		# cut out all occurrences of
		# square brackets and whatever is
		# inside all these modifications
		# to the line are seen later when
		# processing skips and centroids*/
		if(fxc[line].find('[') != -1) { //this is pretty dumb, removes legitimate entries.
			while(fxc[line].find('[') != -1) {
				fxc[line].erase(fxc[line].find_first_of('['), fxc[line].find_first_of(']') - fxc[line].find_first_of('[') + 1);
			}
		}

		//next if( $line =~ m/^\s*$/ );
		if(fxc[line].find_first_not_of(" \t\v\n\r\f") == -1) {	//this is wrong. It should ignore any line that starts with whitespace.
			continue;											//If Find first of() is iterator 0/1 in the string.
		}

		/*if( $line =~ m/^\s*\/\/\s*STATIC\s*\:\s*\"(.*)\"\s+\"(\d+)\.\.(\d+)\"/ )
		{
			local( $name, $min, $max );
			$name = $1;
			$min = $2;
			$max = $3;
# print STDERR "STATIC: \"$name\" \"$min..$max\"\n";
			push @staticDefineNames, $name;
			push @staticDefineMin, $min;
			push @staticDefineMax, $max;
			$staticDefineInit{$name}=$init_expr;
		}*/
		if(fxc[line].find("//") != -1 && fxc[line].find("STATIC:") != -1) {
			std::string name;
			std::string min;
			std::string max;
			for(unsigned i = fxc[line].find_first_of('"') + 1; i < fxc[line].size(); ++i) { //this might not leave a space in front unlike the perl equivilent
				if(name == "" && fxc[line].at(i) == '"') {
					name = fxc[line].substr(fxc[line].find_first_of('"') + 1, i - fxc[line].find_first_of('"') - 1);
					continue;
				}
				if(min == "" && fxc[line].at(i) == '"') {
					min = fxc[line].substr(i + 1, fxc[line].find("..") - i - 1);
					continue;
				}
				if(max == "" && fxc[line].at(i) == '.') {
					max = fxc[line].substr(i + 2, fxc[line].find_last_of('"') - i - 2);
					break;
				}
			}
			if(name != "" && min != "" && max != "") {
				staticDefineNames.push_back(name);
				staticDefineMin.push_back(atoi(min.c_str()));
				staticDefineMax.push_back(atoi(max.c_str()));
				std::pair<std::map<std::string, std::string>::iterator, bool> ret;
				ret = staticDefineInit.insert(std::pair<std::string, std::string>(name, init_expr));
				if(ret.second == false) {
					std::cout << "Element " << name << " already existed with a value of \"" << ret.first->second << "\".\n";
					//return 0; //todo:
				}
			}
			else {
				std::cout << "// STATIC: \"" << name << "\" \"" << min << ".." << max << "\"\n";
				std::cout << "Your shit is all kinds fucked up!\n"; //todo: proper error message.
				exit(EXIT_FAILURE);
			}
		}
		/*	elsif( $line =~ m/^\s*\/\/\s*DYNAMIC\s*\:\s*\"(.*)\"\s+\"(\d+)\.\.(\d+)\"/ )
		{
		local( $name, $min, $max );
		$name = $1;
		$min = $2;
		$max = $3;
		# print STDERR "DYNAMIC: \"$name\" \"$min..$max\"\n";
		push @dynamicDefineNames, $name;
		push @dynamicDefineMin, $min;
		push @dynamicDefineMax, $max;
		}*/
		else if(fxc[line].find("//") != -1 && fxc[line].find("DYNAMIC:") != -1) {
			std::string name;
			std::string min;
			std::string max;
			for(unsigned i = fxc[line].find_first_of('"') + 1; i < fxc[line].size(); ++i) { //this might not leave a space in front unlike the perl equivilent
				if(name == "" && fxc[line].at(i) == '"') {
					name = fxc[line].substr(fxc[line].find_first_of('"') + 1, i - fxc[line].find_first_of('"') - 1);
					continue;
				}
				if(min == "" && fxc[line].at(i) == '"') {
					min = fxc[line].substr(i + 1, fxc[line].find("..") - i - 1);
					continue;
				}
				if(max == "" && fxc[line].at(i) == '.') {
					max = fxc[line].substr(i + 2, fxc[line].find_last_of('"') - i - 2);
					break;
				}
			}
			if(name != "" && min != "" && max != "") {
				dynamicDefineNames.push_back(name);
				dynamicDefineMin.push_back(atoi(min.c_str()));
				dynamicDefineMax.push_back(atoi(max.c_str()));
			}
			else {
				std::cout << "// DYNAMIC: \"" << name << "\" \"" << min << ".." << max << "\"\n";
				std::cout << "Your shit is all fucked up!\n"; //todo: proper error message.
				exit(EXIT_FAILURE);
			}
		}
	}

	/*# READ THE WHOLE FILE AND FIND SKIP STATEMENTS
	foreach $line ( @fxc )
	{
		if( $line =~ m/^\s*\/\/\s*SKIP\s*\s*\:\s*(.*)$/ )
		{
	#		print $1 . "\n";
			$perlskipcode .= "(" . $1 . ")||";
			push @perlskipcodeindividual, $1;
		}
	}
	# READ THE WHOLE FILE AND FIND CENTROID STATEMENTS
	foreach $line ( @fxc )
	{
		if( $line =~ m/^\s*\/\/\s*CENTROID\s*\:\s*TEXCOORD(\d+)\s*$/ )
		{
			$centroidEnable{$1} = 1;
#		print "CENTROID: $1\n";
		}
	}*/
	// READ THE WHOLE FILE AND FIND SKIP STATEMENTS
	//READ THE WHOLE FILE AND FIND CENTROID STATEMENTS
	for(unsigned line = 0; line < fxc.size(); ++line) {
		if(fxc[line].find("//") != -1) {
			if(fxc[line].find("SKIP: ") != -1 && fxc[line].find("NOSKIP:") == -1) {
				std::string pooptmp = fxc[line].substr(fxc[line].find("SKIP: ") + 6, fxc[line].length() - (fxc[line].find("SKIP:") + 6));
				perlskipcode.append(("(" + pooptmp + ")||").c_str());
				perlskipcodeindividual.push_back(pooptmp);
				//std::cout << "\"" << fxc[line] << "\"\n";
			}
			if(fxc[line].find("CENTROID:") != -1) {
				std::string centroid = (fxc[line].substr(fxc[line].find("CENTROID: ") + 10, fxc[line].length() - (fxc[line].find("CENTROID: ") + 10)));
				std::pair<std::map<std::string, unsigned>::iterator, bool> ret;
				ret = centroidEnable.insert(std::pair<std::string, unsigned>(centroid, 1));
				if(ret.second == false) {
					std::cout << "Centroid element " << centroid << " already existed with a value of \"" << ret.first->second << "\".\n";
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	/*if( defined $perlskipcode )
	{
	$perlskipcode .= "0";
	$perlskipcode =~ s/\n//g;
	}
	else
	{
	$perlskipcode = "0";
	}*/
	if(perlskipcode != "") {
		perlskipcode.push_back('0');
		perlskipcode.erase(std::remove(perlskipcode.begin(), perlskipcode.end(), '\n'), perlskipcode.end());

	}
	else {
		perlskipcode = "0";
	}

	/*if( $spewCombos )
	{
		push @outputHeader, "#include \"windows.h\"\n";
	}*/

	if(spewCombos) {
		outputHeader.append("#include \"windows.h\"\n");
	}

	/*# Go ahead an compute the mask of samplers that need to be centroid sampled
	$centroidMask = 0;
	foreach $centroidRegNum ( keys( %centroidEnable ) )
	{
	#	print "THING: $samplerName $centroidRegNum\n";
	$centroidMask += 1 << $centroidRegNum;
	}*/
	//Go ahead an compute the mask of samplers that need to be centroid sampled
	for(std::map<std::string, unsigned>::iterator it = centroidEnable.begin(); it != centroidEnable.end(); ++it) {
		centroidMask += 1 << it->second;
	}

	//$numCombos = &CalcNumCombos();
	/*sub CalcNumCombos
	{
		local( $i, $numCombos );
		$numCombos = 1;
		for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
		{
			$numCombos *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
		}
		for( $i = 0; $i < scalar( @staticDefineNames ); $i++ )
		{
			$numCombos *= $staticDefineMax[$i] - $staticDefineMin[$i] + 1;
		}
		return $numCombos;
	}*/
	int numCombos = 1;
	for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
		numCombos *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
	}
	for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
		numCombos *= staticDefineMax[i] - staticDefineMin[i] + 1;
	}

	if(produceCompiledVcs && !g_bDynamicCompile) {
		//open FOUT, ">>filelistgen.txt" || die "can't open filelistgen.txt";
		FILE *FOUT;

		if(fopen_s(&FOUT, (shaderpath + "\\filelistgen.txt").c_str(), "a") != 0) {
			std::cout << "The filelistegen was not opened! \"" << g_strShaderPath << "\\filelistgen.txt\"\n";
			exit(EXIT_FAILURE);
		}

		/*print FOUT "**** generated by fxc_prep.pl ****\n";
		print FOUT "#BEGIN " . $fxc_basename . "\n";
		print FOUT "$fxc_filename" . "\n";
		print FOUT "#DEFINES-D:\n";*/
		fprintf(FOUT, "**** generated by fxc_prep.pl ****\n");
		fprintf(FOUT, "#BEGIN %s\n%s\n", fxc_basename.c_str(), fxc_filename.c_str());
		fprintf(FOUT, "#DEFINES-D:\n");

		/*	for( $i = 0; $i < scalar( @dynamicDefineNames ); \$i++ )
		{
		print FOUT "$dynamicDefineNames[$i]=";
		print FOUT $dynamicDefineMin[$i];
		print FOUT "..";
		print FOUT $dynamicDefineMax[$i];
		print FOUT "\n";
		}*/
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			fprintf(FOUT, "%s=%d..%d\n", dynamicDefineNames[i].c_str(), dynamicDefineMin[i], dynamicDefineMax[i]);
		}
		//print FOUT "#DEFINES-S:\n";
		fprintf(FOUT, "#DEFINES-S:\n");
		/*for( $i = 0; $i < scalar( @staticDefineNames ); \$i++ )
		{
			print FOUT "$staticDefineNames[$i]=";
			print FOUT $staticDefineMin[$i];
			print FOUT "..";
			print FOUT $staticDefineMax[$i];
			print FOUT "\n";
		}*/
		for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
			fprintf(FOUT, "%s=%d..%d\n", staticDefineNames[i].c_str(), staticDefineMin[i], staticDefineMax[i]);
		}

		/*	print FOUT "#SKIP:\n";
		print FOUT "$perlskipcode\n";
		print FOUT "#COMMAND:\n";*/
		fprintf(FOUT, "#SKIP:\n%s\n#COMMAND:\n", perlskipcode.c_str());

		//#first line
		/*print FOUT "fxc.exe ";
		print FOUT "/DTOTALSHADERCOMBOS=$numCombos ";
		print FOUT "/DCENTROIDMASK=$centroidMask ";
		print FOUT "/DNUMDYNAMICCOMBOS=" . &CalcNumDynamicCombos() . " ";
		print FOUT "/DFLAGS=0x0 "; # Nothing here for now.
		print FOUT "\n";*/
		/*$numCombos = 1;
		for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
		{
		$numCombos *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
		}*/
		int NumDynamicCombos = 1;
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			NumDynamicCombos *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
		}
		fprintf(FOUT, "fxc.exe /DTOTALSHADERCOMBOS=%d /DCENTROIDMASK=%d /DNUMDYNAMICCOMBOS=%d /DFLAGS=0x0 \n", numCombos, centroidMask, NumDynamicCombos);

		//#defines go here
		//#second line
		/*	print FOUT &RenameMain( $fxc_filename, $i );
		print FOUT "/T" . &GetShaderType( $fxc_filename ) . " ";
		print FOUT "/DSHADER_MODEL_" . &ToUpper( &GetShaderType( $fxc_filename ) ) . "=1 ";*/
		std::string loltmp = GetShaderType(fxc_basename, debug, ps2a);  // Valve passes fxc_filename but the function does not use it, opting for fxc_basename instead.
		std::string fuckinglol = loltmp;
		std::transform(fuckinglol.begin(), fuckinglol.end(), fuckinglol.begin(), ::toupper);
		fprintf(FOUT, "/Dmain=main /Emain /T%s /DSHADER_MODEL_%s=1 ", loltmp.c_str(), fuckinglol.c_str());
		/*if( $nvidia )
		{
			print FOUT "/DNV3X=1 "; # enable NV3X codepath
		}
		if ( $g_x360 )
		{
			print FOUT "/D_X360=1 "; # shaders can identify X360 centric code
# print FOUT "/Xbe:2- "; # use the less-broken old back end
		}
		if( $debug )
		{
			print FOUT "/Od "; # disable optimizations
				print FOUT "/Zi "; # enable debug info
		}*/
		if(nvidia) {
			fprintf(FOUT, "/DNV3X=1 "); //enable NV3X codepath
		}
		if(x360) {
			fprintf(FOUT, "/D_X360=1 "); //shaders can identify X360 centric code.
		}
		if(debug) {
			fprintf(FOUT, "/Od /Zi "); //Disable Optimization (Od), Enable Debug Info (Zi)
		}
		/*	print FOUT "/nologo ";
		#	print FOUT "/Fhtmpshader.h ";
		print FOUT "/Foshader.o ";
		print FOUT "$fxc_filename";
		print FOUT ">output.txt 2>&1";
		print FOUT "\n";
		#end of command line
		print FOUT "#END\n";
		print FOUT "**** end ****\n";*/
		fprintf(FOUT, "/nologo /Foshader.o %s>output.txt 2>&1\n#END\n**** end ****\n", fxc_filename.c_str());

		fclose(FOUT);
	}

	/*if ( $g_produceCppClasses ) {
# Write out the C++ helper class for picking shader combos
		&WriteStaticHelperClasses();
		&WriteDynamicHelperClasses();
		my $incfilename = "$fxctmp\\$fxc_basename" . ".inc";
		&WriteFile( $incfilename, join( "", @outputHeader ) );
	}*/
	if(produceCppClasses) {
		//Write out the C++ helper class for picking shader combos

		//-------------------------------//
		//Start of Static Helper Classes
		//-------------------------------//
		
		/*sub WriteStaticHelperClasses {
		local( $basename ) = $fxc_basename;
		$basename =~ tr/A-Z/a-z/;
		local( $classname ) = $basename . "_Static_Index";
		push @outputHeader, "#include \"shaderlib/cshader.h\"\n";
		push @outputHeader, "class $classname\n";
		push @outputHeader, "{\n";*/
		std::string basename = fxc_basename;
		std::transform(basename.begin(), basename.end(), basename.begin(), ::tolower);
		std::string classname = basename + "_Static_Index";
		outputHeader.append("#include \"shaderlib/cshader.h\"\n");
		outputHeader.append(("class " + classname + "\n").c_str());
		outputHeader.append("{\n");

		/*for( $i = 0; $i < scalar( @staticDefineNames ); $i++ ) {
			$name = $staticDefineNames[$i];
			$min = $staticDefineMin[$i];
			$max = $staticDefineMax[$i];
			&WriteHelperVar( $name, $min, $max );
		}*/
		for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
			outputHeader.append(WriteHelperVar(staticDefineNames[i], staticDefineMin[i], staticDefineMax[i]));
		}

		/*	push @outputHeader, "public:\n";
		#	push @outputHeader, "void SetShaderIndex( IShaderShadow *pShaderShadow ) { pShaderShadow->SetPixelShaderIndex( GetIndex() ); }\n";
		push @outputHeader, "\t$classname( )\n";
		push @outputHeader, "\t{\n";*/
		outputHeader.append("public:\n");
		outputHeader.append(("\t" + classname + "( )\n").c_str());
		outputHeader.append("\t{\n");
		
		/*	for( $i = 0; $i < scalar( @staticDefineNames ); $i++ ) {
		local( $name ) = @staticDefineNames[$i];
		local( $boolname ) = "m_b" . $name;
		local( $varname ) = "m_n" . $name;
		if ( length( $staticDefineInit{$name} ) ) {
		push @outputHeader, "#ifdef _DEBUG\n";
		push @outputHeader, "\t\t$boolname = true;\n";
		push @outputHeader, "#endif // _DEBUG\n";
		push @outputHeader, "\t\t$varname = $staticDefineInit{$name};\n";
		}
		else {
		push @outputHeader, "#ifdef _DEBUG\n";
		push @outputHeader, "\t\t$boolname = false;\n";
		push @outputHeader, "#endif // _DEBUG\n";
		push @outputHeader, "\t\t$varname = 0;\n";
		}
		}*/
		for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
			std::string name = staticDefineNames[i];
			std::string boolname = "m_b" + name;
			std::string varname = "m_n" + name;
			if(staticDefineInit[name] != "") { //todo: this might be backwards!!!!!
				outputHeader.append("#ifdef _DEBUG\n");
				outputHeader.append(("\t\t" + boolname + " = true;\n").c_str());
				outputHeader.append("#endif // _DEBUG\n");
				outputHeader.append(("\t\t" + varname + " =  " + staticDefineInit[name] + ";\n").c_str()); //todo: Valve doesn't have two spaces here, their staticDefineInit[name] might start with a space?
			}
			else {
				outputHeader.append("#ifdef _DEBUG\n");
				outputHeader.append(("\t\t" + boolname + " = false;\n").c_str());
				outputHeader.append("#endif // _DEBUG\n");
				outputHeader.append(("\t\t" + varname + " = 0;\n").c_str());
			}
		}

		/*push @outputHeader, "\t}\n";
		push @outputHeader, "\tint GetIndex()\n";
		push @outputHeader, "\t{\n";
		push @outputHeader, "\t\t// Asserts to make sure that we aren't using any skipped combinations.\n";*/
		outputHeader.append("\t}\n");
		outputHeader.append("\tint GetIndex()\n");
		outputHeader.append("\t{\n");
		outputHeader.append("\t\t// Asserts to make sure that we aren't using any skipped combinations.\n");

		/*	foreach $skip (@perlskipcodeindividual)	{
		$skip =~ s/\$/m_n/g;
		#		push @outputHeader, "\t\tAssert( !( $skip ) );\n";
		}*/
		//for(unsigned skip = 0; skip < perlskipcodeindividual.size(); ++skip) {
		//	derp
		//}

		/*push @outputHeader, "\t\t// Asserts to make sure that we are setting all of the combination vars.\n";
		push @outputHeader, "#ifdef _DEBUG\n";*/
		outputHeader.append("\t\t// Asserts to make sure that we are setting all of the combination vars.\n");
		outputHeader.append("#ifdef _DEBUG\n");

		/*if( scalar( @staticDefineNames ) > 0 ) {
		push @outputHeader, "\t\tbool bAllStaticVarsDefined = ";
		WriteStaticBoolExpression( "", "&&" );
		}
		if( scalar( @staticDefineNames ) > 0 )
		{
		push @outputHeader, "\t\tAssert( bAllStaticVarsDefined );\n";
		}*/
		if(staticDefineNames.size() > 0) {
			outputHeader.append("\t\tbool bAllStaticVarsDefined = ");
			/*sub WriteStaticBoolExpression
			{
				local( $prefix ) = shift;
				local( $operator ) = shift;
				for( $i = 0; $i < scalar( @staticDefineNames ); $i++ )
				{
					if( $i )
					{
						push @outputHeader, " $operator ";
					}
					local( $name ) = @staticDefineNames[$i];
					local( $boolname ) = "m_b" . $name;
					push @outputHeader, "$prefix$boolname";
				}
				push @outputHeader, ";\n";
			}*/
			for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
				if(i) { //dumb
					outputHeader.append(" && ");
				}
				outputHeader.append(("m_b" + staticDefineNames[i]).c_str());
			}
			outputHeader.append(";\n");
			outputHeader.append("\t\tAssert( bAllStaticVarsDefined );\n");
		}

		/*push @outputHeader, "#endif // _DEBUG\n";

		if( $spewCombos && scalar( @staticDefineNames ) ) {
		push @outputHeader, &CreateCCodeToSpewStaticCombo();
		}*/
		outputHeader.append("#endif // _DEBUG\n");

		if(spewCombos && staticDefineNames.size()) {
			/*sub CreateCCodeToSpewStaticCombo
			{
			local( $out ) = "";

			$out .= "\t\tOutputDebugString( \"src:$fxc_filename vcs:$fxc_basename static index\" );\n";
			$out .= "\t\tchar tmp[128];\n";
			$out .= "\t\tint shaderID = ";*/
			outputHeader.append(("\t\tOutputDebugString ( \"src:" + fxc_filename + " vcs:" + fxc_basename + " static index\" );\n").c_str());
			outputHeader.append("\t\tchar tmp[128];\n");
			outputHeader.append("\t\tint shaderID = ");

			/*local( $scale ) = 1;
			for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ ) {
			$scale *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
			}*/
			int scale = 1;
			for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
				scale *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
			}

			/*for( $i = 0; $i < scalar( @staticDefineNames ); $i++ ) {
			local( $name ) = @staticDefineNames[$i];
			local( $varname ) = "m_n" . $name;
			$out .= "( $scale * $varname ) + ";
			$scale *= $staticDefineMax[$i] - $staticDefineMin[$i] + 1;
			}*/
			for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
				outputHeader.append(("( " + AppendInt(scale) + " * m_n" + staticDefineNames[i] + " ) + ").c_str());

				scale *= staticDefineMax[i] - staticDefineMin[i] + 1;
			}
			

			/*$out .= "0;\n";
			if( scalar( @staticDefineNames ) + scalar( @staticDefineNames ) > 0 ) {
				$out .= "\t\tint nCombo = shaderID;\n";
			}*/
			outputHeader.append("0;\n");

			if(staticDefineNames.size() > 0) {
				outputHeader.append("\t\tint nCombo = shaderID;\n");
			}

			/*my $type = GetShaderType( $fxc_filename );
			for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
			{
				$out .= "\t\tnCombo = nCombo / " . ( $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1 ) . ";\n";
			}*/
			std::string type = GetShaderType(fxc_basename, debug, ps2a);  // Valve passes fxc_filename but the function does not use it, opting for fxc_basename instead.
			for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
				outputHeader.append("\t\tnCombo = nCombo / " + AppendInt((dynamicDefineMax[i] - dynamicDefineMin[i] + 1)) + ";\n");
			}

			/*	for( $i = 0; $i < scalar( @staticDefineNames ); $i++ )
			{
			$out .= "\t\tint n$staticDefineNames[$i] = nCombo % ";
			$out .= ( $staticDefineMax[$i] - $staticDefineMin[$i] + 1 ) + $staticDefineMin[$i];
			$out .= ";\n";

			$out .= "\t\tsprintf( tmp, \"\%d\", n$staticDefineNames[$i] );\n";
			$out .= "\t\tOutputDebugString( \" $staticDefineNames[$i]";
			$out .= "=\" );\n";
			$out .= "\t\tOutputDebugString( tmp );\n";

			$out .= "\t\tnCombo = nCombo / " . ( $staticDefineMax[$i] - $staticDefineMin[$i] + 1 ) . ";\n";
			$out .= "\n";
			}*/
			for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
				outputHeader.append("\t\tint n" + staticDefineNames[i] + " = nCombo % ");
				outputHeader.append(AppendInt((staticDefineMax[i] - staticDefineMin[i] + 1) + staticDefineMin[i]) + ";\n");

				outputHeader.append(("\t\tsprintf( tmp, \"%d\", n" + staticDefineNames[i] + " );\n").c_str());
				outputHeader.append(("\t\tOutputDebugString( \" " + staticDefineNames[i] + "=\" );\n").c_str());
				outputHeader.append("\t\tOutputDebugString( tmp );\n");
				
				outputHeader.append("\t\tnCombo = nCombo / " + AppendInt((staticDefineMax[i] - staticDefineMin[i] + 1)) + ";\n\n");
			}

			//$out .= "\t\tOutputDebugString( \"\\n\" );\n";
			outputHeader.append("\t\tOutputDebugString( \"\\n\" );\n");
			//return $out;
			//DF; We don't need to return anything end of the CreateCCodeToSpewStaticCombo()
		}

		/*push @outputHeader, "\t\treturn ";
		local( $scale ) = 1;
		for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ ) {
		$scale *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
		}*/
		outputHeader.append("\t\treturn ");
		unsigned scale = 1;
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			scale *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
		}

		/*for( $i = 0; $i < scalar( @staticDefineNames ); $i++ ) {
			local( $name ) = @staticDefineNames[$i];
			local( $varname ) = "m_n" . $name;
			push @outputHeader, "( $scale * $varname ) + ";
			$scale *= $staticDefineMax[$i] - $staticDefineMin[$i] + 1;
		}*/
		for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
			outputHeader.append(("( " + AppendInt(scale) + " * m_n" + staticDefineNames[i] + " ) + ").c_str());
			scale *= staticDefineMax[i] - staticDefineMin[i] + 1;
		}

		/*	push @outputHeader, "0;\n";
		push @outputHeader, "\t}\n";
		push @outputHeader, "};\n";
		push @outputHeader, "\#define shaderStaticTest_" . $basename . " ";*/
		outputHeader.append("0;\n");
		outputHeader.append("\t}\n");
		outputHeader.append("};\n");
		outputHeader.append(("#define shaderStaticTest_" + basename + " ").c_str());


		/*my $prefix;
		my $shaderType = &GetShaderType( $fxc_filename );
		if( $shaderType =~ m/^vs/i )
		{
		$prefix = "vsh_";
		}
		else
		{
		$prefix = "psh_";
		}*/
		std::string prefix;
		if(GetShaderType(fxc_basename, debug, ps2a).find("vs") != -1) { // Valve passes fxc_filename but the function does not use it, opting for fxc_basename instead.
			prefix = "vsh_";
		}
		else {
			prefix = "psh_";
		}

		/*for( $i = 0; $i < scalar( @staticDefineNames ); $i++ )
		{
			local( $name ) = @staticDefineNames[$i];
			push @outputHeader, $prefix . "forgot_to_set_static_" . $name . " + " unless (length($staticDefineInit{$name} ));
		}
		push @outputHeader, "0\n";*/
		for(unsigned i = 0; i < staticDefineNames.size(); ++i) {
			if(staticDefineInit[staticDefineNames[i]] == "") {
				outputHeader.append((prefix + "forgot_to_set_static_" + staticDefineNames[i] + " + ").c_str());
			}
		}
		outputHeader.append("0\n");

		//-------------------------------//
		//Start of Dynamic Helper Classes
		//-------------------------------//

		/*sub WriteDynamicHelperClasses	{
		local( $basename ) = $fxc_basename;
		$basename =~ tr/A-Z/a-z/;
		local( $classname ) = $basename . "_Dynamic_Index";
		push @outputHeader, "class $classname\n";
		push @outputHeader, "{\n";*/
		std::string basename_dyn = fxc_basename;
		std::transform(basename_dyn.begin(), basename_dyn.end(), basename_dyn.begin(), ::tolower);
		std::string classname_dyn = basename_dyn + "_Dynamic_Index";
		outputHeader.append(("class " + classname_dyn + "\n").c_str());
		outputHeader.append("{\n");

		/*for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )	{
			$name = $dynamicDefineNames[$i];
			$min = $dynamicDefineMin[$i];
			$max = $dynamicDefineMax[$i];
			&WriteHelperVar( $name, $min, $max );
		}*/
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			outputHeader.append(WriteHelperVar(dynamicDefineNames[i], dynamicDefineMin[i], dynamicDefineMax[i]));
		}

		/*push @outputHeader, "public:\n";
		#	push @outputHeader, "void SetPixelShaderIndex( IShaderAPI *pShaderAPI ) { pShaderAPI->SetPixelShaderIndex( GetIndex() ); }\n";
		push @outputHeader, "\t$classname()\n";
		push @outputHeader, "\t{\n";*/
		outputHeader.append("public:\n");
		outputHeader.append(("\t" + classname_dyn + "()\n").c_str());
		outputHeader.append("\t{\n");

		/*for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ ) {
			local( $name ) = @dynamicDefineNames[$i];
			local( $boolname ) = "m_b" . $name;
			local( $varname ) = "m_n" . $name;
			push @outputHeader, "#ifdef _DEBUG\n";
			push @outputHeader, "\t\t$boolname = false;\n";
			push @outputHeader, "#endif // _DEBUG\n";
			push @outputHeader, "\t\t$varname = 0;\n";
		}*/
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			std::string name = dynamicDefineNames[i];
			std::string boolname = "m_b" + name;
			std::string varname = "m_n" + name;
			outputHeader.append("#ifdef _DEBUG\n");
			outputHeader.append(("\t\t" + boolname + " = false;\n").c_str());
			outputHeader.append("#endif // _DEBUG\n");
			outputHeader.append(("\t\t" + varname + " = 0;\n").c_str());
		}

		/*push @outputHeader, "\t}\n";
		push @outputHeader, "\tint GetIndex()\n";
		push @outputHeader, "\t{\n";
		push @outputHeader, "\t\t// Asserts to make sure that we aren't using any skipped combinations.\n";*/
		outputHeader.append("\t}\n");
		outputHeader.append("\tint GetIndex()\n");
		outputHeader.append("\t{\n");
		outputHeader.append("\t\t// Asserts to make sure that we aren't using any skipped combinations.\n");

		/*foreach $skip (@perlskipcodeindividual)
		{
		# can't do this static and dynamic can see each other.
		#	$skip =~ s/\$/m_n/g;
		#	$skip =~ s/defined//g;
		#	push @outputHeader, "\t\tAssert( !( $skip ) );\n";
		#	print "\t\tAssert( !( $skip ) );\n";
		}*/

		/*push @outputHeader, "\t\t// Asserts to make sure that we are setting all of the combination vars.\n";
		push @outputHeader, "#ifdef _DEBUG\n";*/
		outputHeader.append("\t\t// Asserts to make sure that we are setting all of the combination vars.\n");
		outputHeader.append("#ifdef _DEBUG\n");

		/*	if( scalar( @dynamicDefineNames ) > 0 )
		{
		push @outputHeader, "\t\tbool bAllDynamicVarsDefined = ";
		WriteDynamicBoolExpression( "", "&&" );
		}
		if( scalar( @dynamicDefineNames ) > 0 )
		{
		push @outputHeader, "\t\tAssert( bAllDynamicVarsDefined );\n";
		}*/
		if(dynamicDefineNames.size() > 0) {
			outputHeader.append("\t\tbool bAllDynamicVarsDefined = ");
			/*sub WriteDynamicBoolExpression {
				local( $prefix ) = shift;
				local( $operator ) = shift;
				for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
				{
					if( $i )
					{
						push @outputHeader, " $operator ";
					}
					local( $name ) = @dynamicDefineNames[$i];
					local( $boolname ) = "m_b" . $name;
					push @outputHeader, "$prefix$boolname";
				}
				push @outputHeader, ";\n";
			}*/
			for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
				if(i) { //dumb
					outputHeader.append(" && ");
				}
				outputHeader.append(("m_b" + dynamicDefineNames[i]).c_str());
			}
			outputHeader.append(";\n");
			outputHeader.append("\t\tAssert( bAllDynamicVarsDefined );\n");
		}

		/*push @outputHeader, "#endif // _DEBUG\n";
		if( $spewCombos && scalar( @dynamicDefineNames ) )
		{
			push @outputHeader, &CreateCCodeToSpewDynamicCombo();
		}*/
		outputHeader.append("#endif // _DEBUG\n");

		if(spewCombos && dynamicDefineNames.size()) {
			/*sub CreateCCodeToSpewDynamicCombo	{
			local( $out ) = "";

			$out .= "\t\tOutputDebugString( \"src:$fxc_filename vcs:$fxc_basename dynamic index\" );\n";
			$out .= "\t\tchar tmp[128];\n";
			$out .= "\t\tint shaderID = ";*/
			
			outputHeader.append(("\t\tOutputDebugString ( \"src:" + fxc_filename + " vcs:" + fxc_basename + " dynamic index\" );\n").c_str());
			outputHeader.append("\t\tchar tmp[128];\n");
			outputHeader.append("\t\tint shaderID = ");
			/*local( $scale ) = 1;
			for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
			{
				local( $name ) = @dynamicDefineNames[$i];
				local( $varname ) = "m_n" . $name;
				$out .= "( $scale * $varname ) + ";
				$scale *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
			}*/
			int scale = 1;
			for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
				//outputHeader.append("( ");
				//outputHeader += scale;
				outputHeader.append(("( " + AppendInt(scale) + " * m_n" + dynamicDefineNames[i] + " ) + ").c_str());
				scale *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
			}

			/*$out .= "0;\n";
			if( scalar( @dynamicDefineNames ) + scalar( @staticDefineNames ) > 0 )
			{
				$out .= "\t\tint nCombo = shaderID;\n";
			}*/
			outputHeader.append("0;\n");

			if(dynamicDefineNames.size() + staticDefineNames.size() > 0) {
				outputHeader.append("\t\tint nCombo = shaderID;\n");
			}

			/*my $type = GetShaderType( $fxc_filename );
			for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ )
			{
				$out .= "\t\tint n$dynamicDefineNames[$i] = nCombo % ";
				$out .= ( $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1 ) + $dynamicDefineMin[$i];
				$out .= ";\n";

				$out .= "\t\tsprintf( tmp, \"\%d\", n$dynamicDefineNames[$i] );\n";
				$out .= "\t\tOutputDebugString( \" $dynamicDefineNames[$i]";
				$out .= "=\" );\n";
				$out .= "\t\tOutputDebugString( tmp );\n";

				$out .= "\t\tnCombo = nCombo / " . ( $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1 ) . ";\n";
				$out .= "\n";
			}
			$out .= "\t\tOutputDebugString( \"\\n\" );\n";*/
			for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
				outputHeader.append("\t\tint n" + dynamicDefineNames[i] + " = nCombo % ");
				outputHeader.append(AppendInt((dynamicDefineMax[i] - dynamicDefineMin[i] + 1) + dynamicDefineMin[i]) + ";\n");

				outputHeader.append(("\t\tsprintf( tmp, \"%d\", n" + dynamicDefineNames[i] + " );\n").c_str());
				outputHeader.append(("\t\tOutputDebugString( \" " + dynamicDefineNames[i] + "=\" );\n").c_str());
				outputHeader.append("\t\tOutputDebugString( tmp );\n");

				outputHeader.append("\t\tnCombo = nCombo / " + AppendInt((dynamicDefineMax[i] - dynamicDefineMin[i] + 1)) + ";\n\n");
			}

			outputHeader.append("\t\tOutputDebugString( \"\\n\" );\n");
			//return $out;
			//DF; We don't need to return anything end of the CreateCCodeToSpewDynamicCombo()
		}

		/*push @outputHeader, "\t\treturn ";
		local( $scale ) = 1;
		for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ ) {
			local( $name ) = @dynamicDefineNames[$i];
			local( $varname ) = "m_n" . $name;
			push @outputHeader, "( $scale * $varname ) + ";
			$scale *= $dynamicDefineMax[$i] - $dynamicDefineMin[$i] + 1;
		}*/
		outputHeader.append("\t\treturn ");
		int scale_dyn = 1;
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			outputHeader.append(("( " + AppendInt(scale_dyn) + " * m_n" + dynamicDefineNames[i] + " ) + ").c_str());
			scale_dyn *= dynamicDefineMax[i] - dynamicDefineMin[i] + 1;
		}

		/*push @outputHeader, "0;\n";
		push @outputHeader, "\t}\n";
		push @outputHeader, "};\n";
		push @outputHeader, "\#define shaderDynamicTest_" . $basename . " ";*/
		outputHeader.append("0;\n");
		outputHeader.append("\t}\n");
		outputHeader.append("};\n");
		outputHeader.append(("#define shaderDynamicTest_" + basename_dyn + " ").c_str());

		/*my $prefix;
		my $shaderType = &GetShaderType( $fxc_filename );
		if( $shaderType =~ m/^vs/i ) {
			$prefix = "vsh_";
		}
		else {
			$prefix = "psh_";
		}*/
		std::string prefix_dyn;
		if(GetShaderType(fxc_basename, debug, ps2a).find("vs") != -1) { // Valve passes fxc_filename but the function does not use it, opting for fxc_basename instead.
			prefix_dyn = "vsh_";
		}
		else {
			prefix_dyn = "psh_";
		}

		/*for( $i = 0; $i < scalar( @dynamicDefineNames ); $i++ ) {
			local( $name ) = @dynamicDefineNames[$i];
			push @outputHeader, $prefix . "forgot_to_set_dynamic_" . $name . " + ";
		}
		push @outputHeader, "0\n";*/
		for(unsigned i = 0; i < dynamicDefineNames.size(); ++i) {
			outputHeader.append((prefix_dyn + "forgot_to_set_dynamic_" + dynamicDefineNames[i] + " + ").c_str());
		}
		outputHeader.append("0\n");
		//end of dynamic

		/*my $incfilename = "$fxctmp\\$fxc_basename" . ".inc";
		&WriteFile( $incfilename, join( "", @outputHeader ) );*/
		FILE * pIncFile;
		std::string dumbtmp = shaderpath + '\\' + fxctmp + '\\' + fxc_basename + ".inc";
		if(fopen_s(&pIncFile, dumbtmp.c_str(), "w") != 0) {
			std::cout << "The file " << dumbtmp << " was not opened!\n";
			exit(EXIT_FAILURE);
		}
		fprintf(pIncFile, "%s", outputHeader.c_str());
		fclose(pIncFile);
	}

	/*if( $generateListingFile )
	{
		my $listFileName = "$fxctmp/$fxc_basename" . ".lst";
		print "writing $listFileName\n";
		if( !open FILE, ">$listFileName" )
		{
			die;
		}
		print FILE @listingOutput;
		close FILE;
	}*/
	//this apparently does not work.
}
/*for(std::map<std::string, std::string>::iterator it = staticDefineInit.begin(); it != staticDefineInit.end(); ++it) {
	std::cout << it->first << " => " << it->second << "\n";
}*/

/*sub ReadInputFile
{
	local( $filename ) = shift;
	local( *INPUT );
	local( @output );
	open INPUT, "<$filename" || die;

	local( $line );
	local( $linenum ) = 1;
	while( $line = <INPUT> )
	{
#		print "LINE: $line";
#		$line =~ s/\n//g;
#		local( $postfix ) = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
#		$postfix .= "; LINEINFO($filename)($linenum)\n";
		if( $line =~ m/\#include\s+\"(.*)\"/i )
		{
			push @output, &ReadInputFile( $1 );
		}
		else
		{
#			push @output, $line . $postfix;
			push @output, $line;
		}
		$linenum++;
	}

	close INPUT;
#	print "-----------------\n";
#	print @output;
#	print "-----------------\n";
	return @output;
}*/
std::vector<std::string> ReadInputFile(std::string shaderpath, std::string filename) {
	std::vector<std::string> output;

	filename = shaderpath + "\\" + filename;
	std::ifstream INPUT(filename);

	//std::cout << "DICKS DICKS DICKS DICKS: " << filename << "\n";

	if(INPUT.is_open()) {
		std::string line;
		//unsigned linenum = 1;
		while(INPUT.good()) {
			getline(INPUT, line);
			//std::cout << line << "\n";
			if(line.substr(0,2) != "//" && !line.empty() && line.find("#include") != -1 && line.find_first_of('"') != -1) {
				std::string tmp = line.substr(line.find_first_of('"') + 1, line.find_last_of('"') - (line.find_first_of('"') + 1));
				std::vector<std::string> tmpvec = ReadInputFile(shaderpath, tmp);
				output.insert(output.end(), tmpvec.begin(), tmpvec.end());
			}
			else {
				output.push_back(line);
			}
		}
	}
	else {
		std::cout << "Could not open Input File: " << shaderpath << '\\' << filename << "\n";
		exit(EXIT_FAILURE);
	}

	INPUT.close();
	return output;
}

/*sub GetShaderType
{
	local( $shadername ) = shift; # hack - use global variables
		$shadername = $fxc_basename;
	if( $shadername =~ m/ps30/i )
	{
		if( $debug )
		{
			return "ps_3_sw";
		}
		else
		{
			return "ps_3_0";
		}
	}
	elsif( $shadername =~ m/ps20b/i )
	{
		return "ps_2_b";
	}
	elsif( $shadername =~ m/ps20/i )
	{
		if( $debug )
		{
			return "ps_2_sw";
		}
		else
		{
			if( $ps2a )
			{
				return "ps_2_a";
			}
			else
			{
				return "ps_2_0";
			}
		}
	}
	elsif( $shadername =~ m/ps14/i )
	{
		return "ps_1_4";
	}
	elsif( $shadername =~ m/ps11/i )
	{
		return "ps_1_1";
	}
	elsif( $shadername =~ m/vs30/i )
	{
		if( $debug )
		{
			return "vs_3_sw";
		}
		else
		{
			return "vs_3_0";
		}
	}
	elsif( $shadername =~ m/vs20/i )
	{
		if( $debug )
		{
			return "vs_2_sw";
		}
		else
		{
			return "vs_2_0";
		}
	}
	elsif( $shadername =~ m/vs14/i )
	{
		return "vs_1_1";
	}
	elsif( $shadername =~ m/vs11/i )
	{
		return "vs_1_1";
	}
	else
	{
		die "\n\nSHADERNAME = $shadername\n\n";
	}
}*/

std::string GetShaderType(const std::string shadername, const bool debug, const bool ps2a) { //todo: reorganize
	if(shadername.find("ps30") != -1) {
		return debug ? "ps_3_sw" : "ps_3_0";
	}
	else if(shadername.find("ps20b") != -1) {
		return "ps_2_b";
	}
	else if(shadername.find("ps20") != -1) {
		if(debug) {
			return "ps_2_sw";
		}
		else if(ps2a) {
			return "ps_2_a";
		}
		else {
			return "ps_2_0";
		}
	}
	else if(shadername.find("ps14") != -1) {
		return "ps_1_4";
	}
	else if(shadername.find("ps11") != -1) {
		return "ps_1_1";
	}
	else if(shadername.find("vs30") != -1) {
		return debug ? "vs_3_sw" : "vs_3_0";
	}
	else if(shadername.find("vs20") != -1) {
		return debug ? "vs_2_sw" : "vs_2_0";
	}
	else if(shadername.find("vs14") != -1) {
		return "vs_1_1";
	}
	else if(shadername.find("vs11") != -1) {
		return "vs_1_1";
	}

	std::cout << "Invalid shader name: " << shadername << "\n";
	exit(EXIT_FAILURE);
}

/*sub WriteHelperVar
{
local( $name ) = shift;
local( $min ) = shift;
local( $max ) = shift;
local( $varname ) = "m_n" . $name;
local( $boolname ) = "m_b" . $name;
push @outputHeader, "private:\n";
push @outputHeader, "\tint $varname;\n";
push @outputHeader, "#ifdef _DEBUG\n";
push @outputHeader, "\tbool $boolname;\n";
push @outputHeader, "#endif\n";
push @outputHeader, "public:\n";
# int version of set function
push @outputHeader, "\tvoid Set" . $name . "( int i )\n";
push @outputHeader, "\t{\n";
push @outputHeader, "\t\tAssert( i >= $min && i <= $max );\n";
push @outputHeader, "\t\t$varname = i;\n";
push @outputHeader, "#ifdef _DEBUG\n";
push @outputHeader, "\t\t$boolname = true;\n";
push @outputHeader, "#endif\n";
push @outputHeader, "\t}\n";
# bool version of set function
push @outputHeader, "\tvoid Set" . $name . "( bool i )\n";
push @outputHeader, "\t{\n";
#		push @outputHeader, "\t\tAssert( i >= $min && i <= $max );\n";
push @outputHeader, "\t\t$varname = i ? 1 : 0;\n";
push @outputHeader, "#ifdef _DEBUG\n";
push @outputHeader, "\t\t$boolname = true;\n";
push @outputHeader, "#endif\n";
push @outputHeader, "\t}\n";
}*/

std::string WriteHelperVar(const std::string name, const int min, const int max) {
	std::string varname = "m_n" + name;
	std::string boolname = "m_b" + name;
	std::string tmpoutput;
	std::string sometimescppisfuckingtmp;
	tmpoutput.append("private:\n");
	tmpoutput.append(("\tint " + varname + ";\n").c_str());
	tmpoutput.append("#ifdef _DEBUG\n");
	tmpoutput.append(("\tbool " + boolname + ";\n").c_str());
	tmpoutput.append("#endif\n");
	tmpoutput.append("public:\n");
	//int version of set function
	tmpoutput.append(("\tvoid Set" + name + "( int i )\n").c_str());
	tmpoutput.append("\t{\n");
	tmpoutput.append(("\t\tAssert( i >= " + AppendInt(min) + " && i <= " + AppendInt(max) + " );\n").c_str()); //line continued below
	tmpoutput.append(("\t\t" + varname + " = i;\n").c_str());
	tmpoutput.append("#ifdef _DEBUG\n");
	tmpoutput.append(("\t\t" + boolname + " = true;\n").c_str());
	tmpoutput.append("#endif\n");
	tmpoutput.append("\t}\n");
	//bool version of the set function
	tmpoutput.append(("\tvoid Set" + name + "( bool i )\n").c_str());
	tmpoutput.append("\t{\n");
	tmpoutput.append(("\t\t" + varname + " = i ? 1 : 0;\n").c_str());
	tmpoutput.append("#ifdef _DEBUG\n");
	tmpoutput.append(("\t\t" + boolname + " = true;\n").c_str());
	tmpoutput.append("#endif\n");
	tmpoutput.append("\t}\n");

	return tmpoutput;
}