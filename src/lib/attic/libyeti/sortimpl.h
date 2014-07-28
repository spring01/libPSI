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

#ifndef yeti_sortimpl_h
#define yeti_sortimpl_h

#include "sort.h"
#include "permutation.h"

#ifdef redefine_size_t
#define size_t custom_size_t
#endif

namespace yeti {

template <class T>
static void
qs(T* item, int* index, int left, int right)
{
    register int i,j;
    T x;
    int y;

    i=left; j=right;
    x=item[index[(left+right)/2]];

    do {
        while(item[index[i]]<x && i<right) i++;
        while(x<item[index[j]] && j>left) j--;

        if (i<=j) {
            if (item[index[i]] != item[index[j]]) {
                y=index[i];
                index[i]=index[j];
                index[j]=y;
            }
            i++; j--;
        }
    } while(i<=j);

    if (left<j) qs(item,index,left,j);
    if (i<right) qs(item,index,i,right);
}

template <class T>
void
quicksort(T* item, int* index, uli n)
{
    int i;
    if (n<=0) return;
    for (i=0; i<n; ++i) {
        index[i] = i;
    }
    qs<T>(item,index,0,n-1);
}

template <class T, class U>
void
Sort::accumulate(const T* src, U* dst, U scale) const
{
    if (scale == 0) //just use the permutation sign
    {
        scale = p_->sign();
    }
    //sending tmp variable is necessary since value is passed by reference
    U* targetptr = dst;
    stride_accumulate<T,U>(0, src, targetptr, scale * p_->sign());
}

template <class T, class U>
void
Sort::stride_accumulate(usi sortlevel, const T* src, U*& target, U scale) const
{
      //not yet finished
    uli src_stride = lengths_[sortlevel];
    uli nstrides = nstrides_[sortlevel];

    ++sortlevel;
    if (sortlevel == nindex_) //finish off
    {
        const T* srcptr = src;
        U* dstptr = target;
        if (src_stride == 1)
        {
            for (uli i=0; i < nstrides; ++srcptr, ++dstptr, ++i)
            {
                (*dstptr) += (*srcptr) * scale;
            }
        }
        else
        {
            for (uli i=0; i < nstrides; srcptr += src_stride, ++dstptr, ++i)
            {
                (*dstptr) += (*srcptr) * scale;
            }
        }

        //increment the target pointer and return
        target += nstrides;
        return;
    }
    else
    {
        for (uli i=0; i < nstrides; ++i, src += src_stride)
            stride_accumulate(sortlevel, src, target, scale);
    }
}

template <class T>
T*
Sort::sort(const T* src) const
{
    T* dst = new T[ntot_];
    sort<T>(src, dst);
    return dst;
}

template <class T, class U>
void
Sort::sort_noscale(const T* src, U* dst) const
{
    //sending tmp variable is necessary since value is passed by reference
    U* targetptr = dst;
    stride_sort(0, src, targetptr);
}

template <class T, class U, class Functor>
void
Sort::sort_functor(T* src, U* dst, Functor& functor) const
{
    U* targetptr = dst;
    stride_functor(0, src, targetptr, functor);
}

template <class T, class U>
void
Sort::sort(const T* src, U* dst) const
{
    sort_noscale<T,U>(src, dst);

    if (p_->sign() == -1) //scale all of the elements
    {
        U* dptr = dst;
        for (uli i=0; i < ntot_; ++i, ++dptr)
            (*dptr) *= -1;
    }
}

template <class T, class U>
void
Sort::stride_sort(usi sortlevel, const T* src, U*& target) const
{
      //not yet finished
    uli src_stride = lengths_[sortlevel];
    uli nstrides = nstrides_[sortlevel];

    ++sortlevel;
    if (sortlevel == nindex_) //finish off
    {
        if (src_stride == 1)
        {
            ::memcpy(target, src, nstrides * sizeof(T));
        }
        else
        {
            const T* srcptr = src;
            U* dstptr = target;
            for (uli i=0; i < nstrides; srcptr += src_stride, ++dstptr, ++i)
            {
                T entry = *srcptr;
                (*dstptr) = entry;
            }
        }

        //increment the target pointer and return
        target += nstrides;
        return;
    }
    else
    {
        for (uli i=0; i < nstrides; ++i, src += src_stride)
            stride_sort(sortlevel, src, target);
    }
}

template <class T, class U, class Functor>
void
Sort::stride_functor(
    usi sortlevel,
    T* src,
    U*& target,
    Functor& functor
) const
{
      //not yet finished
    uli src_stride = lengths_[sortlevel];
    uli nstrides = nstrides_[sortlevel];

    ++sortlevel;
    if (sortlevel == nindex_) //finish off
    {
        T* srcptr = src;
        U* dstptr = target;
        for (uli i=0; i < nstrides; srcptr += src_stride, ++dstptr, ++i)
        {
            functor.sort(*srcptr, *dstptr);
        }

        //increment the target pointer and return
        target += nstrides;
        return;
    }
    else
    {
        for (uli i=0; i < nstrides; ++i, src += src_stride)
            stride_functor(sortlevel, src, target, functor);
    }
}

}

#ifdef redefine_size_t
#undef size_t
#endif

#endif

