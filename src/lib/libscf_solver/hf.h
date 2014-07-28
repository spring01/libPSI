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

#ifndef HF_H
#define HF_H
/*
 *  hf.h
 *  matrix
 *
 *  Created by Justin Turney on 4/9/08.
 *  Copyright 2008 by Justin M. Turney, Ph.D.. All rights reserved.
 *
 */


#include <vector>
#include <libpsio/psio.hpp>
#include <libmints/wavefunction.h>
#include <libmints/basisset.h>
#include <libmints/vector.h>
#include <libdiis/diismanager.h>
#include <libdiis/diisentry.h>
#include <psi4-dec.h>
#include <libqt/qt.h>

namespace boost {
template<class T> class shared_ptr;
}

namespace psi {
class Matrix;
class Vector;
class SimpleVector;
class TwoBodySOInt;
class JK;
namespace scf {

class HF : public Wavefunction {
protected:

    /// The kinetic energy matrix
    SharedMatrix T_;
    /// The 1e potential energy matrix
    SharedMatrix V_;
    /// The orthogonalization matrix (symmetric or canonical)
    SharedMatrix X_;
    /// Temporary matrix for diagonalize_F
    SharedMatrix diag_temp_;
    /// Temporary matrix for diagonalize_F
    SharedMatrix diag_F_temp_;
    /// Temporary matrix for diagonalize_F
    SharedMatrix diag_C_temp_;

    /// Old C Alpha matrix (if needed for MOM)
    SharedMatrix Ca_old_;
    /// Old C Beta matrix (if needed for MOM)
    SharedMatrix Cb_old_;

    /// Previous iteration's energy and current energy
    double Eold_;
    double E_;

    /// Table of energy components
    std::map<std::string, double> energies_;

    /// The RMS error in the density
    double Drms_;

    /// Max number of iterations for HF
    int maxiter_;

    /// Fail if we don't converge by maxiter?
    bool fail_on_maxiter_;

    /// Current Iteration
    int iteration_;

    /// Nuclear repulsion energy
    double nuclearrep_;

    /// Whether DIIS was performed this iteration, or not
    bool diis_performed_;

    /// DOCC vector from input (if found)
    bool input_docc_;

    /// SOCC vector from input (if found)
    bool input_socc_;

    //Initial SAD doubly occupied may be more than ndocc
    int sad_nocc_[8];

    /// Mapping arrays
    int *so2symblk_;
    int *so2index_;

    /// SCF algorithm type
    std::string scf_type_;

    /// Perturb the Hamiltonian?
    int perturb_h_;
    /// How big of a perturbation
    double lambda_;
    /// With what...
    enum perturb { nothing, dipole_x, dipole_y, dipole_z, embpot, dx, sphere };
    perturb perturb_;

    /// The value below which integrals are neglected
    double integral_threshold_;

    /// The soon to be ubiquitous JK object
    boost::shared_ptr<JK> jk_;

    /// Are we to do MOM?
    bool MOM_enabled_;
    /// Are we to do excited-state MOM?
    bool MOM_excited_;
    /// MOM started?
    bool MOM_started_;
    /// MOM performed?
    bool MOM_performed_;

    /// Are we to fractionally occupy?
    bool frac_enabled_;
    /// Frac started? (Same thing as frac_performed_)
    bool frac_performed_;

    /// DIIS manager intiialized?
    bool initialized_diis_manager_;
    /// DIIS manager for all SCF wavefunctions
    boost::shared_ptr<DIISManager> diis_manager_;

    /// How many min vectors for DIIS
    int min_diis_vectors_;
    /// How many max vectors for DIIS
    int max_diis_vectors_;
    /// When do we start collecting vectors for DIIS
    int diis_start_;
    /// Are we even using DIIS?
    int diis_enabled_;

    /// The amount (%) of the previous orbitals to mix in during SCF damping
    double damping_percentage_;
    /// The energy convergence at which SCF damping is disabled
    double damping_convergence_;
    /// Whether to use SCF damping
    bool damping_enabled_;
    /// Whether damping was actually performed this iteration
    bool damping_performed_;

    // parameters for hard-sphere potentials
    double radius_; // radius of spherical potential
    double thickness_; // thickness of spherical barrier
    int r_points_; // number of radial integration points
    int theta_points_; // number of colatitude integration points
    int phi_points_; // number of azimuthal integration points

public:
    /// Nuclear contributions
    Vector nuclear_dipole_contribution_;
    Vector nuclear_quadrupole_contribution_;

    /// The number of iterations needed to reach convergence
    int iterations_needed() {return iterations_needed_;}

    /// The JK object (or null if it has been deleted)
    boost::shared_ptr<JK> jk() const { return jk_; }

    /// The RMS error in the density
    double rms_density_error() {return Drms_;}
protected:

