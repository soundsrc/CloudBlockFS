/*
 This file is part of CloudBlockFS.
 Copyright (c) 2009 Sound <sound -at- sagaforce -dot- com>
 
 WifiPad is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 WifiPad is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with WifiPad.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __TestSuite_TmpDir_h
#define __TestSuite_TmpDir_h

#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

class TmpDir
{
private:
	std::string m_path;
public:
	TmpDir() {
		const char *t = tempnam("/tmp","cloudblockfstest");
		if(!t) throw std::runtime_error("Could not creat temp directory.");
		m_path = t;
		mkdir(t,0700);
	}
	
	~TmpDir() {
		rmdir(m_path.c_str());
	}
	
	const char *GetPath() const { return m_path.c_str(); }
};

#endif
