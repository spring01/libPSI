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
    \ingroup CIS
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <libdpd/dpd.h>

namespace psi { namespace cis {

struct onestack {
    double value;
    int i;
    int a;
};

void onestack_insert(struct onestack *stack, double value, int i, int a, int level, int stacklen);

void amp_write_T1(dpdfile2 *T1, int length, FILE *outfile)
{
  int m, h, nirreps, Gia;
  int i, I, a, A, numt1;
  double value;
  struct onestack *t1stack;

  nirreps = T1->params->nirreps;
  Gia = T1->my_irrep;

  t1stack = (struct onestack *) malloc(length * sizeof(struct onestack));
  for(m=0; m < length; m++) { t1stack[m].value = 0; t1stack[m].i = 0; t1stack[m].a = 0; }

  global_dpd_->file2_mat_init(T1);
  global_dpd_->file2_mat_rd(T1);

  numt1 = 0;
  for(h=0; h < nirreps; h++) {

    numt1 += T1->params->rowtot[h] * T1->params->coltot[h^Gia];

    for(i=0; i < T1->params->rowtot[h]; i++) {
      I = T1->params->roworb[h][i];
      for(a=0; a < T1->params->coltot[h^Gia]; a++) {
	A = T1->params->colorb[h][a];
	value = T1->matrix[h][i][a];
	for(m=0; m < length; m++) {
	  if((fabs(value) - fabs(t1stack[m].value)) > 1e-12) {
	    onestack_insert(t1stack, value, I, A, m, length);
	    break;
	  }
	}
      }
    }
  }

  global_dpd_->file2_mat_close(T1);

  for(m=0; m < ((numt1 < length) ? numt1 : length); m++)
    if(fabs(t1stack[m].value) > 1e-6)
      fprintf(outfile, "\t        %3d %3d %20.10f\n", t1stack[m].i, t1stack[m].a, t1stack[m].value);

  free(t1stack);
}

void onestack_insert(struct onestack *stack, double value, int i, int a, int level, int stacklen)
{
  int l;
  struct onestack temp;

  temp = stack[level];

  stack[level].value = value;
  stack[level].i = i;
  stack[level].a = a;

  value = temp.value;
  i = temp.i;
  a = temp.a;

  for(l=level; l < stacklen-1; l++) {
    temp = stack[l+1];

    stack[l+1].value = value;
    stack[l+1].i = i;
    stack[l+1].a = a;

    value = temp.value;
    i = temp.i;
    a = temp.a;
  }
}


}} // namespace psi::cis
