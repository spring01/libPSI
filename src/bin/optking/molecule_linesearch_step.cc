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

/*! \file molecule_linesearch_step.cc
    \ingroup optking
    \brief Generates geometries for a line-search in internal coordinates.
    Currently, a static line search merely displaces against the gradient in internal
    coordinates generating N geometries.  The other two keywords
    control the min and the max of the largest internal coordinate displacement.

    Relevant parameters:
    Opt_params.step_type   = options.get_int("LINESEARCH_STATIC");
    Opt_params.linesearch_static_N   = options.get_int("LINESEARCH_STATIC_N");
    Opt_params.linesearch_static_min = options.get_int("LINESEARCH_STATIC_MIN");
    Opt_params.linesearch_static_max = options.get_int("LINESEARCH_STATIC_MAX");
*/


#include "molecule.h"

#include <iostream>
#include <sstream>

#include "linear_algebra.h"
#include "print.h"
#include "atom_data.h"
#include "physconst.h"

#define EXTERN
#include "globals.h"

#if defined(OPTKING_PACKAGE_PSI)
 #include <cmath>
#elif defined (OPTKING_PACKAGE_QCHEM)
 #include "qcmath.h"
#endif

namespace opt {

void MOLECULE::linesearch_step(void) {
  int dim = g_nintco();
  double *fq = p_Opt_data->g_forces_pointer();
  double *dq = p_Opt_data->g_dq_pointer();

  for (int i=0; i<dim; ++i)
    dq[i] = fq[i];

  // Zero steps for frozen fragment
  for (int f=0; f<fragments.size(); ++f) {
    if (fragments[f]->is_frozen() || Opt_params.freeze_intrafragment) {
      fprintf(outfile,"\tZero'ing out displacements for frozen fragment %d\n", f+1);
      for (int i=0; i<fragments[f]->g_nintco(); ++i)
        dq[ g_intco_offset(f) + i ] = 0.0;
    }
  }

  // Find largest change in an internal coordinate
  double max_dq_orig = 0;
  for (int i=0; i<dim; ++i) {
    if (abs(dq[i]) > max_dq_orig)
      max_dq_orig = abs(dq[i]);
  }

  double *dq_orig = init_array(dim);
  for (int i=0; i<dim; ++i)
    dq_orig[i] = dq[i];

  double *geom_array_orig = g_geom_array();

  // Number of geometries to generate
  int Ndisp = Opt_params.linesearch_static_N;
  double disp_min = Opt_params.linesearch_static_min;
  double disp_max = Opt_params.linesearch_static_max;

  FILE *fout = fopen("linesearch_geoms.py", "w");

  double step_size =(disp_max - disp_min)/(Ndisp-1);
  
  for (int igeom=0; igeom<Ndisp; ++igeom) {

    double max_dq = disp_min + step_size*igeom;

    double scale = max_dq/max_dq_orig;
    for (int i=0; i<dim; ++i)
      dq[i] = scale * dq_orig[i];

    set_geom_array(geom_array_orig);

    // do displacements for each fragment separately
    for (int f=0; f<fragments.size(); ++f) {
      if (fragments[f]->is_frozen() || Opt_params.freeze_intrafragment) {
        fprintf(outfile,"\tDisplacements for frozen fragment %d skipped.\n", f+1);
        continue;
      }
      fragments[f]->displace(&(dq[g_intco_offset(f)]), &(fq[g_intco_offset(f)]), g_atom_offset(f));
    }

    // do displacements for interfragment coordinates
    for (int I=0; I<interfragments.size(); ++I) {
      if (interfragments[I]->is_frozen() || Opt_params.freeze_interfragment) {
        fprintf(outfile,"\tDisplacements for frozen interfragment %d skipped.\n", I+1);
        continue;
      }
      interfragments[I]->orient_fragment( &(dq[g_interfragment_intco_offset(I)]),
                                          &(fq[g_interfragment_intco_offset(I)]) );
    }

#if defined(OPTKING_PACKAGE_QCHEM)
    // fix rotation matrix for rotations in QCHEM EFP code
    for (int I=0; I<efp_fragments.size(); ++I)
      efp_fragments[I]->displace( I, &(dq[g_efp_fragment_intco_offset(I)]) );
#endif

    symmetrize_geom(); // now symmetrize the geometry for next step

    // Now write out file.
    fprintf(outfile,"\t Line search structure #%d : maximum intco change %8.4f\n", igeom+1, max_dq);
    print_geom();

    std::stringstream geom_string;
    geom_string << "geom_" << igeom+1;

    double *coord = g_geom_array();

    fprintf(fout,"%s = [ \n", geom_string.str().c_str());
    for (int i=0; i<g_natom(); ++i)  {
      for (int xyz=0; xyz<3; ++xyz)
        fprintf(fout, "%15.10lf,", coord[3*i+xyz]);
      fprintf(fout,"\n");
    }
    fprintf(fout, " ] \n");

  }
  fclose(fout);
  free_array(dq_orig);
}

}