    /// Formation of H is the same regardless of RHF, ROHF, UHF
    // Temporarily converting to virtual function for testing embedding
    // potentials.  TDC, 5/23/12.
    virtual void form_H();
    /// Formation of S^+1/2 and S^-1/2 are the same
    void form_Shalf();

    /// Prints the orbital occupation
    void print_occupation();

    /// Perform casting of C from old basis to new basis if desired.
    SharedMatrix BasisProjection(SharedMatrix Cold, int* napi, boost::shared_ptr<BasisSet> old_basis, boost::shared_ptr<BasisSet> new_basis);

    /// Common initializer
    void common_init();

    /// Clears memory and closes files (Should they be open) prior to correlated code execution
    virtual void finalize();

    /// Figure out how to occupy the orbitals in the absence of DOCC and SOCC
    void find_occupation();

    /// Maximum overlap method for prevention of oscillation/excited state SCF
    void MOM();
    /// Start the MOM algorithm (requires one iteration worth of setup)
    void MOM_start();

    /// Fractional occupation UHF/UKS
    void frac();
    /// Renormalize orbitals to 1.0 before saving to chkpt
    void frac_renormalize();

    /// Check the stability of the wavefunction, and correct (if requested)
    virtual void stability_analysis();
    void print_stability_analysis(std::vector<std::pair<double, int> > &vec);


    /// Determine how many core and virtual orbitals to freeze
    void compute_fcpi();
    void compute_fvpi();

    /// Prints the orbitals energies and symmetries (helper method)
    void print_orbitals(const char* header, std::vector<std::pair<double,
                        std::pair<const char*, int> > > orbs);

    /// Prints the orbitals in arbitrary order (works with MOM)
    void print_orbitals();

    /// Prints the energy breakdown from this SCF
    void print_energies();

    /// Prints some opening information
    void print_header();

    /// Prints some details about nsopi/nmopi, and initial occupations
    void print_preiterations();

    /// Do any needed integral setup
    virtual void integrals();

    /// Which set of iterations we're on in this computation, e.g., for stability
    /// analysis, where we want to retry SCF without going through all of the setup
    int attempt_number_;

    /// The number of electrons
    int nelectron_;

    /// The charge of the system
    int charge_;

    /// The multiplicity of the system (specified as 2 Ms + 1)
    int multiplicity_;

    /// The number of iterations need to reach convergence
    int iterations_needed_;

    /// Compute energy for the iteration.
    virtual double compute_E() = 0;

    /// Save the current density and energy.
    virtual void save_density_and_energy() = 0;

    /// Check MO phases
    void check_phases();

    /// SAD Guess and propagation
    void compute_SAD_guess();

    /// Reset to regular occupation from the fractional occupation
    void reset_SAD_occupation();

    /// Form the guess (gaurantees C, D, and E)
    virtual void guess();

    /** Computes the density matrix (D_) */
    virtual void form_D() =0;

    /** Applies damping to the density update */
    virtual void damp_update();

    /** Compute the MO coefficients (C_) */
    virtual void form_C() =0;

    /** Transformation, diagonalization, and backtransform of Fock matrix */
    virtual void diagonalize_F(const SharedMatrix& F, SharedMatrix& C, boost::shared_ptr<Vector>& eps);

    /** Computes the Fock matrix */
    virtual void form_F() =0;

    /** Computes the initial MO coefficients (default is to call form_C) */
    virtual void form_initial_C() { form_C(); }

    /** Forms the G matrix */
    virtual void form_G() =0;

    /** Computes the initial energy. */
    virtual double compute_initial_E() { return 0.0; }

    /** Test convergence of the wavefunction */
    virtual bool test_convergency() { return false; }

    /** Compute/print spin contamination information (if unrestricted) **/
    virtual void compute_spin_contamination();

    /** Saves information to the checkpoint file */
    virtual void save_information() {}

    /** Compute the orbital gradient */
    virtual void compute_orbital_gradient(bool save_diis) {}

    /** Performs DIIS extrapolation */
    virtual bool diis() { return false; }

    /** Form Fia (for DIIS) **/
    virtual SharedMatrix form_Fia(SharedMatrix Fso, SharedMatrix Cso, int* noccpi);

    /** Form X'(FDS - SDF)X (for DIIS) **/
    virtual SharedMatrix form_FDSmSDF(SharedMatrix Fso, SharedMatrix Dso);

    /** Save orbitals to use later as a guess **/
    virtual void save_orbitals();

    /** Load orbitals from previous computation, projecting if needed **/
    virtual void load_orbitals();

    /** Save SAPT info (TODO: Move to Python driver **/
    virtual void save_sapt_info() {}

    /** Saves all wavefunction information to the checkpoint file*/
    void dump_to_checkpoint();

public:
    HF(Options& options, boost::shared_ptr<PSIO> psio, boost::shared_ptr<Chkpt> chkpt);
    HF(Options& options, boost::shared_ptr<PSIO> psio);

    virtual ~HF();

    virtual double compute_energy();
};

}} // Namespaces

#endif
