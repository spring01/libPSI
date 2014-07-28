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

/*! \file set_params.cc
    \ingroup optking
    \brief set optimization parameters
*/

#define EXTERN
#include "globals.h"

#if defined(OPTKING_PACKAGE_PSI)
 #include <psi4-dec.h>
#elif defined(OPTKING_PACKAGE_QCHEM)
 #include <qchem.h>
#endif

#include <sstream>

namespace opt {

void print_params(void);

#if defined(OPTKING_PACKAGE_PSI)
void set_params(psi::Options & options)
#else
void set_params(void)
#endif
{

  std::string s;

#if defined(OPTKING_PACKAGE_PSI)

// optimization type
    s = options.get_str("OPT_TYPE");
    if(s == "MIN")  Opt_params.opt_type = OPT_PARAMS::MIN;
    else if (s == "TS")  Opt_params.opt_type = OPT_PARAMS::TS;
    else if (s == "IRC")  Opt_params.opt_type = OPT_PARAMS::IRC;

    if (options["STEP_TYPE"].has_changed()) {
      s = options.get_str("STEP_TYPE");
      if (s == "RFO") {
        if (Opt_params.opt_type == OPT_PARAMS::MIN)
          Opt_params.step_type = OPT_PARAMS::RFO;
        else if (Opt_params.opt_type == OPT_PARAMS::TS)
          Opt_params.step_type = OPT_PARAMS::P_RFO;
      }
      else if (s == "NR") Opt_params.step_type = OPT_PARAMS::NR;
      else if (s == "SD") Opt_params.step_type = OPT_PARAMS::SD;
      else if (s == "LINESEARCH_STATIC") Opt_params.step_type = OPT_PARAMS::LINESEARCH_STATIC;
   }
   else { // Set defaults for step type.
     if (Opt_params.opt_type == OPT_PARAMS::MIN)
       Opt_params.step_type = OPT_PARAMS::RFO;
     else if (Opt_params.opt_type == OPT_PARAMS::TS)
       Opt_params.step_type = OPT_PARAMS::P_RFO;
     // else if (Opt_params.opt_type == OPT_PARAMS::IRC) options?
   }

// Maximum step size in bohr or radian along an internal coordinate {double}
//  Opt_params.intrafragment_step_limit = 0.4;
    Opt_params.intrafragment_step_limit = options.get_double("INTRAFRAG_STEP_LIMIT");
    Opt_params.intrafragment_step_limit_min = options.get_double("INTRAFRAG_STEP_LIMIT_MIN");
    Opt_params.intrafragment_step_limit_max = options.get_double("INTRAFRAG_STEP_LIMIT_MAX");

    Opt_params.interfragment_step_limit = options.get_double("INTERFRAG_STEP_LIMIT");

// Whether to 'follow' the initial RFO vector after the first step {true, false}
    Opt_params.rfo_follow_root = options.get_bool("RFO_FOLLOW_ROOT");
// Which RFO root to follow; internally 0 (externally 1) indicates minimum; {integer}
//  Opt_params.rfo_root = 0;
    Opt_params.rfo_root = options.get_int("RFO_ROOT");

// When determining connectivity, a bond is assigned if interatomic distance
// is less than (this number) * sum of covalent radii {double}
//  Opt_params.scale_connectivity = 1.3;
    Opt_params.scale_connectivity = options.get_double("COVALENT_CONNECT");

// When determining connectivity BETWEEN FRAGMENTS when fragment mode is set to
// SIMPLE, distance coordinates are created if atoms on different fragments
// are at a distance less than (this number) * sum of covalent radii {double}
// The criterion is gradually increased until all fragments are connected.
//  Opt_params.interfragment_scale_connectivity = 1.8;
    Opt_params.interfragment_scale_connectivity = options.get_double("INTERFRAGMENT_CONNECT");

// Whether to treat multiple molecule fragments as a single bonded molecule;
// or via interfragment coordinates ; a primary difference is that in MULTI mode,
// the interfragment coordinates are not redundant. {SINGLE, MULTI}
    s = options.get_str("FRAG_MODE");
    if (s == "SINGLE")     Opt_params.fragment_mode = OPT_PARAMS::SINGLE;
    else if (s == "MULTI") Opt_params.fragment_mode = OPT_PARAMS::MULTI;

// whether to use fixed linear combinations of atoms as reference points for
//interfragment coordinates or whether to use principal axes {FIXED, PRINCIPAL_AXES}
    s = options.get_str("INTERFRAG_MODE");
    if (s == "FIXED")               Opt_params.interfragment_mode = OPT_PARAMS::FIXED;
    else if (s == "PRINCIPAL_AXES") Opt_params.interfragment_mode = OPT_PARAMS::PRINCIPAL_AXES;

// Whether to only generate the internal coordinates and then stop {true, false}
//  Opt_params.generate_intcos_only;
    Opt_params.generate_intcos_only = options.get_bool("INTCOS_GENERATE_EXIT");

// What model Hessian to use to guess intrafragment force constants {SCHLEGEL, FISCHER, SIMPLE, LINDH}
    s = options.get_str("INTRAFRAG_HESS");
    if (s == "FISCHER")       Opt_params.intrafragment_H = OPT_PARAMS::FISCHER;
    else if (s == "SCHLEGEL") Opt_params.intrafragment_H = OPT_PARAMS::SCHLEGEL;
    else if (s == "SIMPLE") Opt_params.intrafragment_H = OPT_PARAMS::SIMPLE;
    else if (s == "LINDH") Opt_params.intrafragment_H = OPT_PARAMS::LINDH;

// Whether to use the default of FISCHER_LIKE force constants for the initial guess {DEFAULT, FISCHER_LIKE}
    s = options.get_str("INTERFRAG_HESS");
    if (s == "DEFAULT")           Opt_params.interfragment_H = OPT_PARAMS::DEFAULT;
    else if (s == "FISCHER_LIKE") Opt_params.interfragment_H = OPT_PARAMS::FISCHER_LIKE;

// Whether to freeze all fragments rigid
    Opt_params.freeze_intrafragment = options.get_bool("FREEZE_INTRAFRAG");

// Whether to freeze all interfragment modes
    Opt_params.freeze_interfragment = options.get_bool("FREEZE_INTERFRAG");

// Add auxiliary bonds for non-bonded (but nearby) atoms.
    Opt_params.add_auxiliary_bonds = options.get_bool("ADD_AUXILIARY_BONDS");

// By default, optking prints and saves the last (previous) geometry at the end of an
// optimization, i.e., the one at which a gradient was computed.  If this keyword is
// set to true, then the structure obtained from the last projected step is printed out and saved instead.
//  Opt_params.write_final_step_geometry = false;
    Opt_params.write_final_step_geometry = options.get_bool("FINAL_GEOM_WRITE");

// Choose from supported Hessian updates {NONE, BFGS, MS, POWELL, BOFILL}
    s = options.get_str("HESS_UPDATE");
    if (s == "NONE")        Opt_params.H_update = OPT_PARAMS::NONE;
    else if (s == "BFGS")   Opt_params.H_update = OPT_PARAMS::BFGS;
    else if (s == "MS")     Opt_params.H_update = OPT_PARAMS::MS;
    else if (s == "POWELL") Opt_params.H_update = OPT_PARAMS::POWELL;
    else if (s == "BOFILL") Opt_params.H_update = OPT_PARAMS::BOFILL;

//  How many previous steps' data to use in Hessian update; 0=use them all ; {integer}
//  Opt_params.H_update_use_last = 6;
    Opt_params.H_update_use_last = options.get_int("HESS_UPDATE_USE_LAST");

// Whether to limit the magnitutde of changes caused by the Hessian update {true, false}
//  Opt_params.H_update_limit = true;
    Opt_params.H_update_limit = options.get_bool("HESS_UPDATE_LIMIT");

// If the above is true, changes to the Hessian from the update are limited to the larger of
// (H_update_limit_scale)*(the previous value) and H_update_limit_max (in au).
//  Opt_params.H_update_limit_scale = 0.50;
//  Opt_params.H_update_limit_max  = 1.0;
    Opt_params.H_update_limit_scale = options.get_double("HESS_UPDATE_LIMIT_SCALE");
    Opt_params.H_update_limit_max = options.get_double("HESS_UPDATE_LIMIT_MAX");

// Whether to use 1/R(AB) for stretching coordinate between fragments (or just R(AB))
//  Opt_params.interfragment_distance_inverse = false;
    Opt_params.interfragment_distance_inverse = options.get_bool("INTERFRAG_DIST_INV");

// For now, this is a general maximum distance for the definition of H-bonds
//  Opt_params.maximum_H_bond_distance = 4.3;
    Opt_params.maximum_H_bond_distance = options.get_double("H_BOND_CONNECT");

// User-specified direction for IRC mapping
    s = options.get_str("IRC_DIRECTION");
    if (s == "FORWARD")        Opt_params.IRC_direction = OPT_PARAMS::FORWARD;
    else if (s == "BACKWARD")  Opt_params.IRC_direction = OPT_PARAMS::BACKWARD;

// Decide how to quit IRC mapping
// if 'ASK' ==> user will be prompted to decide whether to continue when
// the gradient dotted with the previous gradient goes negative
   s = options.get_str("IRC_STOP");
   if (s == "STOP")        Opt_params.IRC_stop = OPT_PARAMS::STOP;
   else if (s == "ASK")  Opt_params.IRC_stop = OPT_PARAMS::ASK;
   else if (s == "GO")    Opt_params.IRC_stop = OPT_PARAMS::GO;

// General optimization criteria
    Opt_params.i_max_force = false;
    Opt_params.i_rms_force = false;
    Opt_params.i_max_DE = false;
    Opt_params.i_max_disp = false;
    Opt_params.i_rms_disp = false;
    Opt_params.i_untampered = false;

    Opt_params.general_conv = options.get_str("G_CONVERGENCE");
    if (Opt_params.general_conv == "QCHEM") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 3.0e-4;  Opt_params.i_max_force = true;
      Opt_params.conv_max_DE    = 1.0e-6;  Opt_params.i_max_DE = true;
      Opt_params.conv_max_disp  = 1.2e-3;  Opt_params.i_max_disp = true;
    }
    else if (Opt_params.general_conv == "MOLPRO") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 3.0e-4;  Opt_params.i_max_force = true;
      Opt_params.conv_max_DE    = 1.0e-6;  Opt_params.i_max_DE = true;
      Opt_params.conv_max_disp  = 3.0e-4;  Opt_params.i_max_disp = true;
    }
    else if (Opt_params.general_conv == "GAU") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 4.5e-4;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 3.0e-4;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_disp  = 1.8e-3;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 1.2e-3;  Opt_params.i_rms_disp = true;
    }
    else if (Opt_params.general_conv == "GAU_TIGHT") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 1.5e-5;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 1.0e-5;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_disp  = 6.0e-5;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 4.0e-5;  Opt_params.i_rms_disp = true;
    }
    else if (Opt_params.general_conv == "GAU_VERYTIGHT") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 2.0e-6;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 1.0e-6;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_disp  = 6.0e-6;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 4.0e-6;  Opt_params.i_rms_disp = true;
    }
    else if (Opt_params.general_conv == "GAU_LOOSE") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 2.5e-3;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 1.7e-3;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_disp  = 1.0e-2;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 6.7e-3;  Opt_params.i_rms_disp = true;
    }
    else if (Opt_params.general_conv == "TURBOMOLE") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 1.0e-3;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 5.0e-4;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_DE    = 1.0e-6;  Opt_params.i_max_DE = true;
      Opt_params.conv_max_disp  = 1.0e-3;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 5.0e-4;  Opt_params.i_rms_disp = true;
    }
    else if (Opt_params.general_conv == "CFOUR") {
      Opt_params.i_untampered = true;
      Opt_params.conv_rms_force = 1.0e-4;  Opt_params.i_rms_force = true;
    }
    else if (Opt_params.general_conv == "NWCHEM_LOOSE") {
      Opt_params.i_untampered = true;
      Opt_params.conv_max_force = 4.5e-3;  Opt_params.i_max_force = true;
      Opt_params.conv_rms_force = 3.0e-3;  Opt_params.i_rms_force = true;
      Opt_params.conv_max_disp  = 5.4e-3;  Opt_params.i_max_disp = true;
      Opt_params.conv_rms_disp  = 3.6e-3;  Opt_params.i_rms_disp = true;
    }

