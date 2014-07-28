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
    \ingroup CCDENSITY
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#include "Frozen.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccdensity {
#include <physconst.h>

#define _hartree2nm 0.02194746313710

void td_print(void)
{
  int i;

  fprintf(outfile,"\n\t                   Excitation Energy          OS       RS        RS\n");
  fprintf(outfile,"\tState   (eV)    (cm^-1)    (nm)     (au)              (l,au)   (v,au)\n");
  for(i=0; i<params.nstates; i++) {
    fprintf(outfile,"\t %d%3s %7.3lf %9.1lf %7.1lf %10.6lf %8.4lf %8.4lf %8.4lf\n",
            td_params[i].root+1,moinfo.labels[td_params[i].irrep],
            td_params[i].cceom_energy*pc_hartree2ev,
            td_params[i].cceom_energy*pc_hartree2wavenumbers,
            1/(td_params[i].cceom_energy*_hartree2nm),
            td_params[i].cceom_energy, td_params[i].OS,
            td_params[i].RS_length,td_params[i].RS_velocity);
  }
  fprintf(outfile,"\n");
}

}} // namespace psi::ccdensity
