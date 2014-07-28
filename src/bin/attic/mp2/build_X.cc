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
    \ingroup MP2
    \brief Enter brief description of file here 
*/
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

namespace psi{ namespace mp2{

void build_X(void)
{
  dpdfile2 X, I, X2;

  if(params.ref == 0) { 

    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 1, 0, "I'AI");
    global_dpd_->file2_copy(&I, PSIF_CC_OEI, "XAI");
    global_dpd_->file2_close(&I);

    global_dpd_->file2_init(&X, PSIF_CC_OEI, 0, 1, 0, "XAI");
    global_dpd_->file2_scm(&X, -1.0);
    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 0, 1, "I'IA");
    global_dpd_->file2_axpy(&I, &X, 1.0, 1);
    global_dpd_->file2_close(&I);
    global_dpd_->file2_close(&X);

    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 1, 0, "I'ai");
    global_dpd_->file2_copy(&I, PSIF_CC_OEI, "Xai");
    global_dpd_->file2_close(&I);

    global_dpd_->file2_init(&X, PSIF_CC_OEI, 0, 1, 0, "Xai");
    global_dpd_->file2_scm(&X, -1.0);
    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 0, 1, "I'ia");
    global_dpd_->file2_axpy(&I, &X, 1.0, 1);
    global_dpd_->file2_close(&I);
    global_dpd_->file2_close(&X);

    /* Build spatial orbital version of X for Zvector:
       X(A,I) = 1/2[X(A,I)+X(a,i)]  */
    global_dpd_->file2_init(&X, PSIF_CC_OEI, 0, 1, 0, "XAI");
    global_dpd_->file2_copy(&X, PSIF_CC_MISC, "X(A,I)");
    global_dpd_->file2_close(&X);
    global_dpd_->file2_init(&X, PSIF_CC_MISC, 0, 1, 0, "X(A,I)");
    global_dpd_->file2_init(&X2, PSIF_CC_OEI, 0, 1, 0, "Xai");
    global_dpd_->file2_axpy(&X2, &X, 1.0, 0);
    global_dpd_->file2_close(&X2);
    global_dpd_->file2_scm(&X, 0.5);
    global_dpd_->file2_close(&X);
  }
  else if(params.ref == 2) { 

    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 1, 0, "I'AI");
    global_dpd_->file2_copy(&I, PSIF_CC_OEI, "XAI");
    global_dpd_->file2_close(&I);

    global_dpd_->file2_init(&X, PSIF_CC_OEI, 0, 1, 0, "XAI");
    global_dpd_->file2_scm(&X, -1.0);
    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 0, 1, "I'IA");
    global_dpd_->file2_axpy(&I, &X, 1.0, 1);
    global_dpd_->file2_close(&I);
    global_dpd_->file2_close(&X);

    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 3, 2, "I'ai");
    global_dpd_->file2_copy(&I, PSIF_CC_OEI, "Xai");
    global_dpd_->file2_close(&I);

    global_dpd_->file2_init(&X, PSIF_CC_OEI, 0, 3, 2, "Xai");
    global_dpd_->file2_scm(&X, -1.0);
    global_dpd_->file2_init(&I, PSIF_CC_OEI, 0, 2, 3, "I'ia");
    global_dpd_->file2_axpy(&I, &X, 1.0, 1);
    global_dpd_->file2_close(&I);
    global_dpd_->file2_close(&X);
  }

}

}} /* End namespaces */