// Specific optimization criteria
    if (options["MAX_FORCE_G_CONVERGENCE"].has_changed()) {
      Opt_params.i_untampered = false;
      Opt_params.i_max_force = true;
      Opt_params.conv_max_force = fabs(options.get_double("MAX_FORCE_G_CONVERGENCE"));
    }
    if (options["RMS_FORCE_G_CONVERGENCE"].has_changed()) {
      Opt_params.i_untampered = false;
      Opt_params.i_rms_force = true;
      Opt_params.conv_rms_force = fabs(options.get_double("RMS_FORCE_G_CONVERGENCE"));
    }
    if (options["MAX_ENERGY_G_CONVERGENCE"].has_changed()) {
      Opt_params.i_untampered = false;
      Opt_params.i_max_DE = true;
      Opt_params.conv_max_DE = fabs(options.get_double("MAX_ENERGY_G_CONVERGENCE"));
    }
    if (options["MAX_DISP_G_CONVERGENCE"].has_changed()) {
      Opt_params.i_untampered = false;
      Opt_params.i_max_disp = true;
      Opt_params.conv_max_disp = fabs(options.get_double("MAX_DISP_G_CONVERGENCE"));
    }
    if (options["RMS_DISP_G_CONVERGENCE"].has_changed()) {
      Opt_params.i_untampered = false;
      Opt_params.i_rms_disp = true;
      Opt_params.conv_rms_disp = fabs(options.get_double("RMS_DISP_G_CONVERGENCE"));
    }
 
    // even if a specific threshold were given, allow for Molpro/Qchem/G03 flex criteria
    if (options.get_bool("FLEXIBLE_G_CONVERGENCE"))
      Opt_params.i_untampered = true;

