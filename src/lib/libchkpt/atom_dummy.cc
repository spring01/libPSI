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

#include <cstdlib>
#include <psifiles.h>
#include <boost/shared_ptr.hpp>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.h>
#include <libchkpt/chkpt.hpp>

using namespace psi;

int* Chkpt::rd_atom_dummy(void)
{
        int num_allatoms;
        int *atom_dummy;
        char *keyword;
        keyword = build_keyword("Dummy atom flags");

        num_allatoms = rd_nallatom();
        atom_dummy = new int[num_allatoms];
        //atom_dummy = (int *) malloc(sizeof(int)*num_allatoms);

        psio->read_entry(PSIF_CHKPT, keyword, (char *) atom_dummy,
                num_allatoms*sizeof(int));

        free(keyword);
        return atom_dummy;
}

void Chkpt::wt_atom_dummy(int* atom_dummy)
{
        int num_allatoms = rd_nallatom();
        char *keyword;
        keyword = build_keyword("Dummy atom flags");

        psio->write_entry(PSIF_CHKPT, keyword, (char *) atom_dummy,
                num_allatoms*sizeof(int));

        free(keyword);
}

extern "C" {
/*!
** chkpt_rd_atom_dummy()
**
** Reads the array of flags which indicate whether the atom in full_geom
** is dummy
**
** Parameters: none
**
** Returns: atom_dummy = array of integers nallatom long.
** \ingroup CHKPT
*/
        int* chkpt_rd_atom_dummy(void)
        {
                return _default_chkpt_lib_->rd_atom_dummy();
        }


/*!
** chkpt_wt_atom_dummy()
**
** Writes the array of flags which indicate whether the atom in full_geom
** is dummy
**
** \param atom_dummy = array of integers nallatom long.
**
** Returns: none
** \ingroup CHKPT
*/
        void chkpt_wt_atom_dummy(int* atom_dummy)
        {
                _default_chkpt_lib_->wt_atom_dummy(atom_dummy);
        }
}

