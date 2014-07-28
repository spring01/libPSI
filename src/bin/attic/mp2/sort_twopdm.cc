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
#include <cstdio>
#include <libdpd/dpd.h>
#include <libiwl/iwl.h>
#define EXTERN
#include "globals.h"

namespace psi{ namespace mp2{

void rhf_sort_twopdm(void);
void uhf_sort_twopdm(void);
void check_energy(int);

void sort_twopdm(void)
{
  if(params.ref == 0) rhf_sort_twopdm();
  else if(params.ref == 2) uhf_sort_twopdm();
}

void rhf_sort_twopdm(void)
{
  int h, nirreps;
  int i, j, k, l, m, a, b;
  int I, J, K, L, M, A, B;
  int IM, JM, MI, MJ, MK, ML, MA, MB, AM;
  int Gi, Gj, Gk, Gl, Gm, Ga, Gb;
  int *occpi, *virpi;
  int *occ_off, *vir_off;
  int *occ_sym, *vir_sym;
  int ndocc;
  int *qt_occ;
  int *qt_vir;
  dpdfile2 D;
  dpdbuf4 G, G1, G2;
  struct iwlbuf OutBuf;

  nirreps = mo.nirreps;
  ndocc = mo.ndocc;

  occpi = mo.occpi; 
  virpi = mo.virpi;
  occ_off = mo.occ_off; 
  vir_off = mo.vir_off;
  occ_sym = mo.occ_sym; 
  vir_sym = mo.vir_sym;
  qt_occ = mo.qt_occ;
  qt_vir = mo.qt_vir;

  global_dpd_->file2_init(&D, PSIF_CC_OEI, 0, 0, 0, "DIJ");
  global_dpd_->file2_mat_init(&D);
  global_dpd_->file2_mat_rd(&D);

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 0, 0, 0, 0, 0, "GIjKl");
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&G, h);
    for(Gm=0; Gm < nirreps; Gm++) {
      Gi = Gj = h^Gm;
      for(i=0; i < occpi[Gi]; i++) {
        I = occ_off[Gi] + i;
        for(j=0; j < occpi[Gj]; j++) {
          J = occ_off[Gj] + j;
          for(m=0; m < occpi[Gm]; m++) {
            M = occ_off[Gm] + m;
            IM = G.params->rowidx[I][M];
            MI = G.params->rowidx[M][I];
            JM = G.params->colidx[J][M];
            MJ = G.params->colidx[M][J];

            G.matrix[h][IM][JM] += D.matrix[Gi][i][j];
            G.matrix[h][MI][MJ] += D.matrix[Gi][i][j];
          }
        }
      }
    }
    global_dpd_->buf4_mat_irrep_wrt(&G, h);
    global_dpd_->buf4_mat_irrep_close(&G, h);
  }
  global_dpd_->buf4_close(&G);
  global_dpd_->file2_mat_close(&D);
  global_dpd_->file2_close(&D);
  
  global_dpd_->file2_init(&D, PSIF_CC_OEI, 0, 1, 0, "DAI");
  global_dpd_->file2_mat_init(&D);
  global_dpd_->file2_mat_rd(&D); 
  
  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 11, 0, 11, 0, 0, "GAiJk");
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&G, h);
    for(Gm=0; Gm < nirreps; Gm++) {
      Gi = Ga = h^Gm;
      for(a=0; a < virpi[Ga]; a++) {
        A = vir_off[Ga] + a;
        for(i=0; i < occpi[Gi]; i++) {
          I = occ_off[Gi] + i;
          for(m=0; m < occpi[Gm]; m++) {
            M = occ_off[Gm] + m;
            AM = G.params->rowidx[A][M];
            IM = G.params->colidx[I][M];

            G.matrix[h][AM][IM] += D.matrix[Gi][a][i];
          }
        }
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&G, h);
    global_dpd_->buf4_mat_irrep_close(&G, h);
  }
  global_dpd_->buf4_close(&G);
  global_dpd_->file2_mat_close(&D);
  global_dpd_->file2_close(&D);

  global_dpd_->file2_init(&D, PSIF_CC_OEI, 0, 1, 1, "DAB");
  global_dpd_->file2_mat_init(&D);
  global_dpd_->file2_mat_rd(&D);

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 10, 10, 10, 10, 0, "GIbJa");
  for(h=0; h < nirreps; h++) {
    global_dpd_->buf4_mat_irrep_init(&G, h);
    for(Gm=0; Gm < nirreps; Gm++) {
      Ga = Gb = h^Gm;
      for(b=0; b < virpi[Gb]; b++) {
        B = vir_off[Gb] + b;
        for(a=0; a < virpi[Ga]; a++) {
          A = vir_off[Ga] + a;
          for(m=0; m < occpi[Gm]; m++) {
            M = occ_off[Gm] + m;
            MB = G.params->rowidx[M][B];
            MA = G.params->colidx[M][A];

            G.matrix[h][MB][MA] += D.matrix[Ga][a][b];
          }
        }
      }
    }

    global_dpd_->buf4_mat_irrep_wrt(&G, h);
    global_dpd_->buf4_mat_irrep_close(&G, h);
  }
  global_dpd_->buf4_close(&G);
  global_dpd_->file2_mat_close(&D);
  global_dpd_->file2_close(&D);

  check_energy(2);

  global_dpd_->buf4_init(&G1, PSIF_CC_GAMMA, 0, 0, 0, 0, 0, 0, "GIjKl");
  global_dpd_->buf4_sort(&G1, PSIF_CC_GAMMA, pqsr, 0, 0, "GIjlK");
  global_dpd_->buf4_scm(&G1, 2.0);
  global_dpd_->buf4_init(&G2, PSIF_CC_GAMMA, 0, 0, 0, 0, 0, 0, "GIjlK");
  global_dpd_->buf4_axpy(&G2, &G1, -1.0);
  global_dpd_->buf4_close(&G2);
  global_dpd_->buf4_close(&G1);

  global_dpd_->buf4_init(&G1, PSIF_CC_GAMMA, 0, 11, 0, 11, 0, 0, "GAiJk");
  global_dpd_->buf4_sort(&G1, PSIF_CC_GAMMA, pqsr, 11, 0, "GAikJ");
  global_dpd_->buf4_scm(&G1, 2.0);
  global_dpd_->buf4_init(&G2, PSIF_CC_GAMMA, 0, 11, 0, 11, 0, 0, "GAikJ");
  global_dpd_->buf4_axpy(&G2, &G1, -1.0);
  global_dpd_->buf4_close(&G2);
  global_dpd_->buf4_close(&G1);

  global_dpd_->buf4_init(&G1, PSIF_CC_GAMMA, 0, 0, 5, 0, 5, 0, "GIjAb");
  global_dpd_->buf4_sort(&G1, PSIF_CC_GAMMA, pqsr, 0, 5, "GIjbA");
  global_dpd_->buf4_scm(&G1, 2.0);
  global_dpd_->buf4_init(&G2, PSIF_CC_GAMMA, 0, 0, 5, 0, 5, 0, "GIjbA");
  global_dpd_->buf4_axpy(&G2, &G1, -1.0);
  global_dpd_->buf4_close(&G2);
  global_dpd_->buf4_init(&G2, PSIF_CC_GAMMA, 0, 10, 10, 10, 10, 0, "GIbJa");
  global_dpd_->buf4_sort(&G2, PSIF_CC_GAMMA, prsq, 0, 5, "GIbJa (IJ,ab)");
  global_dpd_->buf4_close(&G2);
  global_dpd_->buf4_init(&G2, PSIF_CC_GAMMA, 0, 0, 5, 0, 5, 0, "GIbJa (IJ,ab)");
  global_dpd_->buf4_axpy(&G2, &G1, -0.5);
  global_dpd_->buf4_close(&G2);
  global_dpd_->buf4_close(&G1);

  global_dpd_->buf4_init(&G1, PSIF_CC_GAMMA, 0, 10, 10, 10, 10, 0, "GIbJa");
  global_dpd_->buf4_scm(&G1, 2.0);
  global_dpd_->buf4_close(&G1);

  check_energy(3);

  iwl_buf_init(&OutBuf, PSIF_MO_TPDM, 1e-14, 0, 0);

  for(i=0; i < ndocc; i++) {
    iwl_buf_wrt_val(&OutBuf, i, i, i, i, 1.0, 0, outfile, 0);
    for(j=0; j < i; j++) {
      iwl_buf_wrt_val(&OutBuf, i, i, j, j, 2.0, 0, outfile, 0);
      iwl_buf_wrt_val(&OutBuf, i, j, j, i,-1.0, 0, outfile, 0);
    }
  }

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 0, 0, 0, 0, 0, "GIjKl");
  global_dpd_->buf4_sort(&G, PSIF_CC_TMP0, prqs, 0, 0, "G(IK,JL)");
  global_dpd_->buf4_close(&G);
  global_dpd_->buf4_init(&G, PSIF_CC_TMP0, 0, 0, 0, 0, 0, 0, "G(IK,JL)");
  global_dpd_->buf4_dump(&G, &OutBuf, qt_occ, qt_occ, qt_occ, qt_occ, 1, 0);
  global_dpd_->buf4_close(&G);

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 11, 0, 11, 0, 0, "GAiJk");
  global_dpd_->buf4_sort(&G, PSIF_CC_TMP0, qsrp, 0, 10, "G(IK,JA)");
  global_dpd_->buf4_close(&G);
  global_dpd_->buf4_init(&G, PSIF_CC_TMP0, 0, 0, 10, 0, 10, 0, "G(IK,JA)");
  global_dpd_->buf4_dump(&G, &OutBuf, qt_occ, qt_occ, qt_occ, qt_vir, 0, 0);
  global_dpd_->buf4_close(&G);

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 0, 5, 0, 5, 0, "GIjAb");
  global_dpd_->buf4_sort(&G, PSIF_CC_TMP0, prqs, 10, 10, "G(IA,JB)");
  global_dpd_->buf4_close(&G);
  global_dpd_->buf4_init(&G, PSIF_CC_TMP0, 0, 10, 10, 10, 10, 0, "G(IA,JB)");
  global_dpd_->buf4_symm(&G);
  global_dpd_->buf4_dump(&G, &OutBuf, qt_occ, qt_vir, qt_occ, qt_vir, 1, 0);
  global_dpd_->buf4_close(&G);

  global_dpd_->buf4_init(&G, PSIF_CC_GAMMA, 0, 10, 10, 10, 10, 0, "GIbJa");
  global_dpd_->buf4_sort(&G, PSIF_CC_TMP0, prqs, 0, 5, "G(IJ,AB)");
  global_dpd_->buf4_close(&G);
  global_dpd_->buf4_init(&G, PSIF_CC_TMP0, 0, 0, 5, 0, 5, 0, "G(IJ,AB)");
  global_dpd_->buf4_scm(&G, 0.5);
  global_dpd_->buf4_dump(&G, &OutBuf, qt_occ, qt_occ, qt_vir, qt_vir, 0, 0);
  global_dpd_->buf4_close(&G);

  iwl_buf_flush(&OutBuf, 1);
  iwl_buf_close(&OutBuf, 1);
}

void uhf_sort_twopdm(void)
{
}

}} /* End namespaces */