// Whether to test B matrix and derivative B matrix numerically
//  Opt_params.test_B = false;
//  Opt_params.test_derivative_B = false;
    Opt_params.test_B = options.get_bool("TEST_B");
    Opt_params.test_derivative_B = options.get_bool("TEST_DERIVATIVE_B");

//  Opt_params.print_lvl = 1;
    Opt_params.print_lvl = options.get_int("PRINT");

// Read cartesian Hessian.  Make reading the default for IRC.
    if ((Opt_params.opt_type == OPT_PARAMS::IRC) &&
        (options["CART_HESS_READ"].has_changed() == 0))
      Opt_params.read_cartesian_H = true;
    else
      Opt_params.read_cartesian_H = options.get_bool("CART_HESS_READ");

// only treating "dummy fragments"
    // These are not found in psi4/read_options.cc
    // Not sure if we need these.
  Opt_params.efp_fragments = false;
  Opt_params.efp_fragments_only = false;

//IRC stepsize
  Opt_params.IRC_step_size = options.get_double("IRC_STEP_SIZE");

  // keep internal coordinate definitions file after optimization
  Opt_params.keep_intcos = options.get_bool("KEEP_INTCOS");

  // if we are running only to generate them, then we assume we'll keep them
  if (Opt_params.generate_intcos_only && !options["KEEP_INTCOS"].has_changed())
    Opt_params.keep_intcos = true;

  // for intcos with user-specified equilibrium values - this is the force constant
  //Opt_params.fixed_eq_val_force_constant = options.get_double("INTCO_FIXED_EQ_FORCE_CONSTANT");

  // Currently, a static line search merely displaces along the gradient in internal
  // coordinates generating LINESEARCH_STATIC_N geometries.  The other two keywords
  // control the min and the max of the largest internal coordinate displacement.
  Opt_params.linesearch_static_N   = options.get_int("LINESEARCH_STATIC_N");
  Opt_params.linesearch_static_min = options.get_double("LINESEARCH_STATIC_MIN");
  Opt_params.linesearch_static_max = options.get_double("LINESEARCH_STATIC_MAX");

