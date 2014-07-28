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

#include "sapt0.h"
#include "sapt2.h"

namespace psi { namespace sapt {

void SAPT0::elst10()
{
  e_elst10_ = 4.0*C_DDOT(ndf_+3,diagAA_,1,diagBB_,1);
  
  if (print_) {
    fprintf(outfile,"    Elst10,r            = %18.12lf H\n",e_elst10_);
    fflush(outfile);
  }
}

void SAPT2::elst10()
{
  e_elst10_ = 4.0*C_DDOT(ndf_+3,diagAA_,1,diagBB_,1);
  
  if (print_) {
    fprintf(outfile,"    Elst10,r            = %18.12lf H\n",e_elst10_);
    fflush(outfile);
  }
}

}}
