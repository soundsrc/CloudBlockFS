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
#ifndef __cloudblockfs_Exception_h
#define __cloudblockfs_Exception_h

#include <stdexcept>

namespace cloudblockfs 
{
#define DEFINE_EXCEPTION(Exception) \
	class Exception : public std::runtime_error \
	{ \
	public: \
		Exception(const std::string& err) : std::runtime_error(err) { } \
	}
	
	DEFINE_EXCEPTION(FileIOException);
	DEFINE_EXCEPTION(OutOfDiskSpaceException);
	DEFINE_EXCEPTION(FileNotFoundException);
	DEFINE_EXCEPTION(WriteErrorException);
	DEFINE_EXCEPTION(ReadErrorException);
	DEFINE_EXCEPTION(InvalidArgumentException);
	
#undef DEFINE_EXCEPTION
}

#endif
