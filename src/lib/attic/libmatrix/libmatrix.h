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

#include <libmints/dimension.h>
#include <string>

#include "detail.h"
#include "libmints_wrapper.h"
#include "libdist_wrapper.h"
#include "elemental_wrapper.h"

namespace psi { namespace libmatrix {

// Define the default matrix type. In general, developers should just use this type.
#if !defined(HAVE_MPI) && !defined(HAVE_ELEMENTAL)
    typedef libmints_globals            matrix_globals;
    typedef libmints_matrix_wrapper     matrix;       // No mpi and no elemental
    typedef libmints_vector_wrapper     vector;       // No mpi and no elemental
#elif defined(HAVE_MPI) && !defined(HAVE_ELEMENTAL)
    typedef libdist_globals             matrix_globals;
    typedef libdist_matrix_wrapper      matrix;       // mpi and no elemental
    //typedef libdist_vector_wrapper      vector;       // mpi and no elemental doesn't handle vectors, yet.
#elif defined(HAVE_MPI) && defined(HAVE_ELEMENTAL)
    typedef libelemental_globals        matrix_globals;
    typedef libelemental_matrix_wrapper matrix;       // mpi and elemental
    typedef libelemental_vector_wrapper vector;
#endif

    // Provide typedefs of all wrappers to the developer. In some cases using a serial_matrix is preferred.
    typedef libmints_matrix_wrapper     serial_matrix;
    typedef libmints_vector_wrapper     serial_vector;
    typedef libdist_matrix_wrapper      dist_matrix;
    typedef libelemental_matrix_wrapper elemental_matrix;
    typedef libelemental_vector_wrapper elemental_vector;

    // A templated version of create matrix. Developer must provide template type, e.g.,
    //    serial_matrix mat = create_specific_matrix<serial_matrix>(...);
    template <typename T>
    T create_specific_matrix(const std::string& name, const Dimension& m, const Dimension& n) {
        return detail::create_matrix<T>::create(name, m, n);
    }

    // Create matrix function using the default "matrix" typedef'ed above.
    matrix create_matrix(const std::string& name, const Dimension& m, const Dimension& n) {
        return create_specific_matrix<matrix>(name, m, n);
    }

    template <typename T>
    T create_specific_vector(const std::string& name, const Dimension& m) {
        return detail::create_vector<T>::create(name, m);
    }

    // Create vector function using the default "vector" typedef'ed above.
    vector create_vector(const std::string& name, const Dimension& m) {
        return create_specific_vector<vector>(name, m);
    }
}}

