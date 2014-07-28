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
#include <cmath>
#include <libdpd/dpd.h>
#include <libqt/qt.h>
#include "MOInfo.h"
#include "Params.h"
#include "Local.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace cis {

void local_filter_U2(dpdbuf4 *T2, double lambda);

int U_build(int irrep, int root, double lambda, enum Spin spin)
{
  char lbl[32];
  int iter, h, nirreps, row, col;
  double energy, conv, rms, value;
  dpdfile2 F;
  dpdbuf4 Z, U, Unew, D;

  nirreps = moinfo.nirreps;

  timer_on("Uijab");

  if(params.ref == 0) { /** RHF **/

    /* build initial guess amplitudes */
    sprintf(lbl, "ZIjAb[%d]", irrep);
    global_dpd_->buf4_init(&Z, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
    sprintf(lbl, "UIjAb[%d]", irrep);
    global_dpd_->buf4_copy(&Z, PSIF_CC_MISC, lbl);
    global_dpd_->buf4_close(&Z);

    sprintf(lbl, "UIjAb[%d]", irrep);
    global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
    if(params.local) local_filter_U2(&U, lambda);
    else {
      sprintf(lbl, "dIjAb[%d]", irrep);
      global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      global_dpd_->buf4_dirprd(&D, &U);
      global_dpd_->buf4_close(&D);
    }

    sprintf(lbl, "New UIjAb[%d]", irrep);
    global_dpd_->buf4_copy(&U, PSIF_CC_MISC, lbl);
    global_dpd_->buf4_close(&U);

    /*
    fprintf(outfile, "\n\tSolving for U2(%d)[%d] wave function:\n", root, irrep);
    fprintf(outfile,   "\t-------------------------------------\n");
    */

    conv = 0;
    for(iter=0; iter < params.maxiter; iter++) {

      sprintf(lbl, "ZIjAb[%d]", irrep);
      global_dpd_->buf4_init(&Z, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      sprintf(lbl, "New UIjAb[%d] Increment", irrep);
      global_dpd_->buf4_copy(&Z, PSIF_CC_MISC, lbl);
      global_dpd_->buf4_close(&Z);

      sprintf(lbl, "New UIjAb[%d] Increment", irrep);
      global_dpd_->buf4_init(&Unew, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      sprintf(lbl, "UIjAb[%d]", irrep);
      global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);

      global_dpd_->buf4_axpy(&U, &Unew, -lambda);

      global_dpd_->file2_init(&F, PSIF_CC_OEI, 0, 0, 0, "fIJ");
      global_dpd_->contract424(&U, &F, &Unew, 1, 0, 1, -1, 1);
      global_dpd_->contract244(&F, &U, &Unew, 0, 0, 0, -1, 1);
      global_dpd_->file2_close(&F);

      global_dpd_->file2_init(&F, PSIF_CC_OEI, 0, 1, 1, "fAB");
      global_dpd_->contract244(&F, &U, &Unew, 1, 2, 1, 1, 1);
      global_dpd_->contract424(&U, &F, &Unew, 3, 1, 0, 1, 1);
      global_dpd_->file2_close(&F);

      global_dpd_->buf4_close(&U);

      if(params.local) local_filter_U2(&Unew, lambda);
      else {
	sprintf(lbl, "dIjAb[%d]", irrep);
	global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
	global_dpd_->buf4_dirprd(&D, &Unew);
	global_dpd_->buf4_close(&D);
      }

      rms = 0.0;
      for(h=0; h < nirreps; h++) {
	global_dpd_->buf4_mat_irrep_init(&Unew, h);
	global_dpd_->buf4_mat_irrep_rd(&Unew, h);

	for(row=0; row < U.params->rowtot[h]; row++)
	  for(col=0; col < U.params->coltot[h^irrep]; col++) {
	    value = Unew.matrix[h][row][col];
	    rms += value * value;
	  }

	global_dpd_->buf4_mat_irrep_close(&Unew, h);
      }
      global_dpd_->buf4_close(&Unew);
      rms = sqrt(rms);

      sprintf(lbl, "New UIjAb[%d]", irrep);
      global_dpd_->buf4_init(&Unew, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      sprintf(lbl, "New UIjAb[%d] Increment", irrep);
      global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      global_dpd_->buf4_axpy(&U, &Unew, 1);
      global_dpd_->buf4_close(&U);

      /*      fprintf(outfile, "\titer = %d   RMS = %4.3e\n", iter, rms); */
      if(rms < params.convergence) {
	conv = 1;
	fprintf(outfile, "\tU2(%d)[%d] iterations converged. iter = %d   RMS = %4.3e\n", 
		root, irrep, iter, rms);
	break;
      }
      else {
	sprintf(lbl, "New UIjAb[%d]", irrep);
	global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
	sprintf(lbl, "UIjAb[%d]", irrep);
	global_dpd_->buf4_copy(&U, PSIF_CC_MISC, lbl);
	global_dpd_->buf4_close(&U);
      }
    }

    if(!conv) {
      fprintf(outfile, "\n\tU2(%d)[%d] iterative procedure failed. RMS = %4.3e\n", root, irrep, rms);
      sprintf(lbl, "UIjAb[%d]", irrep);
      global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
      global_dpd_->buf4_scm(&U, 0.0);
      global_dpd_->buf4_close(&U);
      return 1;
      fflush(outfile);
    }
    fflush(outfile);

  }
  else if(params.ref == 0) { /** UHF **/

    /* U(IJ,AB) <-- Z(IJ,AB) * D(IJ,AB) */
    sprintf(lbl, "ZIJAB[%d]", irrep);
    global_dpd_->buf4_init(&Z, PSIF_CC_MISC, irrep, 2, 7, 2, 7, 0, lbl);
    sprintf(lbl, "UIJAB[%d]", irrep);
    global_dpd_->buf4_copy(&Z, PSIF_CC_MISC, lbl);
    global_dpd_->buf4_close(&Z);

    sprintf(lbl, "UIJAB[%d]", irrep);
    global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 2, 7, 2, 7, 0, lbl);
    sprintf(lbl, "dIJAB[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 1, 6, 1, 6, 0, lbl);
    global_dpd_->buf4_dirprd(&D, &U);
    global_dpd_->buf4_close(&D);
    global_dpd_->buf4_close(&U);

    /* U(ij,ab) <-- Z(ij,ab) * D(ij,ab) */
    sprintf(lbl, "Zijab[%d]", irrep);
    global_dpd_->buf4_init(&Z, PSIF_CC_MISC, irrep, 12, 17, 12, 17, 0, lbl);
    sprintf(lbl, "Uijab[%d]", irrep);
    global_dpd_->buf4_copy(&Z, PSIF_CC_MISC, lbl);
    global_dpd_->buf4_close(&Z);

    sprintf(lbl, "Uijab[%d]", irrep);
    global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 12, 17, 12, 17, 0, lbl);
    sprintf(lbl, "dijab[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 11, 16, 11, 16, 0, lbl);
    global_dpd_->buf4_dirprd(&D, &U);
    global_dpd_->buf4_close(&D);
    global_dpd_->buf4_close(&U);

    /* U(Ij,Ab) <-- Z(Ij,Ab) * D(Ij,Ab) */
    sprintf(lbl, "ZIjAb[%d]", irrep);
    global_dpd_->buf4_init(&Z, PSIF_CC_MISC, irrep, 22, 28, 22, 28, 0, lbl);
    sprintf(lbl, "UIjAb[%d]", irrep);
    global_dpd_->buf4_copy(&Z, PSIF_CC_MISC, lbl);
    global_dpd_->buf4_close(&Z);

    sprintf(lbl, "UIjAb[%d]", irrep);
    global_dpd_->buf4_init(&U, PSIF_CC_MISC, irrep, 22, 28, 22, 28, 0, lbl);
    sprintf(lbl, "dIjAb[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 22, 28, 22, 28, 0, lbl);
    global_dpd_->buf4_dirprd(&D, &U);
    global_dpd_->buf4_close(&D);
    global_dpd_->buf4_close(&U);
  }

  timer_off("Uijab");

  return 0;
}

}} // namespace psi::cis
