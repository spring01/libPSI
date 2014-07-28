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

#ifndef BLAS_H
#define BLAS_H

/**
 * fortran-ordered blas routines
 */

#ifndef FC_SYMBOL
#define FC_SYMBOL 2
#endif

#if   FC_SYMBOL==1
#define F77NAME(x) x
#elif FC_SYMBOL==2
#define F77NAME(x) x##_
#endif

typedef long int integer;
typedef double doublereal;

namespace psi{ namespace fnocc{

/**
 * fortran-ordered dgemv
 */
void F_DGEMV(char trans,integer m,integer n,doublereal alpha,doublereal*A,integer lda,
            doublereal*X,integer incx,doublereal beta,doublereal*Y,integer incy);
/**
 * fortran-ordered dgemm
 */
void F_DGEMM(char transa,char transb, integer m, integer n, integer k,
            doublereal alpha,doublereal*A,integer lda,doublereal*B,integer ldb,
            doublereal beta,doublereal*C,integer ldc);
/**
 * ddot
 */
double F_DDOT(integer n,doublereal*dx,integer incx,doublereal*dy,integer incy);

/**
 * dnrm2
 */
double F_DNRM2(integer n,doublereal*x,integer incx);

/**
 * dcopy
 */
void F_DCOPY(integer n,doublereal*dx,integer incx,doublereal*dy,integer incy);

/**
 * daxpy
 */
void F_DAXPY(integer n,doublereal da,doublereal*dx,integer incx,doublereal*dy,
             integer incy);

/**
 * name mangling for fortran-ordered dgemv
 */
extern "C" {
    void F77NAME(dgemv)(char&trans,integer&m,integer&n,doublereal&alpha,doublereal*A,integer&lda,
            doublereal*X,integer&incx,doublereal&beta,doublereal*Y,integer&incy);
};
inline void DGEMV(char&trans,integer&m,integer&n,doublereal&alpha,doublereal*A,integer&lda,
            doublereal*X,integer&incx,doublereal&beta,doublereal*Y,integer&incy){
    F77NAME(dgemv)(trans,m,n,alpha,A,lda,X,incx,beta,Y,incy);
}
/**
 * name mangling for fortran-ordered dgemm
 */
extern "C" {
    void F77NAME(dgemm)(char&transa,char&transb,integer&m,integer&n,integer&k,
         doublereal&alpha,doublereal*A,integer&lda,doublereal*B,integer&ldb,
         doublereal&beta,doublereal*C,integer&ldc);
};
inline void DGEMM(char&transa,char&transb,integer&m,integer&n,integer&k,
         doublereal&alpha,doublereal*A,integer&lda,doublereal*B,integer&ldb,
         doublereal&beta,doublereal*C,integer&ldc)
{
    F77NAME(dgemm)(transa,transb,m,n,k,alpha,A,lda,B,ldb,beta,C,ldc);
};
/**
 * name mangling dcopy
 */
extern "C" {
    void F77NAME(dcopy)(integer&n,doublereal*dx,integer&incx,doublereal*dy,
         integer&incy);
};
inline void DCOPY(integer&n,doublereal*dx,integer&incx,doublereal*dy,
            integer&incy){
    F77NAME(dcopy)(n,dx,incx,dy,incy);
}
/**
 * name mangling daxpy
 */
extern "C" {
   void F77NAME(daxpy)(integer&n,doublereal&da,doublereal*dx,integer&incx,
        doublereal*dy,integer&incy);
};
inline void DAXPY(integer&n,doublereal&da,doublereal*dx,integer&incx,
            doublereal*dy,integer&incy)
{
    F77NAME(daxpy)(n,da,dx,incx,dy,incy);
};
/**
 * name mangling dnrm2
 */
extern"C"{
    double F77NAME(dnrm2)(integer&N,doublereal*X,integer&INCX);
};
inline double DNRM2(integer&N,doublereal*X,integer&INCX){
    return F77NAME(dnrm2)(N,X,INCX);
};
/**
 * name mangling dgesv
 */
extern"C" {
    void F77NAME(dgesv)(integer &N,integer &NRHS,doublereal*A,integer &LDA,integer*IPIV,doublereal*B,integer &LDB,integer &INFO);
};
inline void DGESV(integer &N,integer &NRHS,doublereal*A,integer &LDA,integer*IPIV,doublereal*B,integer &LDB,integer &INFO){
    F77NAME(dgesv)(N,NRHS,A,LDA,IPIV,B,LDB,INFO);
};
/**
 * name mangling ddot
 */
extern "C" {
    double F77NAME(ddot)(integer&n,doublereal*dx,integer&incx,doublereal*dy,integer&incy);
};
inline double DDOT(integer&n,doublereal*dx,integer&incx,doublereal*dy,integer&incy){
    return F77NAME(ddot)(n,dx,incx,dy,incy);
}


/**
 * diagonalize a real symmetric matrix
 */
void Diagonalize(integer N,doublereal*A,doublereal*W);
/**
 * name mangling dsyev
 */
extern "C" {
    void F77NAME(dsyev)(char&JOBZ,char&UPLO,integer&N,doublereal*A,integer&LDA,doublereal*W,doublereal*WORK,integer&LWORK,integer&INFO);
};
inline void DSYEV(char&JOBZ,char&UPLO,integer&N,doublereal*A,integer&LDA,doublereal*W,doublereal*WORK,integer&LWORK,integer&INFO){
    F77NAME(dsyev)(JOBZ,UPLO,N,A,LDA,W,WORK,LWORK,INFO);
}
/**
 * diagonalize a real symmetric packed matrix
 */
void Diagonalize2(integer N,doublereal*AP,doublereal*W,doublereal*Z);
/**
 * name mangling dspev
 */
extern "C" {
    void F77NAME(dspev)(char&JOBZ,char&UPLO,integer&N,doublereal*AP,doublereal*W,doublereal*Z,integer&LDZ,doublereal*WORK,integer&INFO);
};
inline void DSPEV(char&JOBZ,char&UPLO,integer&N,doublereal*AP,doublereal*W,doublereal*Z,integer&LDZ,doublereal*WORK,integer&INFO){
    F77NAME(dspev)(JOBZ,UPLO,N,AP,W,Z,LDZ,WORK,INFO);
}

/**
 *  General SVD
 */
void SVD(integer M,integer N,doublereal*A,doublereal*U,doublereal*VT,doublereal*S);
/**
 * name mangling dgesvd
 */
extern "C" {
    void F77NAME(dgesvd)(char&JOBU,char&JOBVT,integer&M,integer&N,doublereal*A,integer&LDA,doublereal*S,doublereal*U,integer&LDU,doublereal*VT,integer&LDVT,doublereal*WORK,integer&LWORK,integer&INFO);
};
inline void DGESVD(char&JOBU,char&JOBVT,integer&M,integer&N,doublereal*A,integer&LDA,doublereal*S,doublereal*U,integer&LDU,doublereal*VT,integer&LDVT,doublereal*WORK,integer&LWORK,integer&INFO){
    F77NAME(dgesvd)(JOBU,JOBVT,M,N,A,LDA,S,U,LDU,VT,LDVT,WORK,LWORK,INFO);
}

}}

#endif
