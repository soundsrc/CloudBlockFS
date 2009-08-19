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
#ifndef __cloudblockfs_BlockMeta_h
#define __cloudblockfs_BlockMeta_h

#include <inttypes.h>
#include <vector>
#include "DataStore.h"

namespace cloudblockfs 
{
	typedef uint64_t BlockID;
	
	/**
	 * Class for reading block meta data. Block meta data is a table which 
	 * maps block numbers to block ids. Block IDs are unique 64-bit integers
	 * handed out to each block.
	 */
	class BlockMeta
	{
	private:
		DataStore *m_store;
		BlockID m_last_id;
		
		mutable std::vector<BlockID> m_table; // tmp data
	public:
		
		struct Head
		{
			BlockID head_id;
			int32_t block_size;
			int32_t tree_level;
			int64_t disk_size;
			BlockID last_id;
		};
		
		/**
		 * Construct a new meta handler for data store.
		 */
		BlockMeta(DataStore *store);
		
		/**
		 * Retrieves the meta header file.
		 */
		void GetHead(Head *out_head) const { m_store->GetObject("0000000000000000",out_head,sizeof(BlockMeta::Head)); }
		void PutHead(const Head& head) { 
			Head head_copy = head;
			if(m_last_id) head_copy.last_id = m_last_id;
			m_store->PutObject("0000000000000000",&head_copy,sizeof(BlockMeta::Head)); 
		}
		
		/**
		 * Returns a unique 64-bit id.
		 */
		BlockID AllocateBlockID();
		
		/**
		 * Sets the mapping for block no to be block id.
		 */
		void SetBlockIDForBlockNo(uint32_t no,BlockID block_id);
		
		/**
		 * Retrives the block id given the block no.
		 */
		BlockID GetBlockIDForBlockNo(uint32_t no) const;
	};
}

#endif