// consecutive number of backsteps allowed before giving up
  Opt_params.consecutive_backsteps_allowed = options.get_int("CONSECUTIVE_BACKSTEPS");

  // if steepest-descent, then make much larger default
  if (Opt_params.step_type == OPT_PARAMS::SD && !(options["CONSECUTIVE_BACKSTEPS"].has_changed()))
    Opt_params.consecutive_backsteps_allowed = 10;

  Opt_params.geom_maxiter = options.get_int("GEOM_MAXITER");

#elif defined(OPTKING_PACKAGE_QCHEM)

  int i;

// MIN = 0 ; TS = 1 ; IRC = 2      (default 0)
  i = rem_read(REM_GEOM_OPT2_OPT_TYPE); 
  if (i == 0)      Opt_params.opt_type = OPT_PARAMS::MIN;
  else if (i == 1) Opt_params.opt_type = OPT_PARAMS::TS;
  else if (i == 2) Opt_params.opt_type = OPT_PARAMS::IRC;

// RFO = 0 ; NR = 1 ; P_RFO = 2      (default 0)
// defaults should be RFO for MIN; P_RFO for TS
  if (Opt_params.opt_type == OPT_PARAMS::MIN)
    Opt_params.step_type = OPT_PARAMS::RFO;
  else if (Opt_params.opt_type == OPT_PARAMS::TS)
    Opt_params.step_type = OPT_PARAMS::P_RFO;

  // How to check if user specified this?
  i = rem_read(REM_GEOM_OPT2_STEP_TYPE);
  if (i == 0)      Opt_params.step_type = OPT_PARAMS::RFO;
  else if (i == 1) Opt_params.step_type = OPT_PARAMS::NR;
  else if (i == 2) Opt_params.step_type = OPT_PARAMS::P_RFO;

  // Maximum change in an internal coordinate is au; limits on steps are rem / 1000
  i = rem_read(REM_GEOM_OPT2_INTRAFRAG_STEP_LIMIT);     // default is  400 -> 0.4
  Opt_params.intrafragment_step_limit =     i / 1000.0;
  i = rem_read(REM_GEOM_OPT2_INTRAFRAG_STEP_LIMIT_MIN); // default is    1 -> 0.001
  Opt_params.intrafragment_step_limit_min = i / 1000.0;
  i = rem_read(REM_GEOM_OPT2_INTRAFRAG_STEP_LIMIT_MAX); // default is 1000 -> 1.0
  Opt_params.intrafragment_step_limit_max = i / 1000.0;
  i = rem_read(REM_GEOM_OPT2_INTERFRAG_STEP_LIMIT);
  Opt_params.interfragment_step_limit     = i / 1000.0; // default is  400 -> 0.4

// follow root   (default 0)
  Opt_params.rfo_follow_root = rem_read(REM_GEOM_OPT2_RFO_FOLLOW_ROOT);

// which root    (default is 0 for minimum) - should be corrected in qchem default
  Opt_params.rfo_root = rem_read(REM_GEOM_OPT2_RFO_ROOT);

// scale = i / 10   (default 13)
  i = rem_read(REM_GEOM_OPT2_SCALE_CONNECTIVITY);
  Opt_params.scale_connectivity = i / 10.0;

