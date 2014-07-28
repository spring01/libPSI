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

double *Chkpt::rd_evals(void)
{
        double *energies;
        char *keyword;
        keyword = build_keyword("MO energies");

        if (psio->tocscan(PSIF_CHKPT, keyword) != NULL) {
          energies = array<double>(rd_nmo());

          psio->read_entry(PSIF_CHKPT, keyword, (char *) energies,
                           rd_nmo()*sizeof(double));
          free(keyword);
          return energies;
        }
        else
          return rd_alpha_evals();

}

double *Chkpt::rd_alpha_evals(void)
{
        double *energies = 0;
        char *keyword;
        keyword = build_keyword("Alpha MO energies");

    if (psio->tocscan(PSIF_CHKPT, keyword) != NULL) {
      energies = array<double>(rd_nmo());

      psio->read_entry(PSIF_CHKPT, keyword, (char *) energies,
                       rd_nmo()*sizeof(double));
    }

        free(keyword);
        return energies;
}

double *Chkpt::rd_beta_evals(void)
{
        double *energies = 0;
        char *keyword;
        keyword = build_keyword("Beta MO energies");

    if (psio->tocscan(PSIF_CHKPT, keyword) != NULL) {
      energies = array<double>(rd_nmo());

      psio->read_entry(PSIF_CHKPT, keyword, (char *) energies,
                       rd_nmo()*sizeof(double));
    }

        free(keyword);
        return energies;
}

void Chkpt::wt_evals(double *energies)
{
        char *keyword;
        keyword = build_keyword("MO energies");

        psio->write_entry(PSIF_CHKPT, keyword, (char *) energies,
                rd_nmo()*sizeof(double));

        free(keyword);
}

void Chkpt::wt_alpha_evals(double *energies)
{
        char *keyword;
        keyword = build_keyword("Alpha MO energies");

        psio->write_entry(PSIF_CHKPT, keyword, (char *) energies,
                rd_nmo()*sizeof(double));

        free(keyword);
}

void Chkpt::wt_beta_evals(double *energies)
{
        char *keyword;
        keyword = build_keyword("Beta MO energies");

        psio->write_entry(PSIF_CHKPT, keyword, (char *) energies,
                rd_nmo()*sizeof(double));

        free(keyword);
}

extern "C" {
/*!
** chkpt_rd_evals():  Reads in the SCF orbital energies for RHF/ROHF.
**
**  takes no arguments.
**
**  returns: double *evals   an array of _all_ of the SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
** \ingroup CHKPT
*/
        double *chkpt_rd_evals(void)
        {
                double *energies;
                energies = _default_chkpt_lib_->rd_evals();
                return energies;
        }

/*!
** chkpt_rd_alpha_evals():  Reads in the SCF alpha orbital energies for UHF.
**
**  takes no arguments.
**
**  returns: double *evals   an array of _all_ of the alpha SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
** \ingroup CHKPT
*/
        double *chkpt_rd_alpha_evals(void)
        {
                double *energies;
                energies = _default_chkpt_lib_->rd_alpha_evals();
                return energies;
        }

/*!
** chkpt_rd_beta_evals():  Reads in the SCF beta orbital energies for UHF.
**
**  takes no arguments.
**
**  returns: double *evals   an array of _all_ of the beta SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
** \ingroup CHKPT
*/
        double *chkpt_rd_beta_evals(void)
        {
                double *energies;
                energies = _default_chkpt_lib_->rd_beta_evals();
                return energies;
        }

/*!
** chkpt_wt_evals():  Writes the SCF orbital energies for UHF.
**
** arguments:
**  \param evals = an array of _all_ of the SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
**
** returns: none
** \ingroup CHKPT
*/
        void chkpt_wt_evals(double *energies)
        {
                _default_chkpt_lib_->wt_evals(energies);
        }

/*!
** chkpt_wt_alpha_evals():  Writes the SCF alpha orbital energies for UHF.
**
** arguments:
**  \param evals = an array of _all_ of the alpha SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
**
** returns: none
** \ingroup CHKPT
*/
        void chkpt_wt_alpha_evals(double *energies)
        {
                _default_chkpt_lib_->wt_alpha_evals(energies);
        }

/*!
** chkpt_wt_beta_evals():  Writes the SCF beta orbital energies for UHF.
**
** arguments:
**  \param evals =  an array of _all_ of the beta SCF eigenvalues,
**      ordered by irrep, and by increasing energy within each irrep.
**      (i.e. for sto water, the four a1 eigenvalues all come first, and
**      those four are ordered from lowest energy to highest energy,
**      followed by the single b1 eigenvalue, etc.)
**
** returns: none
** \ingroup CHKPT
*/
        void chkpt_wt_beta_evals(double *energies)
        {
                _default_chkpt_lib_->wt_beta_evals(energies);
        }
}
