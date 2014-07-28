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
    \ingroup TRANSQT
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libciomr/libciomr.h>
#include "MOInfo.h"
#include "Params.h"
#include "globals.h"

namespace psi { namespace transqt {

void destruct_evects(int nirreps, double ***evects);


void cleanup(void)
{
  int i;

  /* Free moinfo Arrays */
  free(moinfo.sopi);
  free(moinfo.orbspi);
  free(moinfo.clsdpi);
  free(moinfo.openpi);
  free(moinfo.virtpi);
  free(moinfo.sosym);
  free(moinfo.orbsym);
  free(moinfo.order);
  free(moinfo.order_alpha);
  free(moinfo.order_beta);
  free(moinfo.corr2pitz);
  free(moinfo.corr2pitz_a);
  free(moinfo.corr2pitz_b);
//  delete [] moinfo.frdocc;
//  delete [] moinfo.fruocc;
  free(moinfo.rstrdocc);
  free(moinfo.rstruocc);
//  free(moinfo.sloc);
//  free(moinfo.stype);
//  free(moinfo.snuc);
  if (params.backtr) {
    free(moinfo.corr2pitz_nofzv);
    free(moinfo.corr2pitz_nofzv_a);
    free(moinfo.corr2pitz_nofzv_b);
  }
  free(moinfo.first_so);
  free(moinfo.last_so);
  free(moinfo.first);
  free(moinfo.last);
  free(moinfo.fstact);
  free(moinfo.lstact);
  for(i=0; i < moinfo.nirreps; i++)
    free(moinfo.labels[i]);
  free(moinfo.labels);
  if(params.ref == "UHF") {
    destruct_evects(params.backtr ? moinfo.backtr_nirreps : moinfo.nirreps, 
		    moinfo.evects_alpha);
    destruct_evects(params.backtr ? moinfo.backtr_nirreps : moinfo.nirreps, 
		    moinfo.evects_beta);
  }
  else {
    destruct_evects(params.backtr ? moinfo.backtr_nirreps : moinfo.nirreps, 
		    moinfo.evects);
  }
  free(moinfo.active);
  if(params.ref == "UHF") {
    free_block(moinfo.scf_vector_alpha);
    free_block(moinfo.scf_vector_beta);
  }
  else free_block(moinfo.scf_vector);
  /* free(moinfo.evals); */
  free(moinfo.oe_ints);
  if(params.ref == "UHF") {
    free(moinfo.fzc_operator_alpha);
    free(moinfo.fzc_operator_beta);
  }
  else free(moinfo.fzc_operator);
  free(moinfo.S);
  if (params.reorder) free(params.moorder);
  free(moinfo.backtr_mo_first);
  free(moinfo.backtr_mo_last);
  free(moinfo.backtr_mo_fstact);
  free(moinfo.backtr_mo_lstact);
  free(moinfo.backtr_mo_orbspi);
  free(moinfo.backtr_mo_active);
  free(moinfo.backtr_ao_first);
  free(moinfo.backtr_ao_last);
  free(moinfo.backtr_ao_orbspi);
  free(moinfo.backtr_ao_orbsym);

  /* Free ioff Array */
  free(ioff);

  /* Free params Arrays */
}

}} // end namespace psi::transqt
