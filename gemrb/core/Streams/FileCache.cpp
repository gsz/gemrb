/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "FileCache.h"

#include "Compressor.h"
#include "Interface.h"
#include "PluginMgr.h"
#include "Streams/FileStream.h"
#if defined(SUPPORTS_MEMSTREAM)
#include "Streams/MappedFileMemoryStream.h"
#endif
#include "System/VFS.h"

namespace GemRB {

DataStream* CacheCompressedStream(DataStream *stream, const std::string& filename, int length, bool overwrite)
{
	if (!core->IsAvailable(PLUGIN_COMPRESSION_ZLIB)) {
		Log(ERROR, "FileCache", "No Compression Manager Available. Cannot Load Compressed File.");
		return NULL;
	}

	char fname[_MAX_PATH];
	ExtractFileFromPath(fname, filename.c_str());
	char path[_MAX_PATH];
	PathJoin(path, core->config.CachePath, fname, nullptr);

	if (overwrite || !file_exists(path)) {
		FileStream out;
		if (!out.Create(path)) {
			Log(ERROR, "FileCache", "Cannot write {}.", path);
			return NULL;
		}

		PluginHolder<Compressor> comp = MakePluginHolder<Compressor>(PLUGIN_COMPRESSION_ZLIB);
		if (comp->Decompress(&out, stream, length) != GEM_OK)
			return NULL;
	} else {
		stream->Seek(length, GEM_CURRENT_POS);
	}
#if defined(SUPPORTS_MEMSTREAM)
	return new MappedFileMemoryStream{path};
#else
	return FileStream::OpenFile(path);
#endif
}

}