// scale = i / 10   (default 18) // not yet implemented in QChem
  i = rem_read(REM_GEOM_OPT2_INTERFRAGMENT_SCALE_CONNECTIVITY);
  Opt_params.interfragment_scale_connectivity = i/ 10;

// multi-mode (0=single ; 1= multi) (default 0)
  i = rem_read(REM_GEOM_OPT2_FRAGMENT_MODE);
  if (i == 0)      Opt_params.fragment_mode = OPT_PARAMS::SINGLE;
  else if (i == 1) Opt_params.fragment_mode = OPT_PARAMS::MULTI;

  i = rem_read(REM_GEOM_OPT2_INTERFRAGMENT_MODE);
  if (i == 0)      Opt_params.interfragment_mode = OPT_PARAMS::FIXED;
  else if (i == 1) Opt_params.interfragment_mode = OPT_PARAMS::PRINCIPAL_AXES;

// only generate intcos
  Opt_params.generate_intcos_only = rem_read(REM_GEOM_OPT2_GENERATE_INTCOS_ONLY);

// model 0=FISCHER ; 1 = SCHLEGEL (default 0) ; 2 = simple
  i = rem_read(REM_GEOM_OPT2_INTRAFRAGMENT_H);
  if (i == 0)      Opt_params.intrafragment_H = OPT_PARAMS::FISCHER;
  else if (i == 1) Opt_params.intrafragment_H = OPT_PARAMS::SCHLEGEL;
  else if (i == 2) Opt_params.intrafragment_H = OPT_PARAMS::SIMPLE;
  else if (i == 3) Opt_params.intrafragment_H = OPT_PARAMS::LINDH;

// interfragment 0=DEFAULT ; 1=FISCHER_LIKE
  i = rem_read(REM_GEOM_OPT2_INTERFRAGMENT_H);
  if (i == 0)      Opt_params.interfragment_H = OPT_PARAMS::DEFAULT;
  else if (i == 1) Opt_params.interfragment_H = OPT_PARAMS::FISCHER_LIKE;

// Whether to freeze all fragments rigid (default 0);
  Opt_params.freeze_intrafragment = rem_read(REM_GEOM_OPT2_FREEZE_INTRAFRAGMENT);

// Whether to freeze all interfragment modes:
  Opt_params.freeze_interfragment = rem_read(REM_GEOM_OPT2_FREEZE_INTERFRAGMENT);

// write final step ; default (false);
  Opt_params.write_final_step_geometry = rem_read(REM_GEOM_OPT2_WRITE_FINAL_STEP_GEOMETRY);

// {NONE, BFGS, MS, POWELL, BOFILL} (default 1)
  i = rem_read(REM_GEOM_OPT2_H_UPDATE);
  if (i == 0) Opt_params.H_update = OPT_PARAMS::NONE;
  else if (i == 1) Opt_params.H_update = OPT_PARAMS::BFGS;
  else if (i == 2) Opt_params.H_update = OPT_PARAMS::MS;
  else if (i == 3) Opt_params.H_update = OPT_PARAMS::POWELL;
  else if (i == 4) Opt_params.H_update = OPT_PARAMS::BOFILL;

// previous steps to use ; (0=all) ; default (6)
  Opt_params.H_update_use_last = rem_read(REM_GEOM_OPT2_H_UPDATE_USE_LAST);

// limit hessian changes (default true)
  Opt_params.H_update_limit = rem_read(REM_GEOM_OPT2_H_UPDATE_LIMIT);

// scale is i / 10 (default 5 -> 0.5)  ; max is i / 10 (default 10 -> 1.0)
  i = rem_read(REM_GEOM_OPT2_H_UPDATE_LIMIT_SCALE);
  Opt_params.H_update_limit_scale = i / 10;

  i = rem_read(REM_GEOM_OPT2_H_UPDATE_LIMIT_MAX);
  Opt_params.H_update_limit_max  = i / 10;

// use 1/R(AB) ; (default 0)
  Opt_params.interfragment_distance_inverse = rem_read(REM_GEOM_OPT2_INTERFRAGMENT_DISTANCE_INVERSE);

// H-bond distance = i / 10 ; default (43 -> 4.3)
  i = rem_read(REM_GEOM_OPT2_MAXIMUM_H_BOND_DISTANCE);
  Opt_params.maximum_H_bond_distance = i / 10;

// QCHEM optimization criteria ; names adapted from old QCHEM
  i = rem_read(REM_GEOM_OPT2_TOL_GRADIENT);
  Opt_params.conv_max_force = i / 1.0e6; // default (300 -> 3e-4)

  i = rem_read(REM_GEOM_OPT2_TOL_DISPLACEMENT);
  Opt_params.conv_max_disp  = i / 1.0e6; // default (1200 -> 1.2e-3)

  i = rem_read(REM_GEOM_OPT2_TOL_ENERGY);
  Opt_params.conv_max_DE    = i / 1.0e8; // default (100 -> 1.0e-6)

