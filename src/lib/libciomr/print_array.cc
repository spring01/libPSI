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
** \file
** \brief Print a lower-triangle array of doubles
** \ingroup CIOMR
*/

#include <cstdio>

namespace psi {

/*!
** print_array(): Prints a lower-triangle of a symmetric matrix packed as
**  an array of doubles.
**
** \param a   = array (packed lower triangle of matrix) to print
** \param m   = dimension of matrix (mxm)
** \param out = file pointer for output
**
** Returns: none
**
** \ingroup CIOMR
*/
void print_array(double *a, int m, FILE *out)
   {
      int ii,jj,kk,mm,nn,ll;
      int i,j,i1,i2;

      ii=0;jj=0;
L200:
      ii++;
      jj++;
      kk=10*jj;
      nn = kk + kk*(kk-1)/2;
      mm=m;
      if (m > kk) mm=kk;
      ll = 2*(mm-ii+1)+1;
      fprintf (out,"\n");
      for (i=ii; i <= mm; i++) fprintf(out,"       %5d",i);
      fprintf (out,"\n");
      for (i=ii; i <= m; i++) {
         i1=i*(i-1)/2+ii;
         i2=i+i*(i-1)/2;
         if (i2 > nn) i2 = i1+9;
         fprintf (out,"\n%5d",i);
         for (j=i1; j <= i2; j++) {
            fprintf (out,"%12.7f",a[j-1]);
            }
         }
      if (m <= kk) {
         fprintf(out,"\n");
         fflush(out);
         return;
         }
      ii=kk; goto L200;
      }

}

