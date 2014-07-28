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
#include <cstring>
#include <cstdlib>
#include <psifiles.h>
#include <boost/shared_ptr.hpp>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.h>
#include <libchkpt/chkpt.hpp>

using namespace psi;

double Chkpt::rd_e_labeled(const char *label)
{
        char *s;
        double E;

        s = (char *) malloc((strlen(label)+3)*sizeof(char));
        strcpy(s,"::");
        strcat(s,label);

        psio->read_entry(PSIF_CHKPT, s, (char *) &E, sizeof(double));

        free(s);
        return E;
}

void Chkpt::wt_e_labeled(const char *label, double E)
{
        char *s;

        s = (char *) malloc((strlen(label)+3)*sizeof(char));
        strcpy(s,"::");
        strcat(s,label);

        psio->write_entry(PSIF_CHKPT, s, (char *) &E, sizeof(double));
        free(s);
}

extern "C" {
/*!
** chkpt_rd_e_labeled(): Reads in an energy with a given label
**
**  arguments:
**   \param char * label
**
**  returns: double E, the energy
**  \ingroup CHKPT
*/
        double chkpt_rd_e_labeled(const char *label)
        {
                double E;
                E = _default_chkpt_lib_->rd_e_labeled(label);
                return E;
        }

/*!
** chkpt_wt_e_labeled(): Write an energy along with a label
**
**  arguments:
**   \param char *label, the label
**   \param double E, the energy
**
**  returns: none
**  \ingroup CHKPT
*/
        void chkpt_wt_e_labeled(const char *label, double E)
        {
                _default_chkpt_lib_->wt_e_labeled(label, E);
        }
}
