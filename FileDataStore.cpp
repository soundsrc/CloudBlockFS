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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "Exception.h"
#include "FileDataStore.h"

using namespace cloudblockfs;

FileDataStore::FileDataStore(const std::string& path) : m_path(path)
{
}

void FileDataStore::PutObject(const std::string& name,const void *data,int size)
{
	int fd = open((m_path + "/" + name).c_str(),O_WRONLY | O_CREAT | O_TRUNC,0600);
	if(fd < 0) throw FileIOException(name + ": " + strerror(errno));
	write(fd,data,size);
	close(fd);
}

void FileDataStore::GetObject(const std::string& name,void *data,int size) const
{
	int fd = open((m_path + "/" + name).c_str(),O_RDONLY);
	if(fd < 0) {
		switch(errno) {
			case ENOENT: throw FileNotFoundException(name + ": " + strerror(errno)); break;
			default: throw FileIOException(name + ": " + strerror(errno)); break;
		}
	} else {
		read(fd,data,size);
		close(fd);
	}
}

void FileDataStore::DeleteObject(const std::string& name)
{
	if(unlink((m_path + "/" + name).c_str()) != 0) {
		switch(errno) {
			case ENOENT: throw FileNotFoundException(name + ": " + strerror(errno));
			default: throw FileIOException(name + ": " + strerror(errno));
		}
	}
}

void FileDataStore::ListObjects(void (*list_function)(const std::string& name,void *userdata),void *userdata) const
{
	DIR *dirp;
	struct dirent *dp;
	dirp = opendir(m_path.c_str());
	while((dp = readdir(dirp)) != NULL) {
		list_function(dp->d_name,userdata);
	}
	closedir(dirp);
}
