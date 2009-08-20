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
#include <stdexcept>
#include <tr1/memory>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "Exception.h"
#include "DataStore.h"
#include "FileDataStore.h"
#include "TmpDir.h"
#include "TmpFileDataStore.h"

using namespace cloudblockfs;

class AutoDeleteObject
{
private:
	DataStore& m_store;
	std::string m_object;
public:
	AutoDeleteObject(DataStore& store,const std::string& object) : m_store(store), m_object(object) { }
	~AutoDeleteObject() {
		try { m_store.DeleteObject(m_object); }
		catch(const std::runtime_error& ) { }
	}
};

typedef std::tr1::shared_ptr<DataStore> DataStorePtr;
class DataSourceTestFixture
{
protected:
	typedef std::vector<DataStorePtr> DataStoreList;
	DataStoreList m_stores;
public:
	DataSourceTestFixture()
	{
		m_stores.push_back(DataStorePtr(new TmpFileDataStore()));
	}
};

SUITE(DataSourceTests)
{
	TEST_FIXTURE(DataSourceTestFixture,DeleteTest)
	{
		for(DataStoreList::iterator it = m_stores.begin(); it != m_stores.end(); ++it)
		{
			DataStorePtr store = *it;
			char buf[20];
			
			CHECK_THROW(store->DeleteObject("sajdhasjhdad"),FileNotFoundException);
			
			store->PutObject("abc",buf,20);
			store->DeleteObject("abc");
			CHECK(true);
		}
	}
	
	TEST_FIXTURE(DataSourceTestFixture,ReadWriteTest)
	{
		for(DataStoreList::iterator it = m_stores.begin(); it != m_stores.end(); ++it)
		{
			DataStorePtr store = *it;
			
			char random_buf[4096];
			char buffer[4096];
			FILE *fp = fopen("/dev/urandom","r");
			if(fp) {
				
				// generate 10 random id's
				srandom(time(NULL));
				for(int i = 0; i < 10; i++) {
					char object[32];
					sprintf(object,"%.16llX",(long long)random() << 32LL | random());
					
					AutoDeleteObject del(*store,object); // auto deletes object
					
					// reads random data, writes it to the same buf 5 times
					for(int j = 0; j < 5; j++) {
						fread(random_buf,1,4096,fp);
						memset(buffer,0xCC,4096);
						store->PutObject(object,random_buf,4096);
						store->GetObject(object,buffer,4096);
						CHECK_ARRAY_EQUAL(random_buf,buffer,4096);
					}
				}
				
				fclose(fp);
			} else {
				CHECK(false);
			}
		}
	}
}
