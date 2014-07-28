#ifndef _PSI4_SRC_BIN_DCFT_DCFT_H_
#define _PSI4_SRC_BIN_DCFT_DCFT_H_

#include <libmints/matrix.h>
#include <libmints/vector.h>
#include <libmints/wavefunction.h>
#include <libdpd/dpd.h>
#include <libciomr/libciomr.h>
#include <libmints/dimension.h>

// Handy mints timer macros, requires libqt to be included
#ifdef DCFT_TIMER
#   include <libqt/qt.h>

#   define dcft_timer_on(a) timer_on((a));
#   define dcft_timer_off(a) timer_off((a));
#else
#   define dcft_timer_on(a)
#   define dcft_timer_off(a)
#endif

namespace psi{

class Options;
class PSIO;
class Chkpt;
class Matrix;
class Vector;
class IntegralTransform;

namespace dcft{

class DCFTSolver:public Wavefunction
{
public:
    DCFTSolver(boost::shared_ptr<Wavefunction> reference_wavefunction, Options &options);
    ~DCFTSolver();

    double compute_energy();
    virtual bool same_a_b_orbs() const { return false; }
    virtual bool same_a_b_dens() const { return false; }

protected:
    IntegralTransform *_ints;

    void finalize();
    void transform_integrals();
    void build_lambda();
    void init();
    void compute_dcft_energy();
    void compute_cepa0_energy();
    void update_lambda_from_residual();
    void compute_scf_energy();
    void mp2_guess();
    void build_tau();
    void transform_tau();
    void build_gtau();
    void print_opdm();
    void write_orbitals_to_checkpoint();
    void check_n_representability();
    void print_orbital_energies();
    void find_occupation(SharedVector &evals_a, SharedVector &evals_b, bool forcePrint = false);
    void build_intermediates();
    void process_so_ints();
    void build_G();
    void build_tensors();
    void build_denominators();
    void update_fock();
    void dump_density();
    void mulliken_charges();
    void dpd_buf4_add(dpdbuf4 *A, dpdbuf4 *B, double alpha);
    void scf_guess();
    void half_transform(dpdbuf4 *A, dpdbuf4 *B, SharedMatrix& C1, SharedMatrix& C2,
                        int *mospi_left, int *mospi_right, int **so_row, int **mo_row,
                        bool backwards, double alpha, double beta);
    void file2_transform(dpdfile2 *A, dpdfile2 *B, SharedMatrix C, bool backwards);
    void AO_contribute(dpdbuf4 *tau1_AO, dpdbuf4 *tau2_AO, int p, int q,
                       int r, int s, double value, dpdfile2* = NULL, dpdfile2* = NULL, dpdfile2* = NULL);
    void compute_tau_squared();
    void compute_energy_tau_squared();
    //void AO_contribute(dpdfile2 *tau1_AO, dpdfile2 *tau2_AO, int p, int q,
    //        int r, int s, double value);
    bool correct_mo_phases(bool dieOnError = true);
    double compute_lambda_residual();
    double compute_scf_error_vector();
    double update_scf_density(bool damp = false);
    void run_twostep_dcft();
    void run_simult_dcft();
    // DCFT analytic gradient subroutines
    virtual SharedMatrix compute_gradient();
    void response_guess();
    void gradient_init();
    void compute_density();
    void compute_lagrangian_OV();
    void compute_lagrangian_VO();
    void iterate_orbital_response();
    void orbital_response_guess();
    void compute_orbital_response_intermediates();
    double update_orbital_response();
    double compute_response_coupling();
    void iterate_cumulant_response();
    void cumulant_response_guess();
    void build_perturbed_tau();
    void compute_cumulant_response_intermediates();
    double compute_cumulant_response_residual();
    void update_cumulant_response();
    void compute_lagrangian_OO();
    void compute_lagrangian_VV();
    void compute_ewdm();
    void compute_density_VVVV();
    // Quadratically-convergent DCFT
    void run_qc_dcft();
    void qc_dcft_init();
    void compute_orbital_gradient();
    void form_idps();
    void compute_sigma_vector();
    int iterate_conjugate_gradients();
    void check_qc_convergence();
    void update_cumulant_and_orbitals();
    void run_davidson();
    void davidson_guess();
    // Exact Tau
    void refine_tau();
    void compute_F_intermediate();
    void form_density_weighted_fock();
    // FNO-DCFT
    void form_nso_basis();

