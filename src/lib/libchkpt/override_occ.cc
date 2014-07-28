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

#include <stdio.h>
#include <cstdlib>
#include <psifiles.h>
#include <boost/shared_ptr.hpp>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.h>
#include <libchkpt/chkpt.hpp>

using namespace psi;

int Chkpt::rd_override_occ(void)
{
        int override=0, keylen;
        char *keyword;
/*
        keyword = chkpt_build_keyword("Override Occupations");
        if ( chkpt_exist(keyword) )
                psio_read_entry(PSIF_CHKPT, keyword, (char *) &override, sizeof(int));
        free(keyword);
        return override;
*/

/* new way - keep override in root area always */
        keylen = 22;
        keyword = (char *) malloc((keylen+1)*sizeof(char));
        sprintf(keyword, "::%s", "Override Occupations");
        keyword[keylen] = '\0';

        if ( exist(keyword) )
                psio->read_entry(PSIF_CHKPT, keyword, (char *) &override, sizeof(int));

        free(keyword);
        return override;
}

void Chkpt::wt_override_occ(int override)
{
        char *keyword;
        int keylen;
/*
        keyword = chkpt_build_keyword("Override Occupations");
        psio_write_entry(PSIF_CHKPT, keyword, (char *) &override, sizeof(int));
*/

/* new way - keep override in root area always */
        keylen = 22;
        keyword = (char *) malloc((keylen+1)*sizeof(char));
        sprintf(keyword, "::%s", "Override Occupations");
        keyword[keylen] = '\0';

        psio->write_entry(PSIF_CHKPT, keyword, (char *) &override, sizeof(int));

        free(keyword);
        return;
}

extern "C" {
/*!
** chkpt_rd_override_occ(): Reads flag which tells cscf to ignore docc/socc
** vectors and use occupations in chkpt file instead
**
** takes no arguments.
**
** returns: 1 if chkpt occupations should be forced; 0 otherwise
**
** \ingroup CHKPT
*/
        int chkpt_rd_override_occ(void)
        {
                return _default_chkpt_lib_->rd_override_occ();
        }

/*!
** chkpt_wt_override_occ(): Writes flag which tells cscf to ignore docc/socc
** vectors and use occupations in chkpt file instead
**
** arguments: (int) 1 to set override; 0 otherwise
**
** returns: none
**
** \ingroup CHKPT
*/
        void chkpt_wt_override_occ(int override)
        {
                _default_chkpt_lib_->wt_override_occ(override);
        }
}
