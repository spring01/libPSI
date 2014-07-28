/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

#ifndef yeti_dataabstract_h
#define yeti_dataabstract_h

#include "class.h"

#include "data.hpp"
#include "cache.hpp"
#include "sort.hpp"
#include "index.hpp"
#include "permutation.hpp"
#include "tensor.hpp"
#include "thread.hpp"
#include "filler.hpp"

#include "mallocimpl.h"
#include "yetiobject.h"
#include "cache.h"

#define MALLOC_CACHE_BLOCKS 0

#ifdef redefine_size_t
#define size_t custom_size_t
#endif

namespace yeti {


class StorageBlock :
    public Malloc<StorageBlock>
{

    protected:
        char* data_;

        size_t size_;

        bool retrieved_;

    public:

        StorageBlock(size_t size);

        virtual ~StorageBlock();

        char* data() const;

        size_t size() const;

        virtual void clear() = 0;

        virtual bool retrieve() = 0;

        virtual void release() = 0;

        virtual void commit() = 0;

        virtual bool is_cached() const = 0;

        virtual bool is_retrieved() const = 0;

        void memset();

};

/**
    @param MemoryBlock
    Block of data that is persistent in memory
*/
class InCoreBlock :
    public StorageBlock
{

    public:
        InCoreBlock(char* data, size_t size);

        ~InCoreBlock();

        void commit();

        void clear();

        bool retrieve();

        bool is_cached() const;

        bool is_retrieved() const;

        void release();


};

/**
    @class CachedDataBlock
    Block of data that is allocated from a central cache system.
    Caching is used to minimize fetches from disk/remote locations.
*/
class CachedStorageBlock :
    public StorageBlock
{

    protected:
        friend class DataCacheEntry;

        /**
            A data cache holding blocks of the appropriate size
            for this data block
        */
        DataCache* cache_;

        /**
            A cache entry containing a block of data that
            this cache entry is linked to
        */
        DataCacheEntry* cache_entry_;

        Cachable* cache_item_;


    public:
        CachedStorageBlock(
            Cachable* parent,
            DataCache* cache
        );

        ~CachedStorageBlock();

        void assign_location(char* data);

        Cachable* get_cache_item() const;

        bool is_cached() const;

        bool retrieve();

        void release();

        void clear();

        DataCacheEntry* get_entry() const;

        virtual void commit();

        bool is_retrieved() const;

};


class DiskBuffer :
    public YetiRuntimeCountable
{

    private:
        std::string filename_;

        int fileno_;

        size_t size_;

    public:
        DiskBuffer(const std::string& filename);

        ~DiskBuffer();

        size_t size() const;

        /**
            @return The buffer offset the region begins at
        */
        size_t allocate_region(size_t size);

        void read(size_t offset, size_t size, char* buffer);

        void write(size_t offset, size_t size, const char* buffer);

        const std::string& filename() const;

};

class LocalDiskBlock :
    public CachedStorageBlock
{
    private:
        DiskBufferPtr buffer_;

        size_t offset_;

    public:
        LocalDiskBlock(
            Cachable* parent,
            DataCache* cache,
            const DiskBufferPtr& buffer
        );

        void commit();

        void fetch_data();

};


}

#ifdef redefine_size_t
#undef size_t
#endif

#endif
