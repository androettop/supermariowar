#include "path.h"

#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
    #ifdef _XBOX
        #include <xtl.h>
    #else
        #include <shlobj.h>
        #include "SDL_platform.h"
    #endif
#else
#include <stdlib.h>
#endif


extern char *RootDataDirectory;

using namespace std;
char SMW_Root_Data_Dir[PATH_MAX + 2] = "";

std::string	GetHomeDirectory()
{
	//NOTE: _WIN32 is also defined on _XBOX
#ifdef _XBOX
		return std::string("D:\\.smw\\");
#elif __MACOSX__
		char * folder=getenv("HOME");
			return std::string(folder) + std::string("/Library/Preferences/.smw/");
#elif	_WIN32
		 char folder[ MAX_PATH ];
    if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, folder ) != S_OK) {
			throw "I could not retrieve the user's home directory!\n";
		}

		return std::string(folder) + std::string("\\.smw\\");
#else // catch-all for Linux-based systems
			char * folder=getenv("HOME");
			return std::string(folder) + std::string("/.smw/");
#endif
}

bool File_Exists (const std::string fileName)
{
	struct stat buffer;
	int i = stat(fileName.c_str(), &buffer);

	return (i == 0);
}

/*********************************************************************
  Mac OS X Application Bundles                              *********/

#ifdef __MACOSX__
#include <CoreFoundation/CoreFoundation.h>


/* Call this when your application launches */
void Initialize_Paths()
{
    if (SMW_Root_Data_Dir[0] != 0) {
		return;
	}

#ifdef APPBUNDLE
    UInt8 temp[PATH_MAX];
    CFBundleRef mainBundle;
    CFURLRef dirURL;
    mainBundle = CFBundleGetMainBundle();
    if (!mainBundle) {
        cout << "Not running from an .app bundle" << endl;
        return;
    }

	dirURL = CFBundleCopyBundleURL(mainBundle);

    if (!CFURLGetFileSystemRepresentation(dirURL, TRUE, temp,
        PATH_MAX)) {
        cout << "Could not get file system representation" << endl;
        return;
    }

	strlcat(SMW_Root_Data_Dir, (char*)temp, PATH_MAX);
	int i = strlen(SMW_Root_Data_Dir) -1;
	while (SMW_Root_Data_Dir[i] !='/'){
	   SMW_Root_Data_Dir[i] = 0;
	   --i;
	}
	SMW_Root_Data_Dir[i] = 0;

    CFRelease(dirURL);
#else
	strlcat(SMW_Root_Data_Dir, "./", 4);
#endif
	cout << "Located data folder at: " << SMW_Root_Data_Dir << endl;
}
#endif

/*********************************************************************
  All filenames should go through this                      *********/

const string convertPath(const string& source)
{
    string s;

/****** XBOX ******/
#ifdef _XBOX

	s = source;
    int slash = string :: npos;

    while (string::npos != (slash = s.find("/")))
        s.replace(slash, 1, "\\");

    s = "D:\\" + s;
	return s;
#else
    static bool are_paths_initialized = false;

    if (!are_paths_initialized) {
//		#ifdef PREFIXPATH
			strcpy(SMW_Root_Data_Dir, RootDataDirectory);
//		#endif

		#ifdef __MACOSX__
			Initialize_Paths();
		#endif

		#ifndef _WIN32
			strcat(SMW_Root_Data_Dir, "/");
		#else
			if (SMW_Root_Data_Dir[strlen(SMW_Root_Data_Dir)-1] != '\\')
				strcat(SMW_Root_Data_Dir, "\\");
		#endif

		are_paths_initialized = true;
    }

    s = SMW_Root_Data_Dir;
#endif
    s += source;
    return s;
}

