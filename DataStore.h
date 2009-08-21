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
#ifndef __cloudblockfs_DataStore_h
#define __cloudblockfs_DataStore_h

#include <string>

namespace cloudblockfs
{
	/**
	 * Data store interface.
	 * A data storage is basic storage system which operates mainly on fixed size
	 * put and get operations. Implementors only need to implement 4 methods to define their
	 * storage system: PutObject, GetObject, DeleteObject, ListObjects
	 */
	class DataStore
	{
	public:
		virtual ~DataStore() { }
		
		/**
		 * Writes the named object with data from the data source.
		 * If object exists, overwrite it. If size is provided, then up to
		 * size bytes are written. Otherwise, the size is not known ahead of time.
		 * @param name Name of object.
		 * @param data Data
		 * @param size Size in bytes to write, or if -1, read data source until EOF.
		 * @return True on success, false otherwise
		 */
		virtual void PutObject(const std::string& name,const void *data,int size) = 0;
		
		/**
		 * Retrieves the named object from the data store.
		 * @param name Name of object
		 * @param data Data
		 * @param size Size in bytes to read, or if -1, read entire data until EOF
		 */
		virtual void GetObject(const std::string& name,void *data,int size) const = 0;
		
		/**
		 * Removes the named object.
		 * @param name Name of object
		 */
		virtual void DeleteObject(const std::string& name) = 0;
		
		/**
		 * Obtain's a list of objects.
		 * @param list_function A callback function.
		 * @param userdata User define data that is passed to list_function.
		 */
		virtual void ListObjects(void (*list_function)(const std::string& name,void *userdata),void *userdata) const = 0;
		
		/**
		 * Flushes any buffers, ensure they are written to disk.
		 */
		virtual void Flush() = 0;
	};
}

#endif
