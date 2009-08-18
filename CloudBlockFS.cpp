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

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdexcept>
#include <memory>
#include "DataStore.h"
#include "FileDataStore.h"
#include "BlockStorageDevice.h"

using namespace cloudblockfs;

static std::auto_ptr<BlockStorageDevice> blockstore;
#define CLOUDBLOCK_DEVICE_NAME "cloudblockdisk"

static int cloudblockfs_fgetattr(const char *path, struct stat *stbuf,
                  struct fuse_file_info *fi) 
{
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 3;
		return 0;
	}
	if (strcmp(path, "/" CLOUDBLOCK_DEVICE_NAME) == 0) { /* The root directory of our file system. */
		stbuf->st_dev = 1;
		stbuf->st_ino = 1;
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_size = blockstore->GetDiskSize();
		stbuf->st_blksize = blockstore->GetBlockSize();
		return 0;
	}
	return -ENOENT;
}

static int cloudblockfs_getattr(const char *path, struct stat *stbuf) 
{
	return cloudblockfs_fgetattr(path, stbuf, NULL);
}

static int cloudblockfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) 
{
	if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
		return -ENOENT;

	filler(buf, ".", NULL, 0);           /* Current directory (.)  */
	filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
	filler(buf, "cloudblockdisk", NULL, 0);

	return 0;
}


static int cloudblockfs_open(const char *path, struct fuse_file_info *fi) 
{
	return 0;
}

int cloudblockfs_release(const char *, struct fuse_file_info *)
{
	return 0;
}

static int cloudblockfs_flush(const char *path, struct fuse_file_info *fi)
{
	return 0;
}

static int cloudblockfs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
	if(strcmp(path, "/" CLOUDBLOCK_DEVICE_NAME) == 0) {
		try {
			int64_t disk_size = blockstore->GetDiskSize();
			if(offset > disk_size) return 0;
			if(size + offset > disk_size) {
				size = disk_size - offset;
			}
			
			blockstore->Read(buf,size,offset);
		} catch(const std::runtime_error& ) {
			return -EIO;
		}
		return size;
	} 
	return size;
}

static int cloudblockfs_write(const char *path, const char *buf, size_t size,
               off_t offset, struct fuse_file_info *fi) {
	if(strcmp(path, "/" CLOUDBLOCK_DEVICE_NAME) == 0) {
		try {
			if(size + offset > blockstore->GetMaxDiskSize()) return -ENOSPC;
		
			if(size + offset > blockstore->GetDiskSize()) {
				blockstore->Truncate(size + offset);
			}
			
			blockstore->Write(buf,size,offset);
		} catch(const std::runtime_error& ) {
			return -EIO;
		}
		return size;
	}
	return size;
}

static int cloudblockfs_statfs(const char *path, struct statvfs *stbuf) 
{
	memset(stbuf, 0, sizeof(*stbuf));
	stbuf->f_files = 2;  /* For . and .. in the root */
	return 0;
}

static int cloudblockfs_utime(const char *path, struct utimbuf *tv)
{
	return 0;
}

static int cloudblockfs_chmod(const char *path,mode_t mode)
{
	return 0;
}

static int cloudblockfs_chown(const char *path, uid_t uid, gid_t gid)
{
	return 0;
}

int cloudblockfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
	return 0;
}

int cloudblockfs_access(const char *path, int amode)
{
	return 0;
}

int cloudblockfs_create(const char *, mode_t, struct fuse_file_info *)
{
	return 0;
}

int cloudblockfs_ftruncate(const char *path, off_t size, struct fuse_file_info *fi)
{
	if(strcmp(path, "/" CLOUDBLOCK_DEVICE_NAME) == 0) {
		try {
			blockstore->Truncate(size);
		} catch(const std::runtime_error& ) {
			return -EIO;
		}
	}
	
	return 0;
}

int main(int argc, char* argv[], char* envp[], char** exec_path) 
{
	umask(0);
	
	// initialize blockstore
	blockstore.reset(new BlockStorageDevice(new FileDataStore("/Users/sound/Desktop/cloudblockfs/build/Debug/store")));
	if(!blockstore->IsValid()) 
		blockstore->Format(1024,3);
	
	struct fuse_operations ops;
	memset(&ops,0,sizeof(ops));
	
	ops.getattr = cloudblockfs_getattr;
	ops.fgetattr = cloudblockfs_fgetattr;
	ops.readdir = cloudblockfs_readdir;
	ops.open = cloudblockfs_open;
	ops.release = cloudblockfs_release;
	ops.flush = cloudblockfs_flush;
	ops.read = cloudblockfs_read;
	ops.write = cloudblockfs_write;
	ops.statfs = cloudblockfs_statfs;
	ops.utime = cloudblockfs_utime;
	ops.chmod = cloudblockfs_chmod;
	ops.chown = cloudblockfs_chown;
	ops.fsync = cloudblockfs_fsync;
	ops.access = cloudblockfs_access;
	ops.create = cloudblockfs_create;
	ops.ftruncate = cloudblockfs_ftruncate;
	
	return fuse_main(argc, argv, &ops, NULL);
}
