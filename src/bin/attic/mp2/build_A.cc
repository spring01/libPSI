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
#include <cmath>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

namespace psi{ namespace mp2{

void rhf_build_A(void);
void uhf_build_A(void);
void rhf_sf_build_A(void);
void uhf_sf_build_A(void);

void build_A(void)
{
  if(params.gradient) {
    if(params.ref == 0) rhf_sf_build_A();
    else if(params.ref == 2) uhf_sf_build_A();
  }
  else {
    if(params.ref == 0) rhf_build_A();
    else if(params.ref == 2) uhf_build_A();
  }
}

void rhf_build_A(void)
{
  dpdbuf4 Amat;  /* MO Hessian */
  dpdbuf4 D;     /* Two-electron integral */
  dpdbuf4 C;     /* Two-electron integral */
  dpdfile2 fIJ;  /* Occ-Occ Fock matrix */
  dpdfile2 fAB;  /* Vir-Vir Fock matrix */
  int h, nirreps;
  int i, j, a, b, ai, bj;
  int I, J, A, B;
  int Asym, Bsym, Isym, Jsym;
  
  global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  global_dpd_->buf4_sort(&D, PSIF_CC_MISC, rpsq, 11, 11, "A(AI,BJ)");
  global_dpd_->buf4_close(&D);

  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(AI,BJ)");
  global_dpd_->buf4_scm(&Amat, 4.0);
  global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  global_dpd_->buf4_sort_axpy(&D, PSIF_CC_MISC, sprq, 11, 11, "A(AI,BJ)", -1.0);
  global_dpd_->buf4_close(&D);
  global_dpd_->buf4_init(&C, PSIF_CC_CINTS, 0, 10, 10, 10, 10, 0, "C <ia|jb>");
  global_dpd_->buf4_sort_axpy(&C, PSIF_CC_MISC, qpsr, 11, 11, "A(AI,BJ)", -1.0);
  global_dpd_->buf4_close(&C);
  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(AI,BJ)");
  global_dpd_->buf4_close(&Amat);

  global_dpd_->file2_init(&fIJ, PSIF_CC_OEI, 0, 0, 0, "fIJ");
  global_dpd_->file2_mat_init(&fIJ);
  global_dpd_->file2_mat_rd(&fIJ);
  global_dpd_->file2_init(&fAB, PSIF_CC_OEI, 0, 1, 1, "fAB");
  global_dpd_->file2_mat_init(&fAB);
  global_dpd_->file2_mat_rd(&fAB);

  nirreps = mo.nirreps;

  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(AI,BJ)");
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&Amat, h);
    global_dpd_->buf4_mat_irrep_rd(&Amat, h);
    for(ai=0; ai < Amat.params->rowtot[h]; ai++) {
      a = Amat.params->roworb[h][ai][0];
      i = Amat.params->roworb[h][ai][1];
      A = fAB.params->rowidx[a];
      I = fIJ.params->rowidx[i];
      Asym = fAB.params->psym[a];
      Isym = fIJ.params->psym[i];
      for(bj=0; bj < Amat.params->coltot[h]; bj++) {
        b = Amat.params->colorb[h][bj][0];
        j = Amat.params->colorb[h][bj][1];
        B = fAB.params->colidx[b];
        J = fIJ.params->colidx[j];
        Bsym = fAB.params->qsym[b];
        Jsym = fIJ.params->qsym[j];
        if((I==J)&&(Asym==Bsym)) Amat.matrix[h][ai][bj] += fAB.matrix[Asym][A][B];
        if((A==B)&&(Isym==Jsym)) Amat.matrix[h][ai][bj] -= fIJ.matrix[Isym][I][J];
      }
    }
    global_dpd_->buf4_mat_irrep_wrt(&Amat, h);
    global_dpd_->buf4_mat_irrep_close(&Amat, h);
  }
  global_dpd_->buf4_close(&Amat);

  global_dpd_->file2_mat_close(&fAB);
  global_dpd_->file2_close(&fAB);
  global_dpd_->file2_mat_close(&fIJ);
  global_dpd_->file2_close(&fIJ);
}

void uhf_build_A(void)
{

}

