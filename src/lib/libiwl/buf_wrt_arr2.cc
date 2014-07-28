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

/*!
  \file
  \ingroup IWL
*/
#include <cstdio>
#include <cmath>
#include <libciomr/libciomr.h>
#include "iwl.h"
#include "iwl.hpp"

namespace psi {
  
void IWL::write_array2(double *arr, int p, int q, int *rlist, int *slist, int size, 
    int printflag, FILE *out)
{
    int i,idx;
    double value;
    Label *lblptr;
    Value *valptr;

    if (size < 0) {
        printf("(iwl_buf_wrt_arr2): Called with size = %d\n",
            size);
        return;
    }

    if (arr == NULL || rlist == NULL || slist == NULL) {
        printf("(iwl_buf_wrt_arr2): Called with null pointer argument\n");
        return;
    }

    lblptr = labels_;
    valptr = values_;

    for (i=0; i<size; i++) {
        value = *arr++;
        if (fabs(value) > cutoff_) {
            idx = 4 * idx_;
            lblptr[idx++] = (Label) p;
            lblptr[idx++] = (Label) q;
            lblptr[idx++] = (Label) rlist[i];
            lblptr[idx++] = (Label) slist[i];
            valptr[idx_] = (Value) value;

            if(printflag) 
                fprintf(out, "%d %d %d %d %20.10f\n", p, q, 
                rlist[i], slist[i], value);

            idx_++;

            if (idx_ == ints_per_buf_) {
                lastbuf_ = 0;
                inbuf_ = idx_;
                put();
                idx_ = 0;
            }
        } /* end if (fabs(value) > Buf->cutoff) ... */             
    } /* end loop over i */
}

/*!
** IWL_BUF_WRT_ARR2()
**
** This function writes out an array of two-electron
** integrals using the Integrals With Labels file format.  All integrals
** in the input arr have common indices p and q.  r and s indices are
** given in the lists plist and qlist.
** David Sherrill, March 1995
**
** Revised 6/27/96 by CDS for new format
** \ingroup IWL
*/
void iwl_buf_wrt_arr2(struct iwlbuf *Buf, double *arr, int p, int q, 
      int *rlist, int *slist, int size, int printflag, FILE *out)
{
  int i,idx;
  double value;
  Label *lblptr;
  Value *valptr;

  if (size < 0) {
    printf("(iwl_buf_wrt_arr2): Called with size = %d\n",
	   size);
    return;
  }
  
  if (Buf == NULL || arr == NULL || rlist == NULL || slist == NULL) {
    printf("(iwl_buf_wrt_arr2): Called with null pointer argument\n");
    return;
  }
  
  lblptr = Buf->labels;
  valptr = Buf->values;

  for (i=0; i<size; i++) {
    value = *arr++;
    if (fabs(value) > Buf->cutoff) {
      idx = 4 * Buf->idx;
      lblptr[idx++] = (Label) p;
      lblptr[idx++] = (Label) q;
      lblptr[idx++] = (Label) rlist[i];
      lblptr[idx++] = (Label) slist[i];
      valptr[Buf->idx] = (Value) value;
   
      if(printflag) 
	fprintf(out, "%d %d %d %d %20.10f\n", p, q, 
		rlist[i], slist[i], value);
      
      Buf->idx++;
      
      if (Buf->idx == Buf->ints_per_buf) {
	Buf->lastbuf = 0;
	Buf->inbuf = Buf->idx;
	iwl_buf_put(Buf);
	Buf->idx = 0;
      }
      
    } /* end if (fabs(value) > Buf->cutoff) ... */             
  } /* end loop over i */

}

}

