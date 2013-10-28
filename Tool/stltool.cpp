#include "stltool.h"

#include <assert.h>

void Split(const string& strSrc, const string& splitCh, vector<string>& splitString)
{
	splitString.clear();
	
	if (strSrc.size() == 0)
	{
		return;
	}

    size_t startpos = 0;
    size_t found = strSrc.find_first_of(splitCh.c_str());
    if (found == string::npos)
    {
        splitString.push_back(strSrc);
        return;
    }

    while (found != string::npos)
    {
        splitString.push_back(strSrc.substr(startpos, found-startpos));
        startpos = found+1;
        found = strSrc.find_first_of(splitCh.c_str(), startpos);
    }

    if (strSrc.size() != startpos)
    {
        splitString.push_back(strSrc.substr(startpos, strSrc.size() - startpos));
    }

}


