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
		if(!m_last_id) {
			srand(time(NULL));
			m_last_id = rand();
		}
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
	
	const int bk_count = head.block_size >> 3; // block count per object
	m_table.resize(bk_count);
	char object[32];
	
	// grab head object
	sprintf(object,"%.16llX",head.head_id);
	m_store->GetObject(object,&m_table[0],head.block_size);
	
	// allocate new id for current object
	BlockID cur_id = AllocateBlockID();
	head.head_id = cur_id;
	
	// chain down the tree
	for(int i = 1; i < head.tree_level; i++) {
		// there maybe sub-trees
		sprintf(object,"%.16llX",m_table[no % bk_count]);
		m_store->GetObject(object,&m_table[0],head.block_size);
	
		const BlockID new_id = AllocateBlockID();
		m_table[no % bk_count] = new_id;
		sprintf(object,"%.16llX",cur_id);
		m_store->PutObject(object,&m_table[0],head.block_size);
		
		cur_id = new_id;
		
		no /= bk_count;
	}
	
	m_table[no] = block_id;
	sprintf(object,"%.16llX",cur_id);
	m_store->PutObject(object,&m_table[0],head.block_size);
	
	// finally write head
	PutHead(head);
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
		sprintf(object,"%.16llX",m_table[no % bk_count]);
		m_store->GetObject(object,&m_table[0],head.block_size);
		no /= bk_count;
	}
	
	return m_table[no];
}
