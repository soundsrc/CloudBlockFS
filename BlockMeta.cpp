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
#include <stdlib.h>
#include <time.h>
#include "Exception.h"
#include "DataStore.h"
#include "BlockMeta.h"

using namespace cloudblockfs;

BlockMeta::BlockMeta(DataStore *store) : m_store(store), m_last_id(0)
{
}

BlockID BlockMeta::AllocateBlockID()
{
	if(!m_last_id) {
		BlockMeta::Head head;
		GetHead(&head);
		m_last_id = head.last_id;
	}

	// 64-bit LFSR
	// x^64 + x^4 + x^3 + x^1 + 1
	const int64_t bit = m_last_id ^
		(m_last_id >> 60) ^
		(m_last_id >> 61) ^
		(m_last_id >> 63) & 1;
	m_last_id = (bit << 63) | (m_last_id >> 1);
	return m_last_id;
}

void BlockMeta::SetBlockIDForBlockNo(uint32_t no,BlockID block_id)
{
	// get head
	BlockMeta::Head head;
	GetHead(&head);

	std::vector<BlockID> delete_list;
	delete_list.reserve(head.tree_level + 2);
	
	const int bk_count = head.block_size >> 3; // block count per object
	m_table.resize(bk_count);
	char object[32];
	
	// grab head object
	sprintf(object,"%.16llX",head.head_id);
	m_store->GetObject(object,&m_table[0],head.block_size);
	
	// allocate new id for head object
	BlockID cur_id = AllocateBlockID();
	delete_list.push_back(head.head_id);
	head.head_id = cur_id;
	
	// chain down the tree
	for(int i = 1; i < head.tree_level; i++) {
		const BlockID new_id = AllocateBlockID(); // new id for next table
		
		const BlockID next = m_table[no % bk_count]; // save a copy of the next table
		delete_list.push_back(next);

		m_table[no % bk_count] = new_id; // chain to new tree and write it
		sprintf(object,"%.16llX",cur_id);
		m_store->PutObject(object,&m_table[0],head.block_size);
		
		// there maybe sub-trees
		if(next) {
			sprintf(object,"%.16llX",next); // grab next data
			m_store->GetObject(object,&m_table[0],head.block_size);
		} else {
			memset(&m_table[0],0,head.block_size);
		}
		
		cur_id = new_id;
		
		no /= bk_count;
	}
	
	if(no > bk_count) throw OutOfDiskSpaceException("No space left on device.");
	delete_list.push_back(m_table[no]);
	m_table[no] = block_id;
	sprintf(object,"%.16llX",cur_id);
	m_store->PutObject(object,&m_table[0],head.block_size);
	
	// finally write head
	PutHead(head);
	
	// remove objects
	for(std::vector<BlockID>::const_iterator it = delete_list.begin(); it != delete_list.end(); ++it) {
		if(*it) {
			sprintf(object,"%.16llX",*it);
			m_store->DeleteObject(object);
		}
	}
}

BlockID BlockMeta::GetBlockIDForBlockNo(uint32_t no) const
{
	// get head
	BlockMeta::Head head;
	GetHead(&head);

	const int bk_count = head.block_size >> 3; // block count per object
	m_table.resize(bk_count);
	char object[32];
	
	// grab head object
	sprintf(object,"%.16llX",head.head_id);
	m_store->GetObject(object,&m_table[0],head.block_size);
	
	// chain down the tree
	for(int i = 1; i < head.tree_level; i++) {
		// there maybe sub-trees
		BlockID obj = m_table[no % bk_count];
		if(obj) {
			sprintf(object,"%.16llX",obj);
			m_store->GetObject(object,&m_table[0],head.block_size);
		} else {
			memset(&m_table[0],0,head.block_size);
		}
		no /= bk_count;
	}
	
	return m_table[no];
}
