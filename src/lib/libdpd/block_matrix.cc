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

/*! \file
    \ingroup DPD
    \brief Enter brief description of file here
*/

/*
** dpd_block_matrix(): Allocates a contiguous block of memory for use
** as a 2-dimensional array.
**
** The memory is provided as a contiguous block of doubles, with an
** additional array of pointers used to mark the beginning of each
** row.  This allows transparent 2-dimensional-array style access, but
** keeps memory together such that it could be used in FORTRAN BLAS
** routines, for example.
**
** Prior to allocation, this routine checks the current status of
** dpd_main.memfree to make sure the malloc() request will not
** overrun the user-specified memory limits.  If there is insufficient
** memory available, entries are deleted from the dpd_file4_cache (in
** LRU order) until the memory limits are satisfied.  If, after
** deletion of the entire dpd_file4_cache (or at least until no other
** zero-priority entries remain), there is still insufficient memory
** available to satisfy the request, a NULL pointer is returned to the
** caller, indicating that either an out-of-core algorithm must be
** used, or the caller must exit().
**
** TDC, 6/24/00
*/

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <libqt/qt.h>
#include "dpd.h"

namespace psi {

double **
DPD::dpd_block_matrix(size_t n, size_t m)
{
    int i;
    double **A, *B;

#ifdef DPD_TIMER
    timer_on("block_mat");
#endif

    A = NULL;  B = NULL;

    size_t size = m * n;

    while((dpd_main.memory - dpd_main.memused - size) < 0) {
        /* Delete cache entries until there's enough memory or no more cache */

        /* Priority-based cache */
        if(dpd_main.cachetype == 1) {
            if(file4_cache_del_low()) {
                file4_cache_print(stderr);
                fprintf(stderr, "dpd_block_matrix: n = %zd  m = %zd\n", n, m);
                dpd_error("dpd_block_matrix: No memory left.", stderr);
            }
        }

        /* Least-recently-used cache */
        else if(dpd_main.cachetype == 0) {
            if(file4_cache_del_lru()) {
                file4_cache_print(stderr);
                fprintf(stderr, "dpd_block_matrix: n = %zd  m = %zd\n", n, m);
                dpd_error("dpd_block_matrix: No memory left.", stderr);
            }
        }

        else dpd_error("LIBDPD Error: invalid cachetype.", stderr);
    }

    if(!m || !n) {
#ifdef DPD_TIMER
        timer_off("block_mat");
#endif
        return(NULL);
    }

    if((A = (double **) malloc(n * sizeof(double *)))==NULL) {
        fprintf(stderr,"dpd_block_matrix: trouble allocating memory \n");
        fprintf(stderr,"n = %zd  m = %zd\n",n, m);
        exit(PSI_RETURN_FAILURE);
    }

    /* Allocate the main block here */
    /* NB: If we delete the entire cache and STILL get NULL from malloc(), */
    /* we're either out of real memory or the heap is seriously fragmented */
    while((B = (double *) malloc(size * sizeof(double))) == NULL) {
        /* Priority-based cache */
        if(dpd_main.cachetype == 1) {
            if(file4_cache_del_low()) {
                file4_cache_print(stderr);
                fprintf(stderr, "dpd_block_matrix: n = %zd  m = %zd\n", n, m);
                dpd_error("dpd_block_matrix: No memory left.", stderr);
            }
        }

        /* Least-recently-used cache */
        else if(dpd_main.cachetype == 0) {
            if(file4_cache_del_lru()) {
                file4_cache_print(stderr);
                fprintf(stderr, "dpd_block_matrix: n = %zd  m = %zd\n", n, m);
                dpd_error("dpd_block_matrix: No memory left.", stderr);
            }
        }
    }

    /*  memset((void *) B, 0, m*n*sizeof(double)); */
    //bzero(B, m*n*sizeof(double));
    ::memset(B, '\0', m*n*sizeof(double));

    for (i = 0; i < n; i++) A[i] = &(B[i*m]);

    /* Increment the global memory counter */
    dpd_main.memused += n*m;

#ifdef DPD_TIMER
    timer_off("block_mat");
#endif

    return(A);
}

void DPD::free_dpd_block(double **array, size_t n, size_t m)
{
    size_t size =  m * n;
    if(array == NULL) return;
    free(array[0]);
    free(array);
    /* Decrement the global memory counter */
    dpd_main.memused -= size;
}

}
