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
#ifndef __cloudblockfs_BlockDevice_h
#define __cloudblockfs_BlockDevice_h

#include <string>
#include <inttypes.h>
#include <memory>
#include "BlockMeta.h"

namespace cloudblockfs
{
	class DataStore;

	/**
	 * Block device represents the 
	 */
	class BlockStorageDevice
	{
	private:
		std::auto_ptr<DataStore> m_store; // storage backend
		BlockMeta m_meta;
		
		mutable std::vector<uint8_t> m_block; // tmp storage
		
		BlockStorageDevice(const BlockStorageDevice&);
		BlockStorageDevice& operator =(const BlockStorageDevice&);
	public:
		// getters & setters
		int GetBlockSize() const { 
			BlockMeta::Head head;
			m_meta.GetHead(&head);
			return head.block_size; 
		}
		
		int GetTreeDepth() const
		{
			BlockMeta::Head head;
			m_meta.GetHead(&head);
			return head.tree_depth; 
		}
		
		int64_t GetDiskSize() const { 
			BlockMeta::Head head;
			m_meta.GetHead(&head);
			return head.disk_size; 
		}
		
		/**
		 * Create a handle to a block device using the provided storage backend.
		 * @param store Storage backend
		 */
		BlockStorageDevice(DataStore *store);
		
		/**
		 * Checks whether the data storage is a valid block device.
		 * Notice that this is a weak check.
		 * @return True if block device is valid
		 */
		bool IsValid() const;
		
		/**
		 * Checks if our data store head is synchronized with the journal.
		 */
		void Check();
		
		/**
		 * Synchronizes all metadata to the data store
		 */
		void Sync();
		
		/**
		 * Initializes the block storage device.
		 * @param block_size Size of each block. May be one of 1024, 2048, 4096, 8192, 16384, 32768.
		 * @param tree_level The depth of the meta trees.
		 */
		void Format(int block_size,int tree_depth = 1);
		
		/**
		 * Deletes all files in the block storage.
		 * Use with caution.
		 */
		void Delete();
		
		/**
		 * Analyses the data storage and remove all unlinked objects.
		 * This operation is expensive. Use with caution.
		 */
		void GC();
		
		/**
		 * Resize the disk file.
		 * @param size Size in bytes.
		 */
		void Truncate(int size);
		
		/**
		 * Low level block writer. Writes a block to the data store.
		 * @param blockno Block number.
		 * @param data A block of data. Data must be of the same size as a block size.
		 */
		void WriteBlock(uint64_t blockno,const void *data);
		
		/**
		 * Low level block reader. Reads a block from the data store.
		 * @param blockno Block number.
		 * @param data A block of data. Data must be of the same size as a block size.
		 */
		void ReadBlock(uint64_t blockno,void *data) const;

		/**
		 * Write data with size to offset.
		 * @param data Data
		 * @param size Size in bytes to write
		 * @param offset Offset to write
		 */
		void Write(const void *data,int size,uint64_t offset);
		
		/**
		 * Reads data with size to offset.
		 * @param data Data
		 * @param size Size in bytes to read
		 * @param offset Offset to read
		 */
		void Read(void *data,int size,uint64_t offset) const;
	};
}

#endif
