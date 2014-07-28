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

/*! \file stre.h
    \ingroup optking
    \brief STRE class declaration
*/

#ifndef _opt_stretch_h_
#define _opt_stretch_h_

#include "simple.h"

namespace opt {

class STRE : public SIMPLE {

    bool hbond; // whether stretch is a hydrogen bond
    bool inverse_stre; // whether stretch is really 1/R

  public:

    STRE(int A_in, int B_in, bool freeze_in=false);

    ~STRE() { } // also calls ~SIMPLE()

    double value(GeomType geom) const;

    // compute and return array of first derivative (B matrix elements)
    // returned matrix is [atom][x,y,z]
    double **DqDx(GeomType geom) const;

    // compute and return array of second derivative (B' matrix elements)
    // returned matrix is order 3N cart by 3N cart
    double **Dq2Dx2(GeomType geom) const;

    void print(FILE *fp, GeomType geom, int atom_offset=0) const;
    void print_intco_dat(FILE *fp, int atom_offset=0) const;
    void print_s(FILE *fp, GeomType geom) const;
    void print_disp(FILE *fp, const double old_q, const double f_q,
      const double dq, const double new_q, int atom_offset = 0) const;
    bool operator==(const SIMPLE & s2) const;
    std::string get_definition_string(int atom_offset=0) const;

    void make_hbond(void) { hbond = true; }
    bool is_hbond(void) const { return hbond; }
    void make_inverse_stre(void) { inverse_stre = true; }
    bool is_inverse_stre(void) const { return inverse_stre; }

};

}

#endif