const string convertPath(const string& source, const string& pack)
{
    if (source.find("gfx/packs/") == 0) {
		string trailingdir = source.substr(9);

#ifdef _XBOX
		const string s = convertPartialPath(pack + trailingdir);  //Hack because pack already has d:\ in it
#else
		const string s = convertPath(pack + trailingdir);
#endif

		//If the file exists, return the path to it
		if (File_Exists(s))
			return s;

		//If not, use the classic file
		return convertPath("gfx/packs/Classic" + trailingdir);
	}

    if (source.find("sfx/packs/") == 0) {
		string trailingdir = source.substr(9);

#ifdef _XBOX
		const string s = convertPartialPath(pack + trailingdir);  //Hack because pack already has d:\ in it
#else
		const string s = convertPath(pack + trailingdir);
#endif

		//If the file exists, return the path to it
		if (File_Exists(s))
			return s;

		//If not, use the classic file
		return convertPath("sfx/packs/Classic" + trailingdir);
	}

	return convertPath(source);
}

const string getDirectorySeperator()
{
#ifdef _XBOX
	return std::string("\\");
#else
	return std::string("/");
#endif
}

const string convertPartialPath(const string & source)
{
	string s = source;

#ifdef _XBOX
    int slash = string :: npos;

    while (string::npos != (slash = s.find("/")))
        s.replace(slash, 1, "\\");
#endif

    return s;
}

const string getFileFromPath(const string &path)
{
	short iPos = path.find_last_of(getDirectorySeperator()[0]);

	if (iPos > 0)
		return path.substr(iPos + 1);

	return path;
}

//Takes a path to a file and gives you back the file name (with or without author) as a char *
void GetNameFromFileName(char * szName, const char * szFileName, bool fStripAuthor)
{
#ifdef _XBOX
    const char * p = strrchr(szFileName, '\\');
#else
    const char * p = strrchr(szFileName, '/');
#endif

    if (!p)
        p = szFileName;
    else
        p++;

    strcpy(szName, p);

    if (fStripAuthor) {
        char * pUnderscore = strchr(szName, '_');
        if (pUnderscore)
            strcpy(szName, ++pUnderscore);
    }

    char * pLastPeriod = strrchr(szName, '.');

    if (pLastPeriod)
        *pLastPeriod = 0;
}

//Takes a file name and gives you back just the name of the file with no author or file extention
//and the first letter of the name will come back capitalized
std::string stripCreatorAndDotMap(const std::string &filename)
{
    size_t firstUnderscore = filename.find("_");    //find first _
    if (firstUnderscore == std::string::npos)   //if not found start with first character
        firstUnderscore = 0;
    else
        firstUnderscore++;                      //we don't want the _

    std::string withoutPrefix = filename.substr(firstUnderscore);   //substring without bla_ and .map (length-4)
    withoutPrefix = withoutPrefix.substr(0, withoutPrefix.length() - 4);        //i have no idea why this doesn't work if i do it like this: (leaves .map if the map starts with an underscore)
    //                                                              return filename.substr(firstUnderscore, filename.length()-4);

    //Capitalize the first letter so the hash table sorting works correctly
    if (withoutPrefix[0] >= 97 && withoutPrefix[0] <= 122)
        withoutPrefix[0] -= 32;

    return withoutPrefix;
}

//Takes a path to a file and gives you back just the name of the file with no author or file extention
std::string stripPathAndExtension(const std::string &path)
{
    size_t chopHere = path.find("_");   //find first _
    if (chopHere == std::string::npos) {    //if not found, then find the beginning of the filename
        chopHere = path.find_last_of(getDirectorySeperator());  //find last /
        if (chopHere == std::string::npos)  //if not found, start with first character
            chopHere = 0;
        else
            chopHere++;                     //we don't want the /
    } else {
        chopHere++;                     //we don't want the _
    }

    std::string withoutPath = path.substr(chopHere);    //substring without bla_
    withoutPath = withoutPath.substr(0, withoutPath.length() - 4); //and without extension like .map (length-4)

    return withoutPath;
}
