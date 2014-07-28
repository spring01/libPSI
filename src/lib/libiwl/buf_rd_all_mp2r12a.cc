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

#define MIN0(a,b) (((a)<(b)) ? (a) : (b))
#define MAX0(a,b) (((a)>(b)) ? (a) : (b))
#define INDEX(i,j) ((i>j) ? (((i)*((i)+1))/2+(j)) : (((j)*((j)+1))/2+(i)))
  
int IWL::read_all_mp2r12a(double *ints, int *ioff_lt, int *ioff_rt, 
    int bra_ket_symm, int *, int printflg, FILE *out)
{
    int lastbuf;
    Label *lblptr;
    Value *valptr;
    int idx, p, q, r, s;
    long int pq, rs, pqrs;

    lblptr = labels_;
    valptr = values_;

    lastbuf = lastbuf_;

    for (idx=4*idx_; idx_ < inbuf_; idx_++) {
        p = (int) lblptr[idx++];
        q = (int) lblptr[idx++];
        r = (int) lblptr[idx++];
        s = (int) lblptr[idx++];

        pq = ioff_lt[p] + q;
        rs = ioff_rt[r] + s;

        pqrs = INDEX(pq,rs);

        if (bra_ket_symm) /*! ERIs or R12-integrals */
            ints[pqrs] = (double) valptr[idx_];
        else { /*! (ip|[T1+T2,r12]|jq) = -[(ip|[r12,T1]|jq) + (jq|[r12,T2]|ip)] */
            if (pq != rs)
                ints[pqrs] -= (double) valptr[idx_];
            else
                ints[pqrs] -= (double) 2.0*valptr[idx_];
        }

        if (printflg) 
            fprintf(out, "<%2d %2d %2d %2d [%2ld][%2ld] [[%3ld]] = %20.10f\n",
            p, q, r, s, pq, rs, pqrs, ints[pqrs]) ;

    } /*! end loop through current buffer */

     /*! read new PSI buffers */
    while (!lastbuf) {
        fetch();
        lastbuf = lastbuf_;

        for (idx=4*idx_; idx_ < inbuf_; idx_++) {
            p = (int) lblptr[idx++];
            q = (int) lblptr[idx++];
            r = (int) lblptr[idx++];
            s = (int) lblptr[idx++];

            pq = ioff_lt[p] + q;
            rs = ioff_rt[r] + s;

            pqrs = INDEX(pq,rs);

            if (bra_ket_symm) /*! ERIs or R12-integrals */
                ints[pqrs] = (double) valptr[idx_];
            else { /*! (ip|[T1+T2,r12]|jq) = -[(ip|[r12,T2]|jq)+(jq|[r12,T2]|ip)] */
                if (pq != rs)
                    ints[pqrs] -= (double) valptr[idx_];
                else
                    ints[pqrs] -= (double) 2.0*valptr[idx_];
            }

            if (printflg) 
                fprintf(out, "<%d %d %d %d [%ld][%ld] [[%ld]] = %20.10f\n",
                p, q, r, s, pq, rs, pqrs, ints[pqrs]) ;

        } /*! end loop through current buffer */

    } /*! end loop over reading buffers */

    return(0); /*! we must have reached the last buffer at this point */
}

/*!
** iwl_buf_rd_all_mp2r12a()
**
** Read from an Integrals With Labels formatted buffer.
** The buffer must have been initialized with iwl_buf_init().
**
**    \param Buf           =  IWL Buffer to read from (already initialized)
**    \param ints          =  memory buffer to put integrals into
**    \param ioff_lt       =  ioff array for the left pair of indices (p and q)
**    \param ioff_rt       =  ioff array for the right pair of indices (r and s)
**    \param bra_ket_symm  =  if 1, then these are ERI or R12 integrals, read
**                     them in as usual, else these are [r12,T2] integrals -
**                     form [T1+T2,r12] out of these.
**
**    WARNING - if bra_ket_symm = 0 - ints must be zeroed out!
**
**    \param ioff          =  the ioff array to figure the total index pqrs 
**                            from the pair indices pq and rs
**    \param printflg      =  if 1, print integrals as they are read
**    \param out       =  pointer to output file for printing
**
** Returns: 0 if end of file, otherwise 1
** \ingroup IWL
*/
int iwl_buf_rd_all_mp2r12a(struct iwlbuf *Buf, double *ints,
			   int *ioff_lt, int *ioff_rt, int bra_ket_symm, 
                           int *, int printflg, FILE *out)
{
  int lastbuf;
  Label *lblptr;
  Value *valptr;
  int idx, p, q, r, s;
  long int pq, rs, pqrs;
  
  lblptr = Buf->labels;
  valptr = Buf->values;
  
  lastbuf = Buf->lastbuf;
  
  for (idx=4*Buf->idx; Buf->idx<Buf->inbuf; Buf->idx++) {
    p = (int) lblptr[idx++];
    q = (int) lblptr[idx++];
    r = (int) lblptr[idx++];
    s = (int) lblptr[idx++];

    pq = ioff_lt[p] + q;
    rs = ioff_rt[r] + s;

    pqrs = INDEX(pq,rs);

    if (bra_ket_symm) /*! ERIs or R12-integrals */
      ints[pqrs] = (double) valptr[Buf->idx];
    else { /*! (ip|[T1+T2,r12]|jq) = -[(ip|[r12,T1]|jq) + (jq|[r12,T2]|ip)] */
      if (pq != rs)
	ints[pqrs] -= (double) valptr[Buf->idx];
      else
	ints[pqrs] -= (double) 2.0*valptr[Buf->idx];
    }
    
    if (printflg) 
      fprintf(out, "<%2d %2d %2d %2d [%2ld][%2ld] [[%3ld]] = %20.10f\n",
	      p, q, r, s, pq, rs, pqrs, ints[pqrs]) ;
    
  } /*! end loop through current buffer */
  
   /*! read new PSI buffers */
  while (!lastbuf) {
    iwl_buf_fetch(Buf);
    lastbuf = Buf->lastbuf;
    
    for (idx=4*Buf->idx; Buf->idx<Buf->inbuf; Buf->idx++) {
      p = (int) lblptr[idx++];
      q = (int) lblptr[idx++];
      r = (int) lblptr[idx++];
      s = (int) lblptr[idx++];

      pq = ioff_lt[p] + q;
      rs = ioff_rt[r] + s;

      pqrs = INDEX(pq,rs);

      if (bra_ket_symm) /*! ERIs or R12-integrals */
	ints[pqrs] = (double) valptr[Buf->idx];
      else { /*! (ip|[T1+T2,r12]|jq) = -[(ip|[r12,T2]|jq)+(jq|[r12,T2]|ip)] */
	if (pq != rs)
	  ints[pqrs] -= (double) valptr[Buf->idx];
	else
	  ints[pqrs] -= (double) 2.0*valptr[Buf->idx];
      }
      
      if (printflg) 
	fprintf(out, "<%d %d %d %d [%ld][%ld] [[%ld]] = %20.10f\n",
		p, q, r, s, pq, rs, pqrs, ints[pqrs]) ;
      
    } /*! end loop through current buffer */
    
  } /*! end loop over reading buffers */
  
  return(0); /*! we must have reached the last buffer at this point */
}

}

