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
#ifndef __TestSuite_TmpFileDataStore_h
#define __TestSuite_TmpFileDataStore_h

#include "TmpDir.h"
#include "DataStore.h"
#include "FileDataStore.h"

class TmpFileDataStore : public cloudblockfs::DataStore
{
private:
	TmpDir m_tmp_dir;
	cloudblockfs::FileDataStore m_store;
public:
	TmpFileDataStore() : m_store(m_tmp_dir.GetPath())
	{
	}
	virtual ~TmpFileDataStore() {}
	virtual void PutObject(const std::string& name,const void *data,int size) { m_store.PutObject(name,data,size); }
	virtual void GetObject(const std::string& name,void *data,int size) const { m_store.GetObject(name,data,size); }
	virtual void DeleteObject(const std::string& name) { m_store.DeleteObject(name); }
	virtual void ListObjects(void (*list_function)(const std::string& name,void *userdata),void *userdata) const { m_store.ListObjects(list_function,userdata); }
	virtual void Flush() { m_store.Flush(); }
};

#endif
