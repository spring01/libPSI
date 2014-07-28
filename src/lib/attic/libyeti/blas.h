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

#if FC_SYMBOL==2
#define F_DGBMV dgbmv_
#define F_DGEMM dgemm_
#define F_DGEMV dgemv_
#define F_DGER dger_
#define F_DSBMV dsbmv_
#define F_DSPMV dspmv_
#define F_DSPR dspr_
#define F_DSPR2 dspr2_
#define F_DSYMM dsymm_
#define F_DSYMV dsymv_
#define F_DSYR dsyr_
#define F_DSYR2 dsyr2_
#define F_DSYR2K dsyr2k_
#define F_DSYRK dsyrk_
#define F_DTBMV dtbmv_
#define F_DTBSV dtbsv_
#define F_DTPMV dtpmv_
#define F_DTPSV dtpsv_
#define F_DTRMM dtrmm_
#define F_DTRMV dtrmv_
#define F_DTRSM dtrsm_
#define F_DTRSV dtrsv_
#elif FC_SYMBOL==1
#define F_DGBMV dgbmv
#define F_DGEMM dgemm
#define F_DGEMV dgemv
#define F_DGER dger
#define F_DSBMV dsbmv
#define F_DSPMV dspmv
#define F_DSPR dspr
#define F_DSPR2 dspr2
#define F_DSYMM dsymm
#define F_DSYMV dsymv
#define F_DSYR dsyr
#define F_DSYR2 dsyr2
#define F_DSYR2K dsyr2k
#define F_DSYRK dsyrk
#define F_DTBMV dtbmv
#define F_DTBSV dtbsv
#define F_DTPMV dtpmv
#define F_DTPSV dtpsv
#define F_DTRMM dtrmm
#define F_DTRMV dtrmv
#define F_DTRSM dtrsm
#define F_DTRSV dtrsv
#elif FC_SYMBOL==3
#define F_DGBMV DGBMV
#define F_DGEMM DGEMM
#define F_DGEMV DGEMV
#define F_DGER DGER
#define F_DSBMV DSBMV
#define F_DSPMV DSPMV
#define F_DSPR DSPR
#define F_DSPR2 DSPR2
#define F_DSYMM DSYMM
#define F_DSYMV DSYMV
#define F_DSYR DSYR
#define F_DSYR2 DSYR2
#define F_DSYR2K DSYR2K
#define F_DSYRK DSYRK
#define F_DTBMV DTBMV
#define F_DTBSV DTBSV
#define F_DTPMV DTPMV
#define F_DTPSV DTPSV
#define F_DTRMM DTRMM
#define F_DTRMV DTRMV
#define F_DTRSM DTRSM
#define F_DTRSV DTRSV
#elif FC_SYMBOL==4
#define F_DGBMV DGBMV_
#define F_DGEMM DGEMM_
#define F_DGEMV DGEMV_
#define F_DGER DGER_
#define F_DSBMV DSBMV_
#define F_DSPMV DSPMV_
#define F_DSPR DSPR_
#define F_DSPR2 DSPR2_
#define F_DSYMM DSYMM_
#define F_DSYMV DSYMV_
#define F_DSYR DSYR_
#define F_DSYR2 DSYR2_
#define F_DSYR2K DSYR2K_
#define F_DSYRK DSYRK_
#define F_DTBMV DTBMV_
#define F_DTBSV DTBSV_
#define F_DTPMV DTPMV_
#define F_DTPSV DTPSV_
#define F_DTRMM DTRMM_
#define F_DTRMV DTRMV_
#define F_DTRSM DTRSM_
#define F_DTRSV DTRSV_
#endif
#if FC_SYMBOL==2
#define F_DGEEV dgeev_
#define F_DGESV dgesv_
#define F_DGETRF dgetrf_
#define F_DGETRI dgetri_
#define F_DPOTRF dpotrf_
#define F_DPOTRI dpotri_
#define F_DPOTRS dpotrs_
#define F_DGESVD dgesvd_
#define F_DSYEV dsyev_
#define F_DSPSVX dspsvx_
#elif FC_SYMBOL==1
#define F_DGEEV dgeev
#define F_DGESV dgesv
#define F_DGETRF dgetrf
#define F_DGETRI dgetri
#define F_DPOTRF dpotrf
#define F_DPOTRI dpotri
#define F_DPOTRS dpotrs
#define F_DGESVD dgesvd
#define F_DSYEV dsyev
#define F_DSPSVX dspsvx
#elif FC_SYMBOL==3
#define F_DGEEV DGEEV
#define F_DGESV DGESV
#define F_DGETRF DGETRF
#define F_DGETRI DGETRI
#define F_DPOTRF DPOTRF
#define F_DPOTRI DPOTRI
#define F_DPOTRS DPOTRS
#define F_DGESVD DGESVD
#define F_DSYEV DSYEV
#define F_DSPSVX DSPSVX
#elif FC_SYMBOL==4
#define F_DGEEV DGEEV_
#define F_DGESV DGESV_
#define F_DGETRF DGETRF_
#define F_DGETRI DGETRI_
#define F_DPOTRF DPOTRF_
#define F_DPOTRI DPOTRI_
#define F_DPOTRS DPOTRS_
#define F_DGESVD DGESVD_
#define F_DSYEV DSYEV_
#define F_DSPSVX DSPSVX_
#endif