// Turn "on" these convergence criteria; At least for now, QChem, doesn't
// support all the special string names for convergence criteria.
  Opt_params.i_untampered = true; // allow flex between force and displacement
  Opt_params.i_max_force = true;
  Opt_params.i_max_disp = true;
  Opt_params.i_max_DE = true;

// test B (default 0)
  Opt_params.test_B = rem_read(REM_GEOM_OPT2_TEST_B);
  Opt_params.test_derivative_B = rem_read(REM_GEOM_OPT2_TEST_DERIVATIVE_B);

// (default 1)
  Opt_params.print_lvl = rem_read(REM_GEOM_OPT2_PRINT_LVL);

// read Hessian (default 0)
  Opt_params.read_cartesian_H = rem_read(REM_GEOM_OPT2_READ_CARTESIAN_H);

// This optimizer will not work unless only EFP fragments are present
// Last I tried, I can't even get geometry data when running EFP_opt.in
  Opt_params.efp_fragments = rem_read(REM_EFP);

// are ONLY EFP fragments present
  if(Opt_params.efp_fragments)
    Opt_params.efp_fragments_only = rem_read(REM_EFP_FRAGMENTS_ONLY);
  else {
    Opt_params.efp_fragments_only = false;
  }

  // for intcos with user-specified equilibrium values - this is the force constant
  //i = rem_read(REM_INTCO_FIXED_EQ_FORCE_CONSTANT);
  //Opt_params.fixed_eq_val_force_constant = i / 10; // default (20 -> 2 au)

  Opt_params.consecutive_backsteps_allowed = rem_read(REM_GEOM_OPT2_CONSECUTIVE_BACKSTEPS);

  // if steepest-descent, then make much larger default
  if (Opt_params.step_type == OPT_PARAMS::SD && RemUninitialized(REM_GEOM_OPT2_CONSECUTIVE_BACKSTEPS))
    Opt_params.consecutive_backsteps_allowed = 10;

//TO DO: initialize IRC_step_size for Q-Chem

#endif

// Strings that carry user-specified constraints
// "frozen" means unchanging, while "fixed" means eq. value is specified
#if defined(OPTKING_PACKAGE_PSI)
  Opt_params.frozen_distance_str = options.get_str("FROZEN_DISTANCE");
  Opt_params.frozen_bend_str     = options.get_str("FROZEN_BEND");
  Opt_params.frozen_dihedral_str = options.get_str("FROZEN_DIHEDRAL");

  Opt_params.fixed_distance_str = options.get_str("FIXED_DISTANCE");
  Opt_params.fixed_bend_str     = options.get_str("FIXED_BEND");
  Opt_params.fixed_dihedral_str = options.get_str("FIXED_DIHEDRAL");
#elif defined(OPTKING_PACKAGE_QCHEM)
  // Read QChem input and write all the frozen distances into a string
  if (rem_read(REM_GEOM_OPT2_FROZEN_DISTANCES) > 0) {
    INTEGER n_frozen = rem_read(REM_GEOM_OPT2_FROZEN_DISTANCES);
    double* fd = init_array(2*n_frozen);
    FileMan(FM_READ,FILE_FROZEN_DISTANCES,FM_DP,2*n_frozen,0,FM_BEG,fd);

    std::stringstream atoms;
    for (int i=0; i<2*n_frozen; ++i)
      atoms << (int) fd[i] << ' ';
    Opt_params.frozen_distance_str = atoms.str(); 
    
    free_array(fd);
  }
  // Read QChem input and write all the frozen bends into a string
  if (rem_read(REM_GEOM_OPT2_FROZEN_BENDS) > 0) {
    INTEGER n_frozen = rem_read(REM_GEOM_OPT2_FROZEN_BENDS);
    double* fd = init_array(3*n_frozen);
    FileMan(FM_READ,FILE_FROZEN_BENDS,FM_DP,3*n_frozen,0,FM_BEG,fd);

    std::stringstream atoms;
    for (int i=0; i<3*n_frozen; ++i)
      atoms << (int) fd[i] << ' ';
    Opt_params.frozen_bend_str = atoms.str(); 
    
    free_array(fd);
  }
  // Read QChem input and write all the frozen dihedrals into a string
  if (rem_read(REM_GEOM_OPT2_FROZEN_DIHEDRALS) > 0) {
    INTEGER n_frozen = rem_read(REM_GEOM_OPT2_FROZEN_DIHEDRALS);
    double* fd = init_array(4*n_frozen);
    FileMan(FM_READ,FILE_FROZEN_DIHEDRALS,FM_DP,4*n_frozen,0,FM_BEG,fd);

    std::stringstream atoms;
    for (int i=0; i<4*n_frozen; ++i)
      atoms << (int) fd[i] << ' ';
    Opt_params.frozen_dihedral_str = atoms.str(); 
    
    free_array(fd);
  }
#endif

// ** Items are below unlikely to need modified

