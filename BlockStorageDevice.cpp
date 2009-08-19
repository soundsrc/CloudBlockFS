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
#include <stdexcept>
#include <math.h>
#include "Exception.h"
#include "DataStore.h"
#include "BlockStorageDevice.h"

using namespace cloudblockfs;

BlockStorageDevice::BlockStorageDevice(DataStore *store) : m_store(store), m_meta(store)
{
}

bool BlockStorageDevice::IsValid() const
{
	try {
		return GetBlockSize() != 0;
	} catch(const std::runtime_error& ) {
	}
	return false;
}

void BlockStorageDevice::Check()
{
}

void BlockStorageDevice::Sync()
{
}

void BlockStorageDevice::Format(int block_size,int tree_level)
{
	BlockMeta::Head head;
	memset(&head,0,sizeof(head));

	switch(block_size) {
		case 1024:
		case 2048:
		case 4096:
		case 8192:
		case 16384:
		case 32768:
		case 65536: break;
		default: throw InvalidArgumentException("Invalid block size. Must be: 1024, 2048, 4096, 8192, 16384, 32768, 65536");
	}
	
	head.block_size = block_size;
	head.tree_level = tree_level;
	head.disk_size = 0;
	srand(time(NULL));
	head.head_id = rand() + 1;
	head.last_id = head.head_id;
	
	std::vector<char> table(head.block_size,0);
	char object[32];
	sprintf(object,"%.16llX",head.head_id);
	m_store->PutObject(object,&table[0],head.block_size);

	m_meta.PutHead(head);
}

void BlockStorageDevice::Truncate(int size)
{
	BlockMeta::Head head;
	m_meta.GetHead(&head);
	head.disk_size = size;
	m_meta.PutHead(head);
}

void BlockStorageDevice::Delete()
{
}

void BlockStorageDevice::GC()
{
}

void BlockStorageDevice::WriteBlock(uint32_t blockno,const void *data)
{
	const BlockID block_id = m_meta.AllocateBlockID();
	char object[32];
	sprintf(object,"%.16llX",block_id);
	m_store->PutObject(object,data,GetBlockSize());
	m_meta.SetBlockIDForBlockNo(blockno,block_id);
}

void BlockStorageDevice::ReadBlock(uint32_t blockno,void *data) const
{
	const BlockID block_id = m_meta.GetBlockIDForBlockNo(blockno);
	char object[32];
	
	if(block_id == 0) {
		memset(data,0,GetBlockSize());
	} else {
		sprintf(object,"%.16llX",block_id);
		m_store->GetObject(object,data,GetBlockSize());
	}
}

void BlockStorageDevice::Write(const void *data,int size,long offset)
{
	const int block_size = GetBlockSize();
	uint32_t i, start_block, end_block;
	int bytes_to_write, remaining;
	const unsigned long offset_mask = block_size - 1;
	
	m_block.resize(block_size);
	
	start_block = offset / block_size;
	end_block = (offset + size - 1) / block_size;
	
	remaining = size;
	if(start_block == end_block) {
		// we must issue a read if this is not a partial write
		if(size != block_size) {
			ReadBlock(start_block,&m_block[0]);
		}
		memcpy(&m_block[offset & offset_mask],data,size);
		WriteBlock(start_block,&m_block[0]);
	} else {
		bytes_to_write = block_size - (offset & offset_mask);
		if(bytes_to_write != block_size) {
			ReadBlock(start_block,&m_block[0]);
		}
		memcpy(&m_block[offset & offset_mask],data,bytes_to_write);
		WriteBlock(start_block,&m_block[0]);
		
        (char *&)data += bytes_to_write;
		remaining -= bytes_to_write;
		for(i = start_block + 1; i < end_block; i++) {
			WriteBlock(i,data);
			(char *&)data += block_size;
			remaining -= block_size;
		}
		
		if(remaining != block_size) {
			ReadBlock(end_block,&m_block[0]);
		}
		memcpy(&m_block[0],data,remaining);
		WriteBlock(end_block,&m_block[0]);
	}
}

void BlockStorageDevice::Read(void *data,int size,long offset) const
{
	const int block_size = GetBlockSize();
	uint32_t i, start_block, end_block;
	int bytes_to_read, remaining;
	const unsigned long offset_mask = block_size - 1;
	
	m_block.resize(block_size);
	
	// compute start and end block
	start_block = offset / block_size;
	end_block = (offset + size - 1) / block_size;

	remaining = size;
	if(start_block == end_block) {
		ReadBlock(start_block,&m_block[0]);
		memcpy(data,&m_block[offset & offset_mask],size);
	} else {
		// copy start block portion
		bytes_to_read = block_size - (offset & offset_mask);
		ReadBlock(start_block,&m_block[0]);
		memcpy(data,&m_block[offset & offset_mask],bytes_to_read);
		
		(char *&)data += bytes_to_read;
		remaining -= bytes_to_read;
		
		// copy each block in between
		for(i = start_block + 1; i < end_block; i++) {
			ReadBlock(i,data);
			(char *&)data += block_size;
			remaining -= block_size;
		}
		
		// copy end block portion
		ReadBlock(end_block,&m_block[0]);
		memcpy(data,&m_block[0],remaining);
	}
}
