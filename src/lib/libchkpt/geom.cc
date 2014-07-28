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

double **Chkpt::rd_geom(void)
{
        double **geom, **full_geom;
        int *atom_dummy;
        int natom, nallatom, atom, atomcount;

        natom = rd_natom();
        geom = matrix<double>(natom, 3);

        nallatom = rd_nallatom();
        full_geom = rd_fgeom();
        atom_dummy = rd_atom_dummy();

        atomcount = 0;
        for(atom=0;atom<nallatom;++atom) {
                if (!atom_dummy[atom]) {
                        geom[atomcount][0] = full_geom[atom][0];
                        geom[atomcount][1] = full_geom[atom][1];
                        geom[atomcount][2] = full_geom[atom][2];
                        ++atomcount;
                }
        }

        delete[] (full_geom);
        delete[] (atom_dummy);

        return geom;
}

void Chkpt::wt_geom(double **geom)
{
        double **full_geom;
        int *atom_dummy;
        int natom, nallatom, atom, atomcount;

        natom = rd_natom();

        nallatom = rd_nallatom();
        full_geom = rd_fgeom();
        atom_dummy = rd_atom_dummy();

        atomcount = 0;
        for(atom=0;atom<nallatom;++atom) {
                if (!atom_dummy[atom]) {
                        full_geom[atom][0] = geom[atomcount][0];
                        full_geom[atom][1] = geom[atomcount][1];
                        full_geom[atom][2] = geom[atomcount][2];
                        ++atomcount;
                }
        }

        wt_fgeom(full_geom);
        free(full_geom);
        free(atom_dummy);
}

extern "C" {
/* chkpt_rd_geom(): Reads in the cartesian geometry from chkpt
**
**  takes no arguments.
**
**  returns: double **geom   The cartesian geometry is returned as a matrix
**     of doubles.  The row index is the atomic index, and the column is the
**     cartesian direction index (x=0, y=1, z=2).  Therefore, geom[2][0]
**     would be the x-coordinate of the third atom.
** \ingroup CHKPT
*/
        double **chkpt_rd_geom(void)
        {
                return _default_chkpt_lib_->rd_geom();
        }

/* chkpt_wt_geom(): Writes out the cartesian geometry to chkpt
**
** arguments:
**  \param geom =  The cartesian geometry is supplied as a matrix
**     of doubles.  The row index is the atomic index, and the column is the
**     cartesian direction index (x=0, y=1, z=2).  Therefore, geom[2][0]
**     would be the x-coordinate of the third atom.
** \ingroup CHKPT
*/
        void chkpt_wt_geom(double **geom)
        {
                _default_chkpt_lib_->wt_geom(geom);
        }
}