// For RFO step, eigenvectors of augmented Hessian are divided by the last
// element unless it is smaller than this value {double}
  Opt_params.rfo_normalization_min = 1.0e-8;

// how close to pi should a torsion be to assume it may have passed through 180
  Opt_params.fix_tors_near_pi = _pi / 2;

// torsional angles will not be computed if the contained bond angles are within
// this many radians of zero or 180. (< ~10 and > ~170 degrees)
  Opt_params.tors_angle_lim = 0.17;

// only used for determining which atoms in a fragment are acceptable for use
// as reference atoms.  We avoid collinear sets.
// angle is 0/pi if the bond angle is within this fraction of pi from 0/pi
  Opt_params.interfrag_collinear_tol = 0.01;

// cos(torsional angle) must be this close to -1/+1 for angle to count as 0/pi
  Opt_params.tors_cos_tol = 1e-10;

// if bend exceeds this value, then also create linear bend complement
  Opt_params.linear_bend_threshold = 3.05; // about 175 degrees

// threshold for which entries in diagonalized redundant matrix are kept and inverted
// while computing a generalized inverse of a matrix
  Opt_params.redundant_eval_tol = 1.0e-10;

// Parameters that control the backtransformation to cartesian coordinates
  Opt_params.bt_max_iter = 25;
  Opt_params.bt_dx_conv = 1.0e-6;
  Opt_params.bt_dx_conv_rms_change = 1.0e-12;
  //Opt_params.bt_dx_conv = 1.0e-10;
  //Opt_params.bt_dx_conv_rms_change = 1.0e-14;

// Hessian update is avoided if the denominators (Dq*Dq) or (Dq*Dg) are smaller than this
  Opt_params.H_update_den_tol = 1e-7;

// Hessian update is avoided if any internal coordinate has changed by more than this in radians/au
  Opt_params.H_update_dq_tol = 0.5;

// Some parameter error-checking / modification
  if (Opt_params.efp_fragments_only) {
    Opt_params.test_B = false;
    Opt_params.test_derivative_B = false;
  }

  if (Opt_params.print_lvl > 1) print_params();

}