    bool augment_b(double *vec, double tol);
    /// Whether to force the code to keep the same occupation from SCF
    bool lock_occupation_;
    /// Controls convergence of the orbital updates
    bool scfDone_;
    /// Controls convergence of the decnsity cumulant updates
    bool lambdaDone_;
    /// Controls convergence of the idempotent one-particle density
    bool densityConverged_;
    /// Controls convergence of the DCFT energy
    bool energyConverged_;
    /// The maximum number of lambda iterations per update
    int lambdamaxiter_;
    /// The maximum number of SCF iterations per update
    int scfmaxiter_;
    /// The amount of information to print
    int print_;
    /// The number of unique pairs of symmetrized atomic orbitals
    int ntriso_;
    /// The number of active alpha electrons
    int nalpha_;
    /// The number of active beta electrons
    int nbeta_;
    /// The number of virtual alpha orbitals
    int navir_;
    /// The number of virtual beta orbitals
    int nbvir_;
    /// The maximum size of the DIIS subspace
    int maxdiis_;
    /// The number of DIIS vectors needed for extrapolation
    int mindiisvecs_;
    /// The maximum number of iterations
    int maxiter_;
    /// The current number of macroiteration for energy or gradient computation
    int iter_;
    // Quadratically-convergent DCFT
    /// The total number of independent pairs for the current NR step
    int nidp_;
    /// The number of orbital independent pairs for the current NR step (Alpha spin)
    int orbital_idp_a_;
    /// The number of orbital independent pairs for the current NR step (Beta spin)
    int orbital_idp_b_;
    /// The total number of orbital independent pairs for the current NR step
    int orbital_idp_;
    /// The number of cumulant independent pairs for the current NR step (Alpha-Alpha spin)
    int lambda_idp_aa_;
    /// The number of cumulant independent pairs for the current NR step (Alpha-Beta spin)
    int lambda_idp_ab_;
    /// The number of cumulant independent pairs for the current NR step (Beta-Beta spin)
    int lambda_idp_bb_;
    /// The total number of cumulant independent pairs for the current NR step
    int lambda_idp_;
    /// The maximum number of IDPs ever possible
    int dim_;
    /// The lookup array that determines which compound indices belong to IDPs and which don't
    int *lookup_;
    /// The number of the guess subspace vectors for the Davidson diagonalization
    int nguess_;
    /// The dimension of the subspace in the Davidson diagonalization
    int b_dim_;
    /// Convergence of the residual in the Davidson diagonalization
    double r_convergence_;
    /// The number of vectors that can be added during the iteration in the Davidson diagonalization
    int n_add_;
    /// The number of eigenvalues requested in the stability check
    int nevals_;
    /// The maximum size of the subspace in stability check
    int max_space_;
    /// The number of occupied alpha orbitals per irrep
    Dimension naoccpi_;
    /// The number of occupied beta orbitals per irrep
    Dimension nboccpi_;
    /// The number of virtual alpha orbitals per irrep
    Dimension navirpi_;
    /// The number of virtual beta orbitals per irrep
    Dimension nbvirpi_;
    /// The nuclear repulsion energy in Hartree
    double enuc_;
    /// The cutoff below which and integral is assumed to be zero
    double int_tolerance_;
    /// The RMS value of the error vector after the SCF iterations
    double scf_convergence_;
    /// The RMS value of the change in lambda after the lambda iterations
    double lambda_convergence_;
    /// The RMS value of the change in the orbital response
    double orbital_response_rms_;
    /// The RMS value of the change in the cumulant response
    double cumulant_response_rms_;
    /// The RMS value of the change in the coupling of orbital and cumulant response
    double response_coupling_rms_;
    /// The convergence criterion for the lambda iterations
    double lambda_threshold_;
    /// The convergence criterion for the scf iterations
    double scf_threshold_;
    /// The convergence that must be achieved before DIIS extrapolation starts
    double diis_start_thresh_;
    /// The SCF component of the energy
    double scf_energy_;
    /// The Lambda component of the energy
    double lambda_energy_;
    /// The Tau^2 correction to the SCF component of the energy
    double energy_tau_squared_;
    /// The previous total energy
    double old_total_energy_;
    /// The updated total energy
    double new_total_energy_;
    /// The Tikhonow regularizer used to remove singularities (c.f. Taube and Bartlett, JCP, 2009)
    double regularizer_;
    /// The threshold for the norm of the residual part of the subspace (|b'> = |b'> - |b><b|b'>) that is used to augment the subspace
    double vec_add_tol_;

