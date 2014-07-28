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

/*! \defgroup CIS cis: Compute CI singles for excited states */

/*! \file
    \ingroup CIS
    \brief Compute CI singles for excited states
*/

#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#include "Local.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace cis {

void Fab_build(void)
{
  dpdfile2 F;
  dpdbuf4 D, T2;

  if(params.ref == 0) { /** RHF **/
    global_dpd_->file2_init(&F, PSIF_CC_MISC, 0, 1, 1, "FAB");
    global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 0, 5, 0, 5, 0, "D 2<ij|ab> - <ij|ba>");
    global_dpd_->buf4_init(&T2, PSIF_CC_MISC, 0, 0, 5, 0, 5, 0, "MP2 tIjAb");
    global_dpd_->contract442(&T2, &D, &F, 3, 3, -1, 0);
    global_dpd_->buf4_close(&T2);
    global_dpd_->buf4_close(&D);
    global_dpd_->file2_close(&F);
  }
  else if(params.ref == 2) { /** UHF **/
    global_dpd_->file2_init(&F, PSIF_CC_MISC, 0, 1, 1, "FAB");

    global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 2, 5, 2, 5, 0, "D <IJ||AB> (I>J,AB)");
    global_dpd_->buf4_init(&T2, PSIF_CC_MISC, 0, 2, 5, 2, 7, 0, "MP2 tIJAB");
    global_dpd_->contract442(&T2, &D, &F, 3, 3, -1, 0);
    global_dpd_->buf4_close(&T2);
    global_dpd_->buf4_close(&D);

    global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 22, 28, 22, 28, 0, "D <Ij|Ab>");
    global_dpd_->buf4_init(&T2, PSIF_CC_MISC, 0, 22, 28, 22, 28, 0, "MP2 tIjAb");
    global_dpd_->contract442(&T2, &D, &F, 2, 2, -1, 1);
    global_dpd_->buf4_close(&T2);
    global_dpd_->buf4_close(&D);

    global_dpd_->file2_close(&F);

    global_dpd_->file2_init(&F, PSIF_CC_MISC, 0, 3, 3, "Fab");

    global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 12, 15, 12, 15, 0, "D <ij||ab> (i>j,ab)");
    global_dpd_->buf4_init(&T2, PSIF_CC_MISC, 0, 12, 15, 12, 17, 0, "MP2 tijab");
    global_dpd_->contract442(&T2, &D, &F, 3, 3, -1, 0);
    global_dpd_->buf4_close(&T2);
    global_dpd_->buf4_close(&D);

    global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 22, 28, 22, 28, 0, "D <Ij|Ab>");
    global_dpd_->buf4_init(&T2, PSIF_CC_MISC, 0, 22, 28, 22, 28, 0, "MP2 tIjAb");
    global_dpd_->contract442(&T2, &D, &F, 3, 3, -1, 1);
    global_dpd_->buf4_close(&T2);
    global_dpd_->buf4_close(&D);

    global_dpd_->file2_close(&F);
  }
}

}} // namespace psi::cis
