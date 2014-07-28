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

/*!
  \file
  \ingroup CHKPT
*/

#include <cstdio>
#include <cstdlib>
#include <psifiles.h>
#include <boost/shared_ptr.hpp>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.h>
#include <libchkpt/chkpt.hpp>

using namespace psi;

double **Chkpt::rd_cartrep(void)
{
        int nirrep;
        double **cartrep;
        psio_address ptr;
        char *keyword;
        keyword = build_keyword("Cart. Repr. Matrices");

        nirrep = rd_nirreps();
        ptr = PSIO_ZERO;
        cartrep = matrix<double>(nirrep,9);

        psio->read_entry(PSIF_CHKPT, keyword, (char *) cartrep[0],
                9*nirrep*sizeof(double));

        free(keyword);
        return cartrep;
}

void Chkpt::wt_cartrep(double **cartrep)
{
        int i, nirrep;
        psio_address ptr;
        char *keyword;
        keyword = build_keyword("Cart. Repr. Matrices");

        nirrep = rd_nirreps();

        ptr = PSIO_ZERO;
        for(i=0; i < nirrep; i++)
                psio->write(PSIF_CHKPT, keyword, (char *) cartrep[i],
                                        9*sizeof(double), ptr, &ptr);

        free(keyword);
}

extern "C" {
/*!
** chkpt_rd_cartrep():  Reads the point group representation in the basis of
**     cartesian unit vectors.
**
** Parameters: none
**
** Returns: double **cartrep  a vector of block matrices of doubles. Each
**     row corresponds to a particular symmetry operation, each column is
**     a 3x3 block matrix.
** \ingroup CHKPT
*/
        double **chkpt_rd_cartrep(void)
        {
                return _default_chkpt_lib_->rd_cartrep();
        }

/*!
** chkpt_wt_cartrep():  Writes the point group representation in the basis of
**     cartesian unit vectors.
**
** \param cartrep = a vector of block matrices of doubles. Each row
**                  corresponds to a particular symmetry operation, each
**                  column is a 3x3 block matrix.
**
** Returns: none
** \ingroup CHKPT
*/
        void chkpt_wt_cartrep(double **cartrep)
        {
                _default_chkpt_lib_->wt_cartrep(cartrep);
        }
}

