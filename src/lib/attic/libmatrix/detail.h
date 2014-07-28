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

#if !defined(LIBMATRIX_DETAILS_H)
#define LIBMATRIX_DETAILS_H

namespace psi{ namespace libmatrix {

    namespace detail {
        template <typename matrix_type>
        struct create_matrix {
            static matrix_type create(const std::string& name, const Dimension& m, const Dimension& n) {
                return matrix_type (name, m, n);
            }
        };

        // In the case that the generic template isn't sufficient to create the matrix
        // write a specialization of it, e.g:
        //
        // template <>
        // struct create_matrix<libmints_matrix_wrapper> {
        //     typedef libmints_matrix_wrapper matrix_type;
        //     matrix_type create(const std::string& name, const Dimension& m, const Dimension& n) {
        //         return matrix_type(name, m, n);
        //     }
        // }

        template <typename vector_type>
        struct create_vector {
            static vector_type create(const std::string& name, const Dimension& m) {
                return vector_type (name, m);
            }
        };

        // If an implementation is not available due to configuration, derive from this class
        // so that compiler errors will be generated if the developer attempts to use them.
        class is_not_available
        {
        protected:
            is_not_available() {}
            ~is_not_available() {}
        private:  // emphasize the following members are private
            is_not_available( const is_not_available& );
            const is_not_available& operator=( const is_not_available& );
        };
    }

    // Macro for defining unavailable matrix wrappers
    #define UNAVAILABLE_MATRIX(type) struct type : public detail::is_not_available { \
        void print() const; \
        void fill(double); \
        void add(const type&); \
    private: \
        type(); \
        type(const std::string&, const Dimension&, const Dimension&); \
    };

}} // end namespaces

#endif