void rhf_sf_build_A(void)
{
  int h, nirreps, e, m, a, i, em, ai, E, M, A, I;
  int Esym, Msym, Asym, Isym;
  int *virtpi, *occpi, *occ_off, *vir_off;
  int *qt_occ, *qt_vir; /* Spatial orbital translators */
  dpdfile2 fIJ, fij, fAB, fab, fIA, fia;
  dpdbuf4 Amat, Amat2, D, C;

  nirreps = mo.nirreps;
  occpi = mo.occpi; 
  virtpi = mo.virtpi;
  occ_off = mo.occ_off; 
  vir_off = mo.vir_off;
  qt_occ = mo.qt_occ; 
  qt_vir = mo.qt_vir;

  /* Two-electron integral contributions */
  global_dpd_->buf4_init(&D, PSIF_CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  global_dpd_->buf4_sort(&D, PSIF_CC_MISC, rpsq, 11, 11, "A(EM,AI)");
  global_dpd_->buf4_close(&D);
  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(EM,AI)");
  global_dpd_->buf4_sort(&Amat, PSIF_CC_TMP0, psrq, 11, 11, "D <im|ea> (ei,am)");
  global_dpd_->buf4_scm(&Amat, 2.0);
  global_dpd_->buf4_copy(&Amat, PSIF_CC_TMP0, "A(EM,ai)");
  global_dpd_->buf4_init(&D, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "D <im|ea> (ei,am)");
  global_dpd_->buf4_axpy(&D, &Amat, -1.0);
  global_dpd_->buf4_close(&D);
  global_dpd_->buf4_init(&C, PSIF_CC_CINTS, 0, 10, 10, 10, 10, 0, "C <ia|jb>");
  global_dpd_->buf4_sort(&C, PSIF_CC_TMP0, qpsr, 11, 11, "C <ai|bj>");
  global_dpd_->buf4_close(&C);
  global_dpd_->buf4_init(&C, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "C <ai|bj>");
  global_dpd_->buf4_axpy(&C, &Amat, -1.0);
  global_dpd_->buf4_close(&C);
  global_dpd_->buf4_copy(&Amat, PSIF_CC_TMP0, "A(em,ai)");
  global_dpd_->buf4_close(&Amat);

  /* Fock matrix contributions */
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
  global_dpd_->file2_init(&fIA, PSIF_CC_OEI, 0, 0, 1, "fIA");
  global_dpd_->file2_mat_init(&fIA);
  global_dpd_->file2_mat_rd(&fIA);
  global_dpd_->file2_init(&fia, PSIF_CC_OEI, 0, 0, 1, "fia");
  global_dpd_->file2_mat_init(&fia);
  global_dpd_->file2_mat_rd(&fia);

  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(EM,AI)");
  
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&Amat, h);
    global_dpd_->buf4_mat_irrep_rd(&Amat, h);

    for(em=0; em < Amat.params->rowtot[h]; em++) {
      e = Amat.params->roworb[h][em][0];
      m = Amat.params->roworb[h][em][1];
      E = fAB.params->rowidx[e];
      M = fIJ.params->rowidx[m];
      Esym = fAB.params->psym[e];
      Msym = fIJ.params->psym[m];
      for(ai=0; ai < Amat.params->coltot[h]; ai++) {
	a = Amat.params->colorb[h][ai][0];
	i = Amat.params->colorb[h][ai][1];
	A = fAB.params->colidx[a];
	I = fIJ.params->colidx[i];
	Asym = fAB.params->qsym[a];
	Isym = fIJ.params->qsym[i];

	if((M==I) && (Esym==Asym))
	  Amat.matrix[h][em][ai] += fAB.matrix[Esym][E][A];
	if((E==A) && (Msym==Isym))
	  Amat.matrix[h][em][ai] -= fIJ.matrix[Msym][M][I];
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&Amat, h);
    global_dpd_->buf4_mat_irrep_close(&Amat, h);
  }

  global_dpd_->buf4_close(&Amat);

  global_dpd_->buf4_init(&Amat, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "A(em,ai)");
  
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&Amat, h);
    global_dpd_->buf4_mat_irrep_rd(&Amat, h);

    for(em=0; em < Amat.params->rowtot[h]; em++) {
      e = Amat.params->roworb[h][em][0];
      m = Amat.params->roworb[h][em][1];
      E = fab.params->rowidx[e];
      M = fij.params->rowidx[m];
      Esym = fab.params->psym[e];
      Msym = fij.params->psym[m];
      for(ai=0; ai < Amat.params->coltot[h]; ai++) {
	a = Amat.params->colorb[h][ai][0];
	i = Amat.params->colorb[h][ai][1];
	A = fab.params->colidx[a];
	I = fij.params->colidx[i];
	Asym = fab.params->qsym[a];
	Isym = fij.params->qsym[i];

	if((M==I) && (Esym==Asym))
	  Amat.matrix[h][em][ai] += fab.matrix[Esym][E][A];
	if((E==A) && (Msym==Isym))
	  Amat.matrix[h][em][ai] -= fij.matrix[Msym][M][I];
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&Amat, h);
    global_dpd_->buf4_mat_irrep_close(&Amat, h);
  }

  global_dpd_->buf4_close(&Amat);

  global_dpd_->buf4_init(&Amat, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "A(EM,ai)");

  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&Amat, h);
    global_dpd_->buf4_mat_irrep_rd(&Amat, h);

    for(em=0; em < Amat.params->rowtot[h]; em++) {
      e = Amat.params->roworb[h][em][0];
      m = Amat.params->roworb[h][em][1];
      Esym = Amat.params->psym[e];
      Msym = Amat.params->qsym[m];
      E = e - vir_off[Esym];
      M = m - occ_off[Msym];
      for(ai=0; ai < Amat.params->coltot[h]; ai++) {
	a = Amat.params->colorb[h][ai][0];
	i = Amat.params->colorb[h][ai][1];
	Asym = Amat.params->rsym[a];
	Isym = Amat.params->ssym[i];
	A = a - vir_off[Asym];
	I = i - occ_off[Isym];

	/* This comparison is somewhat tricky.  The algebraic
	   expression for the Fock matrix shift here is:

	   A(EM,ai) += delta(M,a) f(E,i)(beta)

	   The Kronecker Delta is actually a comparison between
	   the *spatial* orbitals associated with M, and A.
	   Hence we have to compare the spatial orbital
	   translation of the the two absolute orbital indices. */
	if((qt_occ[m] == qt_vir[a]) && (Esym==Isym))
	  Amat.matrix[h][em][ai] += fia.matrix[Isym][I][E];
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&Amat, h);
    global_dpd_->buf4_mat_irrep_close(&Amat, h);
  }
  global_dpd_->buf4_sort(&Amat, PSIF_CC_TMP0, rspq, 11, 11, "A(em,AI)");
  global_dpd_->buf4_close(&Amat);

  global_dpd_->file2_mat_close(&fIJ);
  global_dpd_->file2_close(&fIJ);
  global_dpd_->file2_mat_close(&fij);
  global_dpd_->file2_close(&fij);
  global_dpd_->file2_mat_close(&fAB);
  global_dpd_->file2_close(&fAB);
  global_dpd_->file2_mat_close(&fab);
  global_dpd_->file2_close(&fab);
  global_dpd_->file2_mat_close(&fIA);
  global_dpd_->file2_close(&fIA);
  global_dpd_->file2_mat_close(&fia);
  global_dpd_->file2_close(&fia);

  /* Now sum all three A-matrix components and divide by 2 */
  global_dpd_->buf4_init(&Amat, PSIF_CC_MISC, 0, 11, 11, 11, 11, 0, "A(EM,AI)");
  global_dpd_->buf4_init(&Amat2, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "A(em,ai)");
  global_dpd_->buf4_axpy(&Amat2, &Amat, 1.0);
  global_dpd_->buf4_close(&Amat2);
  global_dpd_->buf4_init(&Amat2, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "A(EM,ai)");
  global_dpd_->buf4_axpy(&Amat2, &Amat, 1.0);
  global_dpd_->buf4_close(&Amat2);
  global_dpd_->buf4_init(&Amat2, PSIF_CC_TMP0, 0, 11, 11, 11, 11, 0, "A(em,AI)");
  global_dpd_->buf4_axpy(&Amat2, &Amat, 1.0);
  global_dpd_->buf4_close(&Amat2);
  global_dpd_->buf4_scm(&Amat, 0.5);
  global_dpd_->buf4_close(&Amat);
}

void uhf_sf_build_A(void)
{

}

}} /* End namespace */
