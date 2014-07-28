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
#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#include "Local.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace cis {

void denom(int irrep, double root)
{
  int Gij, Gab;
  int ij, ab, i, j, a, b, I, J, A, B, isym, jsym, asym, bsym;
  int nirreps;
  int *occpi, *virtpi, *occ_off, *vir_off;
  int *aoccpi, *avirtpi, *aocc_off, *avir_off;
  int *boccpi, *bvirtpi, *bocc_off, *bvir_off;
  double fii, fjj, faa, fbb;
  dpdfile2 fIJ, fij, fAB, fab;
  dpdbuf4 D;
  char lbl[32];

  nirreps = moinfo.nirreps;

  if(params.ref == 0) { /** RHF **/

    occpi = moinfo.occpi;
    virtpi = moinfo.virtpi;
    occ_off = moinfo.occ_off;
    vir_off = moinfo.vir_off;

    global_dpd_->file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
    global_dpd_->file2_mat_init(&fIJ);
    global_dpd_->file2_mat_rd(&fIJ);
  
    global_dpd_->file2_init(&fij, PSIF_CC_OEI, 0, 0, 0, "fij");
    global_dpd_->file2_mat_init(&fij);
    global_dpd_->file2_mat_rd(&fij);
  
    global_dpd_->file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
    global_dpd_->file2_mat_init(&fAB);
    global_dpd_->file2_mat_rd(&fAB);
  
    global_dpd_->file2_init(&fab, PSIF_CC_OEI, 0, 1, 1, "fab");
    global_dpd_->file2_mat_init(&fab);
    global_dpd_->file2_mat_rd(&fab);

    sprintf(lbl, "dIjAb[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 0, 5, 0, 5, 0, lbl);
    for(Gij=0; Gij < nirreps; Gij++) {

      global_dpd_->buf4_mat_irrep_init(&D, Gij);

      for(ij=0; ij < D.params->rowtot[Gij]; ij++) {
	i = D.params->roworb[Gij][ij][0];
	j = D.params->roworb[Gij][ij][1];
	isym = D.params->psym[i];
	jsym = D.params->qsym[j];

	I = i - occ_off[isym];
	J = j - occ_off[jsym];

	fii = fIJ.matrix[isym][I][I];
	fjj = fij.matrix[jsym][J][J];
	  
	for(ab=0; ab < D.params->coltot[Gij^irrep]; ab++) {
	  a = D.params->colorb[Gij^irrep][ab][0];
	  b = D.params->colorb[Gij^irrep][ab][1];
	  asym = D.params->rsym[a];
	  bsym = D.params->ssym[b];

	  A = a - vir_off[asym];
	  B = b - vir_off[bsym];

	  faa = fAB.matrix[asym][A][A];
	  fbb = fab.matrix[bsym][B][B];

	  D.matrix[Gij][ij][ab] = 1.0/(fii + fjj - faa - fbb + root);
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&D, Gij);
    global_dpd_->buf4_mat_irrep_close(&D, Gij);

  }

  global_dpd_->buf4_close(&D);

}
  else if(params.ref == 2) { /** UHF **/

    aoccpi = moinfo.aoccpi;
    boccpi = moinfo.boccpi;
    avirtpi = moinfo.avirtpi;
    bvirtpi = moinfo.bvirtpi;
    aocc_off = moinfo.aocc_off;
    bocc_off = moinfo.bocc_off;
    avir_off = moinfo.avir_off;
    bvir_off = moinfo.bvir_off;

    global_dpd_->file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
    global_dpd_->file2_mat_init(&fIJ);
    global_dpd_->file2_mat_rd(&fIJ);
  
    global_dpd_->file2_init(&fij, PSIF_CC_OEI, 0, 2, 2, "fij");
    global_dpd_->file2_mat_init(&fij);
    global_dpd_->file2_mat_rd(&fij);
  
    global_dpd_->file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
    global_dpd_->file2_mat_init(&fAB);
    global_dpd_->file2_mat_rd(&fAB);
  
    global_dpd_->file2_init(&fab, PSIF_CC_OEI, 0, 3, 3, "fab");
    global_dpd_->file2_mat_init(&fab);
    global_dpd_->file2_mat_rd(&fab);

    sprintf(lbl, "dIJAB[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 1, 6, 1, 6, 0, lbl);
    for(Gij=0; Gij < nirreps; Gij++) {
      global_dpd_->buf4_mat_irrep_init(&D, Gij);
      for(ij=0; ij < D.params->rowtot[Gij]; ij++) {
	i = D.params->roworb[Gij][ij][0];
	j = D.params->roworb[Gij][ij][1];
	isym = D.params->psym[i];
	jsym = D.params->qsym[j];
	I = i - aocc_off[isym];
	J = j - aocc_off[jsym];
	fii = fIJ.matrix[isym][I][I];
	fjj = fIJ.matrix[jsym][J][J];

	for(ab=0; ab < D.params->coltot[Gij^irrep]; ab++) {
	  a = D.params->colorb[Gij^irrep][ab][0];
	  b = D.params->colorb[Gij^irrep][ab][1];
	  asym = D.params->rsym[a];
	  bsym = D.params->ssym[b];
	  A = a - avir_off[asym];
	  B = b - avir_off[bsym];
	  faa = fAB.matrix[asym][A][A];
	  fbb = fAB.matrix[bsym][B][B];

	  D.matrix[Gij][ij][ab] = 1.0/(fii + fjj - faa - fbb + root);
	}
      }
      global_dpd_->buf4_mat_irrep_wrt(&D, Gij);
      global_dpd_->buf4_mat_irrep_close(&D, Gij);
    }
    global_dpd_->buf4_close(&D);

    sprintf(lbl, "dijab[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 11, 16, 11, 16, 0, lbl);
    for(Gij=0; Gij < nirreps; Gij++) {
      global_dpd_->buf4_mat_irrep_init(&D, Gij);
      for(ij=0; ij < D.params->rowtot[Gij]; ij++) {
	i = D.params->roworb[Gij][ij][0];
	j = D.params->roworb[Gij][ij][1];
	isym = D.params->psym[i];
	jsym = D.params->qsym[j];
	I = i - bocc_off[isym];
	J = j - bocc_off[jsym];
	fii = fij.matrix[isym][I][I];
	fjj = fij.matrix[jsym][J][J];

	for(ab=0; ab < D.params->coltot[Gij^irrep]; ab++) {
	  a = D.params->colorb[Gij^irrep][ab][0];
	  b = D.params->colorb[Gij^irrep][ab][1];
	  asym = D.params->rsym[a];
	  bsym = D.params->ssym[b];
	  A = a - bvir_off[asym];
	  B = b - bvir_off[bsym];
	  faa = fab.matrix[asym][A][A];
	  fbb = fab.matrix[bsym][B][B];

	  D.matrix[Gij][ij][ab] = 1.0/(fii + fjj - faa - fbb + root);
	}
      }
      global_dpd_->buf4_mat_irrep_wrt(&D, Gij);
      global_dpd_->buf4_mat_irrep_close(&D, Gij);
    }
    global_dpd_->buf4_close(&D);

    sprintf(lbl, "dIjAb[%d]", irrep);
    global_dpd_->buf4_init(&D, PSIF_CC_MISC, irrep, 22, 28, 22, 28, 0, lbl);
    for(Gij=0; Gij < nirreps; Gij++) {
      global_dpd_->buf4_mat_irrep_init(&D, Gij);
      for(ij=0; ij < D.params->rowtot[Gij]; ij++) {
	i = D.params->roworb[Gij][ij][0];
	j = D.params->roworb[Gij][ij][1];
	isym = D.params->psym[i];
	jsym = D.params->qsym[j];
	I = i - aocc_off[isym];
	J = j - bocc_off[jsym];
	fii = fIJ.matrix[isym][I][I];
	fjj = fij.matrix[jsym][J][J];

	for(ab=0; ab < D.params->coltot[Gij^irrep]; ab++) {
	  a = D.params->colorb[Gij^irrep][ab][0];
	  b = D.params->colorb[Gij^irrep][ab][1];
	  asym = D.params->rsym[a];
	  bsym = D.params->ssym[b];
	  A = a - avir_off[asym];
	  B = b - bvir_off[bsym];
	  faa = fAB.matrix[asym][A][A];
	  fbb = fab.matrix[bsym][B][B];

	  D.matrix[Gij][ij][ab] = 1.0/(fii + fjj - faa - fbb + root);
	}
      }
      global_dpd_->buf4_mat_irrep_wrt(&D, Gij);
      global_dpd_->buf4_mat_irrep_close(&D, Gij);
    }
    global_dpd_->buf4_close(&D);

    global_dpd_->file2_mat_close(&fIJ);
    global_dpd_->file2_mat_close(&fij);
    global_dpd_->file2_mat_close(&fAB);
    global_dpd_->file2_mat_close(&fab);
    global_dpd_->file2_close(&fIJ);
    global_dpd_->file2_close(&fij);
    global_dpd_->file2_close(&fAB);
    global_dpd_->file2_close(&fab);
  }
}

}} // namespace psi::cis
