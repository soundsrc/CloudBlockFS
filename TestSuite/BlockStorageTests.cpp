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
#include <UnitTest++.h>
#include <vector>
#include <tr1/memory>
#include "BlockStorageDevice.h"
#include "TmpFileDataStore.h"

using namespace cloudblockfs;

typedef std::tr1::shared_ptr<BlockStorageDevice> BlockStorageDevicePtr;

class BlockStorageFixture
{
protected:
	typedef std::vector<BlockStorageDevicePtr> BlockStorageDeviceList;
	BlockStorageDeviceList m_block_devices;
public:
	BlockStorageFixture() {
		BlockStorageDevicePtr block;
		
		// format multiple disks with block sizes 1k, 4k, 16k, 64
		// also format each with multiple tree levels
		for(int i = 1; i <= 3; i++) {
			block.reset(new BlockStorageDevice(new TmpFileDataStore()));
			block->Format(1024,i);
			m_block_devices.push_back(block);
			block.reset(new BlockStorageDevice(new TmpFileDataStore()));
			block->Format(4096,i);
			m_block_devices.push_back(block);
			block.reset(new BlockStorageDevice(new TmpFileDataStore()));
			block->Format(16384,i);
			m_block_devices.push_back(block);
			block.reset(new BlockStorageDevice(new TmpFileDataStore()));
			block->Format(65536,i);
			m_block_devices.push_back(block);
		}
	}
	
	~BlockStorageFixture()
	{
		for(BlockStorageDeviceList::iterator it = m_block_devices.begin(); it != m_block_devices.end(); ++it)
		{
			(*it)->Delete();
		}
	}
};

SUITE(BlockStorageTests)
{
	TEST_FIXTURE(BlockStorageFixture,BlockReadWriteTest)
	{
		std::vector<char> expect, data;
		FILE *fp = fopen("/dev/urandom","r");
		if(fp) {
			for(BlockStorageDeviceList::iterator it = m_block_devices.begin(); it != m_block_devices.end(); ++it)
			{
				BlockStorageDevicePtr block = *it;
				
				const int block_size = block->GetBlockSize();
				expect.resize(block_size);
				data.resize(block_size);
				
				for(int i = 0; i < 5; i++) {
					// read random
					fread(&expect[0],1,block_size,fp);
					
					// store object to block 0
					block->WriteBlock(0,&expect[0]);
					block->ReadBlock(0,&data[0]);
					
					CHECK_ARRAY_EQUAL(&expect[0],&data[0],block_size);
				}
			}
			fclose(fp);
		} else {
			CHECK(false);
		}
	}
}
