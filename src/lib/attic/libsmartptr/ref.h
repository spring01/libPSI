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

#ifndef smartptr_ref_h
#define smartptr_ref_h

//do not use default Boost IO stream behavior on smart ptrs
#define BOOST_NO_IOSTREAM
//#define redefine_size_t
//#define custom_size_t unsigned int

namespace smartptr {

/**
    @class Countable The base class from which all reference counted
                     classes must derive
*/
class Countable {
    
    private:
        unsigned int refcount_;

    public:
        Countable();

        /**
            Increments the reference count by one.
            @return The new reference count
        */
        unsigned int incref();

        /**
            Decrements the reference count by one.  Aborts
            if reference count is already zero.
            @return The new reference count
        */
        unsigned int decref();

        /**
            @return The current reference count
        */
        unsigned int nref() const;

        virtual ~Countable(){}

};

}


namespace boost {

/**
    Boost function to increase the reference count of const objects
    @param c Reference counted object
*/
void
intrusive_ptr_add_ref(const smartptr::Countable* c);

/**
    Boost function to increase the reference count of objects
    @param c Reference counted object
*/
void
intrusive_ptr_add_ref(smartptr::Countable* c);

/**
    Boost function to decrease the reference count of const objects
    @param c Reference counted object
*/
void
intrusive_ptr_release(const smartptr::Countable* c);

/**
    Boost function to decrease the reference count of objects
    @param c Reference counted object
*/
void
intrusive_ptr_release(smartptr::Countable* c);

}




#include "boost/intrusive_ptr.hpp"

#ifdef redefine_size_t
#undef size_t
#endif

#endif