void print_params(void) {

  fprintf(outfile, "conv_max_force         = %18.2e\n", Opt_params.conv_max_force);
  fprintf(outfile, "conv_rms_force         = %18.2e\n", Opt_params.conv_rms_force);
  fprintf(outfile, "conv_max_DE            = %18.2e\n", Opt_params.conv_max_DE);
  fprintf(outfile, "conv_max_disp          = %18.2e\n", Opt_params.conv_max_disp);
  fprintf(outfile, "conv_rms_disp          = %18.2e\n", Opt_params.conv_rms_disp);

  fprintf(outfile, "scale_connectivity     = %18.2e\n", Opt_params.scale_connectivity);
  fprintf(outfile, "interfragment_scale_connectivity = %18.2e\n",
    Opt_params.interfragment_scale_connectivity);

  if (Opt_params.fragment_mode == OPT_PARAMS::SINGLE)
  fprintf(outfile, "fragment_mode          = %18s\n", "single");
  else if (Opt_params.fragment_mode == OPT_PARAMS::MULTI)
  fprintf(outfile, "fragment_mode          = %18s\n", "multi");

  if (Opt_params.interfragment_mode == OPT_PARAMS::FIXED)
  fprintf(outfile, "interfragment_mode        = %18s\n", "fixed");
  else if (Opt_params.interfragment_mode == OPT_PARAMS::PRINCIPAL_AXES)
  fprintf(outfile, "interfragment_mode        = %18s\n", "principal axes");

  if (Opt_params.generate_intcos_only)
  fprintf(outfile, "generate_intcos_only   = %18s\n", "true");
  else
  fprintf(outfile, "generate_intcos_only   = %18s\n", "false");

  if (Opt_params.rfo_follow_root)
  fprintf(outfile, "rfo_follow_root        = %18s\n", "true");
  else
  fprintf(outfile, "rfo_follow_root        = %18s\n", "false");

  fprintf(outfile, "rfo_root               = %18d\n", Opt_params.rfo_root);

  fprintf(outfile, "rfo_normalization_min  = %18.2e\n", Opt_params.rfo_normalization_min);

  if (Opt_params.step_type == OPT_PARAMS::NR)
  fprintf(outfile, "step_type              = %18s\n", "N-R");
  else if (Opt_params.step_type == OPT_PARAMS::RFO)
  fprintf(outfile, "step_type              = %18s\n", "RFO");
  else if (Opt_params.step_type == OPT_PARAMS::P_RFO)
  fprintf(outfile, "step_type              = %18s\n", "P_RFO");
  else if (Opt_params.step_type == OPT_PARAMS::LINESEARCH_STATIC)
  fprintf(outfile, "step_type              = %18s\n", "Static linesearch");

  fprintf(outfile, "linesearch_static_N    = %18d\n", Opt_params.linesearch_static_N);
  fprintf(outfile, "linesearch_static_min  = %18.3e\n", Opt_params.linesearch_static_min);
  fprintf(outfile, "linesearch_static_max  = %18.3e\n", Opt_params.linesearch_static_max);

  if (Opt_params.intrafragment_H == OPT_PARAMS::FISCHER)
  fprintf(outfile, "intrafragment_H        = %18s\n", "Fischer");
  else if (Opt_params.intrafragment_H == OPT_PARAMS::SCHLEGEL)
  fprintf(outfile, "intrafragment_H        = %18s\n", "Schlegel");
  else if (Opt_params.intrafragment_H == OPT_PARAMS::SIMPLE)
  fprintf(outfile, "intrafragment_H        = %18s\n", "Simple");

  if (Opt_params.interfragment_H == OPT_PARAMS::DEFAULT)
  fprintf(outfile, "interfragment_H        = %18s\n", "Default");
  else if (Opt_params.interfragment_H == OPT_PARAMS::FISCHER_LIKE)
  fprintf(outfile, "interfragment_H        = %18s\n", "Fischer_like");

  if (Opt_params.H_update == OPT_PARAMS::NONE)
  fprintf(outfile, "H_update               = %18s\n", "None");
  else if (Opt_params.H_update == OPT_PARAMS::BFGS)
  fprintf(outfile, "H_update               = %18s\n", "BFGS");
  else if (Opt_params.H_update == OPT_PARAMS::MS)
  fprintf(outfile, "H_update               = %18s\n", "MS");
  else if (Opt_params.H_update == OPT_PARAMS::POWELL)
  fprintf(outfile, "H_update               = %18s\n", "Powell");
  else if (Opt_params.H_update == OPT_PARAMS::BOFILL)
  fprintf(outfile, "H_update               = %18s\n", "Bofill");

  fprintf(outfile, "H_update_use_last      = %18d\n", Opt_params.H_update_use_last);

  if (Opt_params.freeze_intrafragment)
  fprintf(outfile, "freeze_intrafragment   = %18s\n", "true");
  else
  fprintf(outfile, "freeze_intrafragment   = %18s\n", "false");

  fprintf(outfile, "intrafragment_step_limit=%18.2e\n", Opt_params.intrafragment_step_limit);

  fprintf(outfile, "interfragment_step_limit=%18.2e\n", Opt_params.interfragment_step_limit);

  if (Opt_params.H_update_limit)
    fprintf(outfile, "H_update_limit         = %18s\n", "true");
  else
    fprintf(outfile, "H_update_limit         = %18s\n", "false");

  fprintf(outfile, "H_update_limit_scale   = %18.2e\n", Opt_params.H_update_limit_scale);
  fprintf(outfile, "H_update_limit_max     = %18.2e\n", Opt_params.H_update_limit_max);

  if (Opt_params.interfragment_distance_inverse)
  fprintf(outfile, "interfragment_distance_inverse=%12s\n", "true");
  else
  fprintf(outfile, "interfragment_distance_inverse=%12s\n", "false");

  if (Opt_params.write_final_step_geometry)
  fprintf(outfile, "write_final_step_geometry= %16s\n", "true");
  else
  fprintf(outfile, "write_final_step_geometry= %16s\n", "false");

  fprintf(outfile, "maximum_H_bond_distance= %18.2e\n", Opt_params.maximum_H_bond_distance);

  if (Opt_params.read_cartesian_H)
  fprintf(outfile, "read_cartesian_H       = %18s\n", "true");
  else
  fprintf(outfile, "read_cartesian_H       = %18s\n", "false");

  if (Opt_params.efp_fragments)
  fprintf(outfile, "efp_fragments          = %18s\n", "true");
  else
  fprintf(outfile, "efp_fragments          = %18s\n", "false");

  if (Opt_params.efp_fragments_only)
  fprintf(outfile, "efp_fragments_only     = %18s\n", "true");
  else
  fprintf(outfile, "efp_fragments_only     = %18s\n", "false");

  fprintf(outfile, "frozen_distance: \n");
  fprintf(outfile, "%s\n", Opt_params.frozen_distance_str.c_str());
  fprintf(outfile, "frozen_bend: \n");
  fprintf(outfile, "%s\n", Opt_params.frozen_bend_str.c_str());
  fprintf(outfile, "frozen_dihedral: \n");
  fprintf(outfile, "%s\n", Opt_params.frozen_dihedral_str.c_str());

  fprintf(outfile, "fixed_distance: \n");
  fprintf(outfile, "%s\n", Opt_params.fixed_distance_str.c_str());
  fprintf(outfile, "fixed_bend: \n");
  fprintf(outfile, "%s\n", Opt_params.fixed_bend_str.c_str());
  fprintf(outfile, "fixed_dihedral: \n");
  fprintf(outfile, "%s\n", Opt_params.fixed_dihedral_str.c_str());
}

}