    /// The alpha occupied eigenvectors, per irrep
    SharedMatrix aocc_c_;
    /// The beta occupied eigenvectors, per irrep
    SharedMatrix bocc_c_;
    /// The alpha virtual eigenvectors, per irrep
    SharedMatrix avir_c_;
    /// The beta virtual eigenvectors, per irrep
    SharedMatrix bvir_c_;
    /// The Tau matrix in the AO basis, stored by irrep, to perturb the alpha Fock matrix
    SharedMatrix a_tau_;
    /// The Tau matrix in the AO basis, stored by irrep, to perturb the beta Fock matrix
    SharedMatrix b_tau_;
    /// The Tau matrix in the MO basis (alpha occupied)
    SharedMatrix aocc_tau_;
    /// The Tau matrix in the MO basis (beta occupied)
    SharedMatrix bocc_tau_;
    /// The Tau matrix in the MO basis (alpha virtual)
    SharedMatrix avir_tau_;
    /// The Tau matrix in the MO basis (beta virtual)
    SharedMatrix bvir_tau_;
    /// The perturbed Tau matrix in the MO basis (alpha occupied)
    SharedMatrix aocc_ptau_;
    /// The perturbed Tau matrix in the MO basis (beta occupied)
    SharedMatrix bocc_ptau_;
    /// The perturbed Tau matrix in the MO basis (alpha virtual)
    SharedMatrix avir_ptau_;
    /// The perturbed Tau matrix in the MO basis (beta virtual)
    SharedMatrix bvir_ptau_;
    /// The Kappa in the MO basis (alpha occupied)
    SharedMatrix akappa_;
    /// The Kappa in the MO basis (beta occupied)
    SharedMatrix bkappa_;
    /// The Tau^2 correction to the alpha Tau matrix in the AO basis
    SharedMatrix a_tautau_;
    /// The Tau^2 correction to the beta Tau matrix in the AO basis
    SharedMatrix b_tautau_;
    /// The overlap matrix in the AO basis
    SharedMatrix ao_s_;
    /// The one-electron integrals in the SO basis
    SharedMatrix so_h_;
    /// The alpha Fock matrix (without Tau contribution) in the SO basis
    SharedMatrix F0a_;
    /// The beta Fock matrix (without Tau contribution) in the SO basis
    SharedMatrix F0b_;
    /// The alpha Fock matrix in the SO basis
    SharedMatrix Fa_;
    /// The beta Fock matrix in the SO basis
    SharedMatrix Fb_;
    /// The copy of the alpha Fock matrix in the SO basis before the Lowdin orthogonalization
    SharedMatrix Fa_copy;
    /// The copy of the alpha Fock matrix in the SO basis before the Lowdin orthogonalization
    SharedMatrix Fb_copy;
    /// The alpha Fock matrix in the MO basis
    SharedMatrix moFa_;
    /// The beta Fock matrix in the MO basis
    SharedMatrix moFb_;
    /// The inverse square root overlap matrix in the SO basis
    SharedMatrix s_half_inv_;
    /// The old full alpha MO coefficients
    SharedMatrix old_ca_;
    /// The old full beta MO coefficients
    SharedMatrix old_cb_;
    /// The alpha kappa matrix in the SO basis
    SharedMatrix kappa_a_;
    /// The beta kappa matrix in the SO basis
    SharedMatrix kappa_b_;
    /// The alpha external potential in the SO basis
    SharedMatrix g_tau_a_;
    /// The beta external potential in the SO basis
    SharedMatrix g_tau_b_;
    /// The alpha external potential in the MO basis (only needed in two-step algorithm)
    SharedMatrix moG_tau_a_;
    /// The beta external potential in the MO basis (only needed in two-step algorithm)
    SharedMatrix moG_tau_b_;
    /// The alpha SCF error vector
    SharedMatrix scf_error_a_;
    /// The beta SCF error vector
    SharedMatrix scf_error_b_;
    // Quadratically-convergent DCFT
    /// The orbital gradient ([f,kappa]) in the MO basis (Alpha spin)
    SharedMatrix orbital_gradient_a_;
    /// The orbital gradient ([f,kappa]) in the MO basis (Beta spin)
    SharedMatrix orbital_gradient_b_;
    /// Orbital and cumulant gradient in the basis of IDP
    SharedVector gradient_;
    /// Contribution of the Fock matrix to the diagonal part of the Hessian. Used as preconditioner for conjugate gradient procedure
    SharedVector Hd_;
    /// The step vector in the IDP basis
    SharedVector X_;
    /// Sigma vector in the basis of IDP (the product of the off-diagonal part of the Hessian with the step vector X)
    SharedVector sigma_;
    /// The conjugate direction vector in the IDP basis for conjugate gradient procedure
    SharedVector D_;
    /// The residual vector in the IDP basis for conjugate gradient procedure
    SharedVector R_;
    /// The search direction vector in the IDP basis for conjugate gradient procedure
    SharedVector S_;
    /// The new element of Krylov subspace vector in the IDP basis for conjugate gradient procedure
    SharedVector Q_;
    /// The subspace vector in the Davidson diagonalization procedure
    SharedMatrix b_;
    /// Used to align things in the output
    std::string indent;
};

}} // Namespaces

#endif // Header guard
