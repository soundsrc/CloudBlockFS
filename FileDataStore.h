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
#ifndef __cloudblockfs_FileDataStore_h
#define __cloudblockfs_FileDataStore_h

#include "DataStore.h"
#include <string>

namespace cloudblockfs
{
	/**
	 * A data store based on plain files.
	 */
	class FileDataStore : public DataStore
	{
	private:
		std::string m_path; // path to store
	public:
		FileDataStore(const std::string& path);
		virtual ~FileDataStore() { }
		
		virtual void PutObject(const std::string& name,const void *data,int size);
		virtual void GetObject(const std::string& name,void *data,int size) const;
		virtual void DeleteObject(const std::string& name);
		virtual void ListObjects(void (*list_function)(const std::string& name,void *userdata),void *userdata) const;
		virtual void Flush();
	};
}

#endif
