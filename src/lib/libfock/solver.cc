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

#include <libmints/mints.h>
#include <libqt/qt.h>
#include <psi4-dec.h>
#include "solver.h"
#include "points.h"
#include "hamiltonian.h"
#include "jk.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace boost;
using namespace std;
using namespace psi;

namespace psi {

Solver::Solver()
{
    common_init();
}
Solver::~Solver()
{
}
void Solver::common_init()
{
    print_ = 2;
    debug_ = 0;
    bench_ = 0;
    // Unlimited default
    memory_ = 0L; 
    converged_ = false;
    iteration_ = 0;
    convergence_ = 0.0;
    criteria_ = 1.0E-6;
    maxiter_ = 100;
    precondition_ = "JACOBI";
    name_ = "Solver";
}

RSolver::RSolver(boost::shared_ptr<RHamiltonian> H) :
    Solver(), H_(H)
{
    name_ = "RSolver";
}
RSolver::~RSolver()
{
}

USolver::USolver(boost::shared_ptr<UHamiltonian> H) :
    Solver(), H_(H)
{
    name_ = "USolver";
}
USolver::~USolver()
{
}

CGRSolver::CGRSolver(boost::shared_ptr<RHamiltonian> H) :
    RSolver(H)
{
    nguess_ = 0;
    name_ = "CGR";
}
CGRSolver::~CGRSolver()
{
}
boost::shared_ptr<CGRSolver> CGRSolver::build_solver(Options& options,
    boost::shared_ptr<RHamiltonian> H)
{
    boost::shared_ptr<CGRSolver> solver(new CGRSolver(H));

    if (options["PRINT"].has_changed()) {
        solver->set_print(options.get_int("PRINT") + 1);
    }
    if (options["DEBUG"].has_changed()) {
        solver->set_debug(options.get_int("DEBUG"));
    }
    if (options["BENCH"].has_changed()) {
        solver->set_bench(options.get_int("BENCH"));
    }
    if (options["SOLVER_PRECONDITION"].has_changed()) {
        solver->set_precondition(options.get_str("SOLVER_PRECONDITION"));
    } else if (options["SOLVER_MAXITER"].has_changed()) {
        solver->set_maxiter(options.get_int("SOLVER_MAXITER"));
    }
    if (options["SOLVER_CONVERGENCE"].has_changed()) {
        solver->set_convergence(options.get_double("SOLVER_CONVERGENCE"));
    }
    if (options["SOLVER_N_GUESS"].has_changed()) {
        solver->set_nguess(options.get_int("SOLVER_N_GUESS"));
    }

    return solver;
}
void CGRSolver::print_header() const
{
    if (print_) {
    	fprintf(outfile, "  ==> CGRSolver (by Rob Parrish) <==\n\n");
        fprintf(outfile, "   Number of roots    = %9zu\n", b_.size());
        fprintf(outfile, "   Preconditioning    = %9s\n", precondition_.c_str());
        fprintf(outfile, "   Convergence cutoff = %9.0E\n", criteria_);
        fprintf(outfile, "   Maximum iterations = %9d\n\n", maxiter_); 
    }
}
unsigned long int CGRSolver::memory_estimate()
{
    unsigned long int dimension = 0L;
    if (!diag_) diag_ = H_->diagonal();
    for (int h = 0; h < diag_->nirrep(); h++) {
        dimension += diag_->dimpi()[h];
    }
    return (6L * b_.size()) * dimension;
}
void CGRSolver::initialize()
{
    finalize();

    int nvec = b_.size();
    for (int N = 0; N < nvec; ++N) {
        std::stringstream xs;
        xs << "Solution Vector " << N+1;
        x_.push_back(boost::shared_ptr<Vector>(new Vector(xs.str(),b_[0]->nirrep(),b_[0]->dimpi())));
        std::stringstream Aps;
        Aps << "Product Vector " << N+1;
        Ap_.push_back(boost::shared_ptr<Vector>(new Vector(Aps.str(),b_[0]->nirrep(),b_[0]->dimpi())));
        std::stringstream zs;
        zs << "Z Vector " << N+1;
        z_.push_back(boost::shared_ptr<Vector>(new Vector(zs.str(),b_[0]->nirrep(),b_[0]->dimpi())));
        std::stringstream rs;
        rs << "Residual Vector " << N+1;
        r_.push_back(boost::shared_ptr<Vector>(new Vector(rs.str(),b_[0]->nirrep(),b_[0]->dimpi())));
        std::stringstream ps;
        ps << "Conjugate Vector " << N+1;
        p_.push_back(boost::shared_ptr<Vector>(new Vector(ps.str(),b_[0]->nirrep(),b_[0]->dimpi())));
        alpha_.push_back(0.0);
        beta_.push_back(0.0);
        r_nrm2_.push_back(0.0);
        z_r_.push_back(0.0);
        r_converged_.push_back(false);
    }

    diag_ = H_->diagonal();
}
void CGRSolver::solve()
{
    iteration_ = 0;
    converged_ = false;
    nconverged_ = 0;
    convergence_ = 0.0;

    if (print_ > 1) {
        fprintf(outfile, "  => Iterations <=\n\n");
        fprintf(outfile, "  %10s %4s %10s %10s %11s\n", "", "Iter", "Converged", "Remaining", "Residual"); 
        fflush(outfile);
    }

    setup();
    guess();
    products_x();
    residual();
    update_z();
    update_p();

    do {
        iteration_++;

        products_p();
        alpha();
        update_x();
        update_r();
        check_convergence();
        if (print_) {
            fprintf(outfile, "  %-10s %4d %10d %10zu %11.3E\n", name_.c_str(), iteration_, nconverged_, 
                b_.size() - nconverged_, convergence_);
            fflush(outfile);
        }
        update_z();
        beta();
        update_p();

    } while (iteration_ < maxiter_ && !converged_);

    if (print_ > 1) {
        fprintf(outfile, "\n");
        if (!converged_) {
            fprintf(outfile, "    %sSolver did not converge.\n\n", name_.c_str());
        } else {
            fprintf(outfile, "    %sSolver converged.\n\n", name_.c_str());
        }
        fflush(outfile);
    }
}
void CGRSolver::finalize()
{
    Ap_.clear();
    z_.clear();
    r_.clear();
    p_.clear();
    alpha_.clear();
    beta_.clear();
    r_nrm2_.clear();
    z_r_.clear();
    r_converged_.clear();
    diag_.reset();
}
void CGRSolver::setup()
{
    if (shifts_.size() == 0) {
        shifts_.resize(diag_->nirrep());
        for (int h = 0; h < diag_->nirrep(); h++) {
            shifts_[h].clear();
            for (int i = 0; i < b_.size(); i++) {
                shifts_[h].push_back(0.0);
            }
        }
    }

    if ((precondition_ == "SUBSPACE") && !A_) {

        // Find the nguess strongest diagonals in the A matrix
        Dimension rank(diag_->nirrep());
        A_inds_.clear();
        A_inds_.resize(diag_->nirrep());
        for (int h = 0; h < diag_->nirrep(); ++h) {
            int n = diag_->dimpi()[h];
            if (!n) continue;

            std::vector<std::pair<double, int> > d;
            for (int i = 0; i < n; ++i) {
                d.push_back(make_pair(diag_->get(h,i),i));
            }
            std::sort(d.begin(), d.end());

            int r = 0;
            for (int i = 0; (i < nguess_) && (i < n); ++i) {
                A_inds_[h].push_back(d[i].second);
                r++;
            } 
            rank[h] = r; 
        }

        // Preconditioner submatrix and Guess Hamiltonian
        A_ = SharedMatrix(new Matrix("A_IJ (Preconditioner)", rank, rank));
        for (int i = 0; i < nguess_; i += b_.size()) {
            x_.clear();
            Ap_.clear();
            int n = (b_.size() > (nguess_ - i) ? (nguess_ - i) : b_.size());
            for (int j = 0; j < n; j++) {
                int k = i + j;
                x_.push_back(boost::shared_ptr<Vector>(new Vector("Delta Guess", diag_->nirrep(), diag_->dimpi())));
                b_.push_back(boost::shared_ptr<Vector>(new Vector("Delta Sigma", diag_->nirrep(), diag_->dimpi())));
                for (int h = 0; h < diag_->nirrep(); h++) {
                    if (k >= A_inds_[h].size()) continue;
                    b_[j]->set(h,A_inds_[h][k],1.0);
                }
            }       

            // Low rank!
            products_x();

            for (int j = 0; j < n; j++) {
                int k = i + j;
                for (int h = 0; h < diag_->nirrep(); h++) {
                    if (k >= A_inds_[h].size()) continue;
                    double** Ap = A_->pointer(h);
                    double*  sp = Ap_[j]->pointer(h);
                    for (int l = 0; l < rank[h]; l++) {
                        Ap[k][l] = sp[A_inds_[h][l]];
                    }
                }
            }       
        }

        Ap_.clear();
        x_.clear();
    
    }
}
void CGRSolver::guess()
{
    for (int N = 0; N < b_.size(); ++N) {
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* bp = b_[N]->pointer();
            double* xp = x_[N]->pointer();
            double* dp = diag_->pointer();
            if (precondition_ == "SUBSPACE") {
                double lambda = shifts_[h][N];
                for (int m = 0; m < n; m++) {
                    xp[m] = bp[m] / (dp[m] - lambda);
                }
                
                int rank = A_inds_[h].size();
                SharedMatrix A2(new Matrix("A2", rank, rank));
                double** A2p = A2->pointer();
                double** Ap = A_->pointer(h);
                ::memcpy((void*) A2p[0], (void*) Ap[0], sizeof(double) * rank * rank);
                for (int i = 0; i < rank; i++) {
                    A2p[i][i] -= lambda;
                }

                int* ipiv = new int[rank];
                int info = C_DGETRF(rank,rank,A2p[0],rank,ipiv);
                // Only apply the improved preconditioner if nonsingular
                if (!info) {
                    double* v = new double[rank];
                    for (int i = 0; i < rank; i++) {
                        v[i] = bp[A_inds_[h][i]];
                    }
                    C_DGETRS('N',rank,1,A2p[0],rank,ipiv,v,rank);
                    for (int i = 0; i < rank; i++) {
                        xp[A_inds_[h][i]] = v[i];
                    }
                    delete[] v;
                }
                delete[] ipiv;
            } else if (precondition_ == "JACOBI") {
                double lambda = shifts_[h][N];
                for (int i = 0; i < n; ++i) {
                    xp[i] = bp[i] / (dp[i] - lambda);
                }
            } else { 
                for (int i = 0; i < n; ++i) {
                    xp[i] = bp[i];
                }
            }
        }
    }

    if (debug_) {
        diag_->print();   
        for (int N = 0; N < b_.size(); N++) {
            x_[N]->print();   
            b_[N]->print();   
        }
    }
}
void CGRSolver::residual()
{
    for (int N = 0; N < b_.size(); ++N) {
        r_[N]->copy(Ap_[N].get());
        r_[N]->scale(-1.0);
        r_[N]->add(b_[N]);
    }

    if (debug_) {
        fprintf(outfile, "  > Residuals x <\n\n");
        for (int N = 0; N < r_.size(); N++) {
            r_[N]->print();
        }
    }
}
void CGRSolver::products_x()
{
    H_->product(x_,Ap_);

    for (int h = 0; h < diag_->nirrep(); h++) {
        for (int i = 0; i < x_.size(); i++) {
            if (shifts_[h][i] != 0.0) {
                double lambda = shifts_[h][i];
                C_DAXPY(diag_->dimpi()[h],-lambda,x_[i]->pointer(h),1,Ap_[i]->pointer(h),1);
            }
        }
    }

    if (debug_) {
        fprintf(outfile, "  > Products x <\n\n");
        for (int N = 0; N < Ap_.size(); N++) {
            Ap_[N]->print();
        }
    }
}
void CGRSolver::products_p()
{
    std::vector<boost::shared_ptr<Vector> > p;
    std::vector<boost::shared_ptr<Vector> > Ap;

    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        p.push_back(p_[N]);
        Ap.push_back(Ap_[N]);
    }

    H_->product(p,Ap);

    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        for (int h = 0; h < diag_->nirrep(); h++) {
            if (shifts_[h][N] != 0.0) {
                double lambda = shifts_[h][N];
                C_DAXPY(diag_->dimpi()[h],-lambda,p_[N]->pointer(h),1,Ap_[N]->pointer(h),1);
            }
        }
    }

    if (debug_) {
        fprintf(outfile, "  > Products p <\n\n");
        for (int N = 0; N < Ap_.size(); N++) {
            Ap_[N]->print();
        }
    }
}
void CGRSolver::alpha()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        z_r_[N] = 0.0;
        double p_Ap = 0.0;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* rp = r_[N]->pointer(h);
            double* zp = z_[N]->pointer(h);
            double* pp = p_[N]->pointer(h);
            double* App = Ap_[N]->pointer(h);
            z_r_[N] += C_DDOT(n,rp,1,zp,1);
            p_Ap += C_DDOT(n,pp,1,App,1);
        }
        alpha_[N] = z_r_[N] / p_Ap;
    }

    if (debug_) {
        fprintf(outfile, "  > Alpha <\n\n");
        for (int N = 0; N < alpha_.size(); N++) {
            fprintf(outfile, "Alpha %d = %24.16E\n", N+1, alpha_[N]);
        }
    }
}
void CGRSolver::update_x()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* xp = x_[N]->pointer(h);
            double* pp = p_[N]->pointer(h);
            C_DAXPY(n,alpha_[N],pp,1,xp,1);
        }
    }

    if (debug_) {
        fprintf(outfile, "  > Update x <\n\n");
        for (int N = 0; N < x_.size(); N++) {
            x_[N]->print();
        }
    }
}
void CGRSolver::update_r()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* rp = r_[N]->pointer(h);
            double* App = Ap_[N]->pointer(h);
            C_DAXPY(n,-alpha_[N],App,1,rp,1);
        }
    }

    if (debug_) {
        fprintf(outfile, "  > Update r <\n\n");
        for (int N = 0; N < r_.size(); N++) {
            r_[N]->print();
        }
    }
}
void CGRSolver::check_convergence()
{
    convergence_ = 0.0;
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        double R2 = 0.0;
        double B2 = 0.0;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* rp = r_[N]->pointer(h);
            double* bp = b_[N]->pointer(h);
            B2 += C_DDOT(n,bp,1,bp,1);
            R2 += C_DDOT(n,rp,1,rp,1);
        }
        r_nrm2_[N] = sqrt(R2/B2);
        if (convergence_ < r_nrm2_[N]) {
            convergence_ = r_nrm2_[N];
        }
        if (r_nrm2_[N] < criteria_) {
            r_converged_[N] = true;
            nconverged_++;
        }
    }
    if (nconverged_ == b_.size()) {
        converged_ = true;
    }
}
void CGRSolver::update_z()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* zp = z_[N]->pointer();
            double* rp = r_[N]->pointer();
            double* dp = diag_->pointer();
            if (precondition_ == "SUBSPACE") {
                double lambda = shifts_[h][N];
                for (int m = 0; m < n; m++) {
                    zp[m] = rp[m] / (dp[m] - lambda);
                }
                
                int rank = A_inds_[h].size();
                SharedMatrix A2(new Matrix("A2", rank, rank));
                double** A2p = A2->pointer();
                double** Ap = A_->pointer(h);
                ::memcpy((void*) A2p[0], (void*) Ap[0], sizeof(double) * rank * rank);
                for (int i = 0; i < rank; i++) {
                    A2p[i][i] -= lambda;
                }

                int* ipiv = new int[rank];
                int info = C_DGETRF(rank,rank,A2p[0],rank,ipiv);
                // Only apply the improved preconditioner if nonsingular
                if (!info) {
                    double* v = new double[rank];
                    for (int i = 0; i < rank; i++) {
                        v[i] = rp[A_inds_[h][i]];
                    }
                    C_DGETRS('N',rank,1,A2p[0],rank,ipiv,v,rank);
                    for (int i = 0; i < rank; i++) {
                        zp[A_inds_[h][i]] = v[i];
                    }
                    delete[] v;
                }
                delete[] ipiv;
            } else if (precondition_ == "JACOBI") {
                double lambda = shifts_[h][N];
                for (int i = 0; i < n; ++i) {
                    zp[i] = rp[i] / (dp[i] - lambda);
                }
            } else { 
                for (int i = 0; i < n; ++i) {
                    zp[i] = rp[i];
                }
            }
        }
    }

    if (debug_) {
        fprintf(outfile, "  > Update z <\n\n");
        for (int N = 0; N < z_.size(); N++) {
            z_[N]->print();
        }
    }
}
void CGRSolver::beta()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        double zr = 0.0;
        for (int h = 0; h < b_[N]->nirrep(); ++h) {
            int n = b_[N]->dimpi()[h];
            if (!n) continue;
            double* rp = r_[N]->pointer();
            double* zp = z_[N]->pointer();
            zr += C_DDOT(n,rp,1,zp,1);
        }
        beta_[N] = zr / z_r_[N];
    }

    if (debug_) {
        fprintf(outfile, "  > Beta <\n\n");
        for (int N = 0; N < beta_.size(); N++) {
            fprintf(outfile, "Beta %d = %24.16E\n", N+1, beta_[N]);
        }
    }
}
void CGRSolver::update_p()
{
    for (int N = 0; N < b_.size(); ++N) {
        if (r_converged_[N]) continue;
        p_[N]->scale(beta_[N]);
        p_[N]->add(z_[N]);
    }

    if (debug_) {
        fprintf(outfile, "  > Update p <\n\n");
        for (int N = 0; N < p_.size(); N++) {
            p_[N]->print();
        }
    }
}

DLRSolver::DLRSolver(boost::shared_ptr<RHamiltonian> H) :
    RSolver(H),
    nroot_(1),
    nguess_(1),
    norm_(1.E-6),
    max_subspace_(6),
    min_subspace_(2),
    nsubspace_(0),
    nconverged_(0)
{
    name_ = "DLR";
}
DLRSolver::~DLRSolver()
{
}
boost::shared_ptr<DLRSolver> DLRSolver::build_solver(Options& options,
    boost::shared_ptr<RHamiltonian> H)
{
    boost::shared_ptr<DLRSolver> solver(new DLRSolver(H));

    if (options["PRINT"].has_changed()) {
        solver->set_print(options.get_int("PRINT") + 1);
    }
    if (options["DEBUG"].has_changed()) {
        solver->set_debug(options.get_int("DEBUG"));
    }
    if (options["BENCH"].has_changed()) {
        solver->set_bench(options.get_int("BENCH"));
    }
    if (options["SOLVER_MAXITER"].has_changed()) {
        solver->set_maxiter(options.get_int("SOLVER_MAXITER"));
    }
    if (options["SOLVER_CONVERGENCE"].has_changed()) {
        solver->set_convergence(options.get_double("SOLVER_CONVERGENCE"));
    }
    if (options["SOLVER_N_ROOT"].has_changed()) {
        solver->set_nroot(options.get_int("SOLVER_N_ROOT"));
    }
    if (options["SOLVER_N_GUESS"].has_changed()) {
        solver->set_nguess(options.get_int("SOLVER_N_GUESS"));
    }
    if (options["SOLVER_MIN_SUBSPACE"].has_changed()) {
        solver->set_min_subspace(options.get_int("SOLVER_MIN_SUBSPACE"));
    }
    if (options["SOLVER_MAX_SUBSPACE"].has_changed()) {
        solver->set_max_subspace(options.get_int("SOLVER_MAX_SUBSPACE"));
    }
    if (options["SOLVER_NORM"].has_changed()) {
        solver->set_norm(options.get_double("SOLVER_NORM"));
    }
    if (options["SOLVER_PRECONDITION"].has_changed()) {
        solver->set_precondition(options.get_str("SOLVER_PRECONDITION"));
    }

    return solver;
}
void DLRSolver::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> DLRSolver (by Rob Parrish) <== \n\n");
        fprintf(outfile, "   Number of roots         = %11d\n", nroot_);
        fprintf(outfile, "   Number of guess vectors = %11d\n", nguess_);
        fprintf(outfile, "   Maximum subspace size   = %11d\n", max_subspace_);
        fprintf(outfile, "   Minimum subspace size   = %11d\n", min_subspace_);
        fprintf(outfile, "   Subspace expansion norm = %11.0E\n", norm_);
        fprintf(outfile, "   Convergence cutoff      = %11.0E\n", criteria_);
        fprintf(outfile, "   Maximum iterations      = %11d\n", maxiter_); 
        fprintf(outfile, "   Preconditioning         = %11s\n\n", precondition_.c_str());
    }
}
unsigned long int DLRSolver::memory_estimate()
{
    unsigned long int dimension = 0L;
    if (!diag_) diag_ = H_->diagonal();
    for (int h = 0; h < diag_->nirrep(); h++) {
        dimension += diag_->dimpi()[h];
    }
    return (2L * max_subspace_ + 3L * nroot_ + 1L) * dimension;
}
void DLRSolver::initialize()
{
    finalize();

    c_.clear();
    E_.clear();

    diag_ = H_->diagonal();
} 
void DLRSolver::solve()
{
    iteration_ = 0;
    converged_ = false;
    nconverged_ = 0;
    convergence_ = 0.0;

    if (print_ > 1) {
        fprintf(outfile, "  => Iterations <=\n\n"); 
        fprintf(outfile, "  %10s %4s %10s %10s %11s\n", "", "Iter", "Converged", "Subspace", "Residual"); 
        fflush(outfile);
    }

    // Compute the first set of sigma vectors
    guess();
    sigma();
    
    do {
        iteration_++;
        
        // Compute subspace Hamiltonian
        subspaceHamiltonian();
        // Diagonalize subspace Hamiltonian
        subspaceDiagonalization();
        // Find eigenvectors 
        eigenvecs();
        // Find eigenvalues
        eigenvals();   
        // Find residuals, update convergence 
        residuals();

        if (print_) {
            fprintf(outfile, "  %-10s %4d %10d %10d %11.3E\n", name_.c_str(), iteration_, nconverged_, 
                nsubspace_, convergence_);
            fflush(outfile);       
        } 

        // Check for convergence
        if (converged_ || iteration_ >= maxiter_) break;

        // Find delta correctors 
        correctors();
        // Collapse subspace if needed
        subspaceCollapse();
        // Orthogonalize/add significant correctors
        subspaceExpansion();
        // Compute new sigma vectors
        sigma();

    } while (true);

    if (print_ > 1) {
        fprintf(outfile, "\n");
        if (!converged_ && print_ > 1) {
            fprintf(outfile, "    %sSolver did not converge.\n\n", name_.c_str());
        } else if (print_ > 1) {
            fprintf(outfile, "    %sSolver converged.\n\n", name_.c_str());
        }
        fflush(outfile);
    }
}
void DLRSolver::finalize()
{
    b_.clear();
    s_.clear();
    G_.reset();
    a_.reset();
    l_.reset();
    r_.clear();
    n_.clear();
    d_.clear();
    diag_.reset();
} 
void DLRSolver::guess()
{
    // Find the nguess strongest diagonals in the A matrix
    Dimension rank(diag_->nirrep());
    A_inds_.clear();
    A_inds_.resize(diag_->nirrep());
    for (int h = 0; h < diag_->nirrep(); ++h) {
        int n = diag_->dimpi()[h];
        if (!n) continue;

        std::vector<std::pair<double, int> > d;
        for (int i = 0; i < n; ++i) {
            d.push_back(make_pair(diag_->get(h,i),i));
        }
        std::sort(d.begin(), d.end());

        int r = 0;
        for (int i = 0; (i < nguess_) && (i < n); ++i) {
            A_inds_[h].push_back(d[i].second);
            r++;
        } 
        rank[h] = r; 
    }

    // Preconditioner submatrix and Guess Hamiltonian
    A_ = SharedMatrix(new Matrix("A_IJ (Preconditioner)", rank, rank));
    for (int i = 0; i < nguess_; i += max_subspace_) {
        b_.clear();
        s_.clear();
        int n = (max_subspace_ > (nguess_ - i) ? (nguess_ - i) : max_subspace_);
        for (int j = 0; j < n; j++) {
            int k = i + j;
            b_.push_back(boost::shared_ptr<Vector>(new Vector("Delta Guess", diag_->nirrep(), diag_->dimpi())));
            for (int h = 0; h < diag_->nirrep(); h++) {
                if (k >= A_inds_[h].size()) continue;
                b_[j]->set(h,A_inds_[h][k],1.0);
            }
        }       

        // Low rank!
        sigma();

        for (int j = 0; j < n; j++) {
            int k = i + j;
            for (int h = 0; h < diag_->nirrep(); h++) {
                if (k >= A_inds_[h].size()) continue;
                double** Ap = A_->pointer(h);
                double*  sp = s_[j]->pointer(h);
                for (int l = 0; l < rank[h]; l++) {
                    Ap[k][l] = sp[A_inds_[h][l]];
                }
            }
        }       
    }

    s_.clear();
    b_.clear();

    SharedMatrix A2(A_->clone());
    SharedMatrix U(new Matrix("U",rank,rank));
    SharedVector L(new Vector("L",rank));

    A2->diagonalize(U,L);

    for (int i = 0; i < nroot_; i++) {
        std::stringstream ss;
        ss << "Guess " << i;
        b_.push_back(boost::shared_ptr<Vector>(new Vector(ss.str(), diag_->nirrep(), diag_->dimpi())));
        for (int h = 0; h < diag_->nirrep(); h++) {
            double** Up = U->pointer(h);
            double*  bp = b_[i]->pointer(h);
            for (int j = 0; j < rank[h]; j++) {
                bp[A_inds_[h][j]] = Up[j][i];
            }
        } 
    }

    nsubspace_ = nroot_;

    if (debug_) {
        fprintf(outfile, "   > Guess <\n\n");
        diag_->print();
        A_->print();
        for (int i = 0; i < b_.size(); ++i) {
            b_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRSolver::sigma()
{
    int n = b_.size() - s_.size();
    int offset = s_.size();
    for (int i = 0; i < n; i++) {   
        std::stringstream s;
        s << "Sigma Vector " << (i + offset);
        s_.push_back(boost::shared_ptr<Vector>(new Vector(s.str(), diag_->nirrep(), diag_->dimpi())));
    }
    
    std::vector<boost::shared_ptr<Vector> > x;
    std::vector<boost::shared_ptr<Vector> > b;

    for (int i = offset; i < offset + n; i++) {
        x.push_back(b_[i]);
        b.push_back(s_[i]);
    }

    H_->product(x,b);

    if (debug_) { 
        fprintf(outfile, "   > Sigma <\n\n");
        for (int i = 0; i < s_.size(); i++) {
            s_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRSolver::subspaceHamiltonian()
{
    int n = s_.size();
    int nirrep = diag_->nirrep();
    int* npi = new int[nirrep];
    for (int h = 0; h < nirrep; ++h) {
        npi[h] = n;
    }

    G_ = SharedMatrix (new Matrix("Subspace Hamiltonian",nirrep,npi,npi));
    delete[] npi;

    for (int h = 0; h < nirrep; ++h) {

        int dimension = diag_->dimpi()[h];

        if (!dimension) continue;

        double** Gp = G_->pointer(h);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j <= i; j++) {
                Gp[i][j] = Gp[j][i] = C_DDOT(dimension,b_[i]->pointer(h),1,s_[j]->pointer(h),1);
            }
        }
    }

    if (debug_) { 
        fprintf(outfile, "   > SubspaceHamiltonian <\n\n");
        G_->print();
        fflush(outfile);
    }
}
void DLRSolver::subspaceDiagonalization()
{
    int n = s_.size();
    int nirrep = diag_->nirrep();
    int* npi = new int[nirrep];
    for (int h = 0; h < nirrep; ++h) {
        npi[h] = n;
    }

    SharedMatrix G2(G_->clone());
    a_ = SharedMatrix (new Matrix("Subspace Eigenvectors",nirrep,npi,npi));
    l_ = boost::shared_ptr<Vector> (new Vector("Subspace Eigenvalues",nirrep,npi));
    delete[] npi;
   
    G2->diagonalize(a_,l_); 

    // Resort to remove false zeros for cases with too small of irreps 
    for (int h = 0; h < nirrep; h++) {

        int dim = diag_->dimpi()[h];

        int nfalse = n - dim;

        if (nfalse <= 0) continue;

        double** ap = a_->pointer(h);    
        double*  lp = l_->pointer(h);        

        for (int m = 0; m < n - nfalse; m++) {
            lp[m] = lp[m + nfalse];
            C_DCOPY(n,&ap[0][m + nfalse], n, &ap[0][m], n);
        }

        for (int m = 0; m < nfalse; m++) {
            lp[n - m - 1] = 0;
            C_DSCAL(n,0.0,&ap[0][n - m - 1], n);
        }

    }

    if (debug_) { 
        fprintf(outfile, "   > SubspaceDiagonalize <\n\n");
        a_->print();
        l_->print();
        fflush(outfile);
    }
}
void DLRSolver::eigenvecs()
{
    if (c_.size() != nroot_) {
        c_.clear();
        for (int m = 0; m < nroot_; ++m) {
            std::stringstream s;
            s << "Eigenvector " << m;
            boost::shared_ptr<Vector> c(new Vector(s.str().c_str(),diag_->nirrep(), diag_->dimpi()));
            c_.push_back(c);
        }
    }

    for (int h = 0; h < diag_->nirrep(); ++h) {

        int dimension = diag_->dimpi()[h];
        
        if (!dimension) continue;

        double** ap = a_->pointer(h);
        for (int m = 0; m < nroot_; m++) {
            double* cp = c_[m]->pointer(h);
            ::memset((void*) cp, '\0', dimension*sizeof(double));
            for (int i = 0; i < b_.size(); i++) {
                double* bp = b_[i]->pointer(h);
                C_DAXPY(dimension,ap[i][m],bp,1,cp,1);
            }       
        } 
    }

    if (debug_) { 
        fprintf(outfile, "   > Eigenvectors <\n\n");
        for (int m = 0; m < c_.size(); m++) {
            c_[m]->print();
        }
        fflush(outfile);
    }
}
void DLRSolver::eigenvals()
{
    E_.clear();
    E_.resize(nroot_);

    for (int h = 0; h < diag_->nirrep(); ++h) {
        for (int k = 0; k < nroot_; k++) {
            E_[k].push_back(l_->get(h,k));
        }
    }

    if (debug_) { 
        fprintf(outfile, "   > Eigenvalues <\n\n");
        for (int m = 0; m < E_.size(); m++) {
            for (int h = 0; h < E_[0].size(); ++h) {
                fprintf(outfile, "    Eigenvalue %d, Irrep %d = %24.16E\n", m, h, E_[m][h]);
            }
        }
        fprintf(outfile, "\n");
        fflush(outfile);
    }
}
void DLRSolver::residuals()
{
    n_.resize(nroot_);
    nconverged_ = 0;

    if (r_.size() != nroot_) {
        r_.clear();
        for (int k = 0; k < nroot_; ++k) {
            // Residual k
            std::stringstream s;
            s << "Residual Vector " << k;
            r_.push_back(boost::shared_ptr<Vector> (new Vector(s.str().c_str(),diag_->nirrep(),diag_->dimpi())));
        }
    }

    for (int k = 0; k < nroot_; k++) {

        double R2 = 0.0;
        double S2 = 0.0;        

        for (int h = 0; h < diag_->nirrep(); ++h) {
        
        int dimension = diag_->dimpi()[h];
        if (!dimension) continue;
    
            double** ap = a_->pointer(h);
            double*  lp = l_->pointer(h);
            double*  rp = r_[k]->pointer(h);
            double*  cp = c_[k]->pointer(h);

            ::memset((void*)rp, '\0', dimension*sizeof(double));

            for (int i = 0; i < b_.size(); i++) {
                double* sp = s_[i]->pointer(h);
                C_DAXPY(dimension,ap[i][k],sp,1,rp,1);
            }       
            S2 += C_DDOT(dimension,rp,1,rp,1);

            C_DAXPY(dimension,-lp[k],cp,1,rp,1);

            R2 += C_DDOT(dimension,rp,1,rp,1);

        }

        // Residual norm k
        double rnorm = sqrt(R2/S2);
        n_[k] = rnorm;
        if (rnorm < criteria_) {
            nconverged_++;
        }
    }

    // Global convergence check
    convergence_ = 0.0;
    for (int k = 0; k < nroot_; k++) {
        if (convergence_ < n_[k]) 
            convergence_ = n_[k]; 
    }

    if (nconverged_ == nroot_) converged_ = true; 
    if (debug_) { 
        fprintf(outfile, "   > Residuals <\n\n");
        for (int i = 0; i < r_.size(); i++) {
            r_[i]->print();
        }
        for (int i = 0; i < n_.size(); i++) {
            fprintf(outfile, "    Residual %d = %24.16E\n", i, n_[i]);
        }
        fprintf(outfile,"\n");
        fprintf(outfile,"    %d of %d roots converged, we are %s\n\n",
            nconverged_, nroot_, (converged_ ? "converged": "not converged"));
        fflush(outfile);
    }
}
void DLRSolver::correctors()
{
    // Only add correctors for roots that are not converged
    d_.clear();

    for (int k = 0; k < nroot_; k++) {

        // Do not attempt to add a corrector if root is already converged
        if (n_[k] < criteria_) continue;

        std::stringstream s;
        s << "Corrector Vector " << k;
        boost::shared_ptr<Vector> d(new Vector(s.str().c_str(),diag_->nirrep(),diag_->dimpi()));

        for (int h = 0; h < diag_->nirrep(); ++h) {

            int dimension = diag_->dimpi()[h];
            if (!dimension) continue;

            double* hp = diag_->pointer(h);
            double lambda = E_[k][h];
            double* dp = d->pointer(h);
            double* rp = r_[k]->pointer(h);

            if (precondition_ == "SUBSPACE") {
                for (int m = 0; m < dimension; m++) {
                    dp[m] = rp[m] / (hp[m] - lambda);
                }
                
                // Cannot subspace precondition on the first iteration
                if (iteration_ > 1) {
                    int rank = A_inds_[h].size();
                    SharedMatrix A2(new Matrix("A2", rank, rank));
                    double** A2p = A2->pointer();
                    double** Ap = A_->pointer(h);
                    ::memcpy((void*) A2p[0], (void*) Ap[0], sizeof(double) * rank * rank);
                    for (int i = 0; i < rank; i++) {
                        A2p[i][i] -= lambda;
                    }

                    int* ipiv = new int[rank];
                    int info = C_DGETRF(rank,rank,A2p[0],rank,ipiv);
                    // Only apply the improved preconditioner if nonsingular
                    if (!info) {
                        double* v = new double[rank];
                        for (int i = 0; i < rank; i++) {
                            v[i] = rp[A_inds_[h][i]];
                        }
                        C_DGETRS('N',rank,1,A2p[0],rank,ipiv,v,rank);
                        for (int i = 0; i < rank; i++) {
                            dp[A_inds_[h][i]] = v[i];
                        }
                        delete[] v;
                    }
                    delete[] ipiv;
                }

            } else if (precondition_ == "JACOBI") {
                for (int m = 0; m < dimension; m++) {
                    dp[m] = rp[m] / (lambda - hp[m]);
                }
            } else {
                C_DCOPY(dimension,rp,1,dp,1);
            }

            // Substitute r for this vector, if norm is bad
            double norm = sqrt(C_DDOT(dimension, dp, 1, dp, 1));
            if (norm != norm || std::isinf(norm)) {
                C_DCOPY(dimension,rp,1,dp,1);
                norm = sqrt(C_DDOT(dimension, dp, 1, dp, 1));
            } 

            double scale = 1.0 / norm;
            if (scale != scale || std::isinf(scale)) {
                scale = 0.0;
            }

            // Normalize the correctors
            C_DSCAL(dimension, scale, dp, 1); 
        }

        d_.push_back(d);
    }
    if (debug_) { 
        fprintf(outfile, "   > Correctors <\n\n");
        for (int i = 0; i < d_.size(); i++) {
            d_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRSolver::subspaceExpansion()
{
    if (debug_) {
        fprintf(outfile, "   > SubspaceExpansion <\n\n");
    }

    // Which vectors are significant?
    std::vector<bool> sig(d_.size());
    for (int i = 0; i < d_.size(); ++i) {
        sig[i] = false;
    }

    // Orthonormalize d_ via Modified Gram-Schmidt
    for (int h = 0; h < diag_->nirrep(); ++h) {

        int dimension = diag_->dimpi()[h];
        if (!dimension) continue;

        // Remove the projection of d on b from b
        for (int i = 0; i < d_.size(); ++i) {
            for (int j = 0; j < b_.size(); ++j) {
                double* dp = d_[i]->pointer(h);
                double* bp = b_[j]->pointer(h);

                double r_ji = C_DDOT(dimension,dp,1,bp,1);
                C_DAXPY(dimension,-r_ji,bp,1,dp,1);
            } 
        }

        // Remove the self-projection of d on d from d
        for (int i = 0; i < d_.size(); ++i) {
            double* dip = d_[i]->pointer(h);
            double r_ii = sqrt(C_DDOT(dimension,dip,1,dip,1));
            C_DSCAL(dimension,(r_ii > norm_ ? 1.0 / r_ii : 0.0), dip,1);
            for (int j = i + 1; j < d_.size(); ++j) {
                double* djp = d_[j]->pointer(h);
                double r_ij = C_DDOT(dimension,djp,1,dip,1);
                C_DAXPY(dimension,-r_ij,dip,1,djp,1);
            }  
            if (r_ii > norm_) {
                sig[i] = sig[i] | true;    
            } 
        }
    }

    // Add significant vectors
    for (int i = 0; i < d_.size(); ++i) {
        if (sig[i]) {
            b_.push_back(d_[i]);
        }
    } 

    nsubspace_ = b_.size();

    if (debug_) {
        fprintf(outfile, "Final subspace after addition\n\n");
        for (int i = 0; i < b_.size(); i++) {
            b_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRSolver::subspaceCollapse()
{
    if (nsubspace_ <= max_subspace_) return;

    std::vector<boost::shared_ptr<Vector> > s2;
    std::vector<boost::shared_ptr<Vector> > b2;

    for (int k = 0; k < min_subspace_; ++k) {
        std::stringstream bs;
        bs << "Subspace Vector " << k; 
        b2.push_back(boost::shared_ptr<Vector>(new Vector(bs.str(), diag_->nirrep(), diag_->dimpi())));
        std::stringstream ss;
        ss << "Sigma Vector " << k; 
        s2.push_back(boost::shared_ptr<Vector>(new Vector(ss.str(), diag_->nirrep(), diag_->dimpi())));
    }

    int n = a_->rowspi()[0];
    for (int k = 0; k < min_subspace_; ++k) {
        for (int h = 0; h < diag_->nirrep(); ++h) {
            int dimension = diag_->dimpi()[h];
            if (!dimension) continue;
            
            double** ap = a_->pointer(h);
            double*  b2p = b2[k]->pointer(h);
            double*  s2p = s2[k]->pointer(h);

            for (int i = 0; i < n; ++i) {
                double*  bp = b_[i]->pointer(h);
                double*  sp = s_[i]->pointer(h);
                C_DAXPY(dimension,ap[i][k],sp,1,s2p,1);
                C_DAXPY(dimension,ap[i][k],bp,1,b2p,1);
            }
        }        
    }

    s_ = s2;
    b_ = b2;
    nsubspace_ = b_.size();

    if (debug_) {
        fprintf(outfile, "   > SubspaceCollapse <\n\n");
        for (int i = 0; i < b_.size(); i++) {
            b_[i]->print();
        }
        for (int i = 0; i < s_.size(); i++) {
            s_[i]->print();
        }
    }
}

RayleighRSolver::RayleighRSolver(boost::shared_ptr<RHamiltonian> H) : 
    DLRSolver(H)
{
    name_ = "RayleighR"; 
    precondition_maxiter_ = 1;
    precondition_steps_ = "TRIANGULAR";
    quantity_ = "RESIDUAL";
}
RayleighRSolver::~RayleighRSolver()
{
}
boost::shared_ptr<RayleighRSolver> RayleighRSolver::build_solver(Options& options,
    boost::shared_ptr<RHamiltonian> H)
{
    boost::shared_ptr<RayleighRSolver> solver(new RayleighRSolver(H));

    if (options["PRINT"].has_changed()) {
        solver->set_print(options.get_int("PRINT") + 1);
    }
    if (options["DEBUG"].has_changed()) {
        solver->set_debug(options.get_int("DEBUG"));
    }
    if (options["BENCH"].has_changed()) {
        solver->set_bench(options.get_int("BENCH"));
    }
    if (options["SOLVER_MAXITER"].has_changed()) {
        solver->set_maxiter(options.get_int("SOLVER_MAXITER"));
    }
    if (options["SOLVER_CONVERGENCE"].has_changed()) {
        solver->set_convergence(options.get_double("SOLVER_CONVERGENCE"));
    }
    if (options["SOLVER_N_ROOT"].has_changed()) {
        solver->set_nroot(options.get_int("SOLVER_N_ROOT"));
    }
    if (options["SOLVER_N_GUESS"].has_changed()) {
        solver->set_nguess(options.get_int("SOLVER_N_GUESS"));
    }
    if (options["SOLVER_MIN_SUBSPACE"].has_changed()) {
        solver->set_min_subspace(options.get_int("SOLVER_MIN_SUBSPACE"));
    }
    if (options["SOLVER_MAX_SUBSPACE"].has_changed()) {
        solver->set_max_subspace(options.get_int("SOLVER_MAX_SUBSPACE"));
    }
    if (options["SOLVER_NORM"].has_changed()) {
        solver->set_norm(options.get_double("SOLVER_NORM"));
    }
    if (options["SOLVER_PRECONDITION"].has_changed()) {
        solver->set_precondition(options.get_str("SOLVER_PRECONDITION"));
    }
    if (options["SOLVER_PRECONDITION_STEPS"].has_changed()) {
        solver->set_precondition_steps(options.get_str("SOLVER_PRECONDITION_STEPS"));
    }
    if (options["SOLVER_PRECONDITION_MAXITER"].has_changed()) {
        solver->set_precondition_maxiter(options.get_int("SOLVER_PRECONDITION_MAXITER"));
    }
    if (options["SOLVER_QUANTITY"].has_changed()) {
        solver->set_quantity(options.get_str("SOLVER_QUANTITY"));
    }

    return solver;
}
void RayleighRSolver::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> RayleighRSolver (by Rob Parrish) <== \n\n");
        fprintf(outfile, "   Number of roots         = %11d\n", nroot_);
        fprintf(outfile, "   Number of guess vectors = %11d\n", nguess_);
        fprintf(outfile, "   Expansion quantity      = %11s\n", quantity_.c_str());
        if (quantity_ == "RESIDUAL") {
            fprintf(outfile, "   Maximum subspace size   = %11d\n", max_subspace_);
            fprintf(outfile, "   Minimum subspace size   = %11d\n", min_subspace_);
        }
        fprintf(outfile, "   Convergence cutoff      = %11.0E\n", criteria_);
        fprintf(outfile, "   Maximum iterations      = %11d\n", maxiter_); 
        fprintf(outfile, "   Rayleigh step type      = %11s\n", precondition_steps_.c_str()); 
        if (precondition_steps_ == "CONSTANT") {
            fprintf(outfile, "   Rayleigh step maxiter   = %11d\n", precondition_maxiter_); 
        } else {
            fprintf(outfile, "   Rayleigh step factor    = %11d\n", precondition_maxiter_); 
        }
        fprintf(outfile, "   Preconditioning         = %11s\n\n", precondition_.c_str());
    }
}
void RayleighRSolver::initialize()
{
    DLRSolver::initialize();
    cg_ = CGRSolver::build_solver(Process::environment.options, H_);
    cg_->set_print(1);
} 
void RayleighRSolver::finalize()
{
    DLRSolver::finalize();
    cg_.reset();
}
void RayleighRSolver::correctors()
{
    cg_->set_A(A_,A_inds_);
    if (precondition_ == "SUBSPACE") {
        if (iteration_ <= 1) {
            cg_->set_precondition("JACOBI");
        } else {
            cg_->set_precondition("SUBSPACE");
        }
    }
    if (precondition_steps_ == "CONSTANT")
        cg_->set_maxiter(precondition_maxiter_);
    else 
        cg_->set_maxiter(iteration_ * precondition_maxiter_);    

    std::vector<SharedVector>& b = cg_->b();
    std::vector<SharedVector>& x = cg_->x();

    b.clear();
    x.clear();
    d_.clear();

    std::vector<SharedVector> force;
    if (quantity_ == "EIGENVECTOR") {
        force = c_;
    } else {
        force = r_;
    }

    std::vector<int> sig_inds;
    std::vector<std::vector<double> > shifts(diag_->nirrep());
    for (int i = 0; i < nroot_; i++) {
        if (n_[i] > criteria_) {
            for (int h = 0; h < diag_->nirrep(); h++) {
                shifts[h].push_back(E_[i][h]);
            }
            b.push_back(force[i]);
            sig_inds.push_back(i);
        } else {
            d_.push_back(force[i]);
        }
    }        
    cg_->set_shifts(shifts);
    cg_->initialize();
    
    cg_->solve();

    for (int i = 0; i < b.size(); i++) {
        d_.push_back(x[i]);
        for (int h = 0; h < diag_->nirrep(); h++) { 

            int dimension = diag_->dimpi()[h];
            if (!dimension) continue;

            double* dp = d_[i]->pointer(h);
            //if (quantity_ == "EIGENVECTOR") {
                int i_abs = sig_inds[i];
                double* cp = c_[i]->pointer(h);
                double S = C_DDOT(dimension, dp, 1, cp, 1);
                C_DAXPY(dimension,-S,cp,1,dp,1);
            //}
            
            double norm = sqrt(C_DDOT(dimension, dp, 1, dp, 1));
            double scale = 1.0 / norm;
            if (scale != scale || std::isinf(scale)) {
                scale = 0.0;
            }

            // Normalize the corrector
            C_DSCAL(dimension, scale, dp, 1); 
        }
    }

    cg_->finalize();
}

DLRXSolver::DLRXSolver(boost::shared_ptr<RHamiltonian> H) :
    RSolver(H),
    nroot_(1),
    nguess_(1),
    norm_(1.E-6),
    max_subspace_(6),
    min_subspace_(2),
    nsubspace_(0),
    nconverged_(0)
{
}
DLRXSolver::~DLRXSolver()
{
}
boost::shared_ptr<DLRXSolver> DLRXSolver::build_solver(Options& options,
    boost::shared_ptr<RHamiltonian> H)
{
    boost::shared_ptr<DLRXSolver> solver(new DLRXSolver(H));

    if (options["PRINT"].has_changed()) {
        solver->set_print(options.get_int("PRINT") + 1);
    }
    if (options["DEBUG"].has_changed()) {
        solver->set_debug(options.get_int("DEBUG"));
    }
    if (options["SOLVER_MAXITER"].has_changed()) {
        solver->set_maxiter(options.get_int("SOLVER_MAXITER"));
    }
    if (options["SOLVER_CONVERGENCE"].has_changed()) {
        solver->set_convergence(options.get_double("SOLVER_CONVERGENCE"));
    }
    if (options["SOLVER_N_ROOT"].has_changed()) {
        solver->set_nroot(options.get_int("SOLVER_N_ROOT"));
    }
    if (options["SOLVER_N_GUESS"].has_changed()) {
        solver->set_nguess(options.get_int("SOLVER_N_GUESS"));
    }
    if (options["SOLVER_MIN_SUBSPACE"].has_changed()) {
        solver->set_min_subspace(options.get_int("SOLVER_MIN_SUBSPACE"));
    }
    if (options["SOLVER_MAX_SUBSPACE"].has_changed()) {
        solver->set_max_subspace(options.get_int("SOLVER_MAX_SUBSPACE"));
    }
    if (options["SOLVER_NORM"].has_changed()) {
        solver->set_norm(options.get_double("SOLVER_NORM"));
    }

    return solver;
}
void DLRXSolver::print_header() const 
{
    if (print_) {
        fprintf(outfile, "  ==> DLRXSolver (by Rob Parrish) <== \n\n");
        fprintf(outfile, "   Number of roots         = %11d\n", nroot_);
        fprintf(outfile, "   Number of guess vectors = %11d\n", nguess_);
        fprintf(outfile, "   Maximum subspace size   = %11d\n", max_subspace_);
        fprintf(outfile, "   Minimum subspace size   = %11d\n", min_subspace_);
        fprintf(outfile, "   Subspace expansion norm = %11.0E\n", norm_);
        fprintf(outfile, "   Convergence cutoff      = %11.0E\n", criteria_);
        fprintf(outfile, "   Maximum iterations      = %11d\n\n", maxiter_); 
    }
}
unsigned long int DLRXSolver::memory_estimate()
{
    unsigned long int dimension = 0L;
    if (!diag_) diag_ = H_->diagonal();
    for (int h = 0; h < diag_->nirrep(); h++) {
        dimension += diag_->dimpi()[h];
    }
    return (2L * max_subspace_ + 3L * nroot_ + 1L) * dimension;
}
void DLRXSolver::initialize()
{
    finalize();

    c_.clear();
    E_.clear();

    diag_ = H_->diagonal();
} 
void DLRXSolver::solve()
{
    iteration_ = 0;
    converged_ = false;
    nconverged_ = 0;
    convergence_ = 0.0;

    if (print_) {
        fprintf(outfile, "  => Iterations <=\n\n"); 
        fprintf(outfile, "   %4s  %10s  %10s  %11s\n", "Iter", "NConverged", "NSubspace", "Residual"); 
        fflush(outfile);
    }

    // Compute the first set of sigma vectors
    guess();
    sigma();
    
    do {
        iteration_++;
        
        // Compute subspace Hamiltonian
        subspaceHamiltonian();
        // Diagonalize subspace Hamiltonian
        subspaceDiagonalization();
        // Find eigenvectors 
        eigenvecs();
        // Find eigenvalues
        eigenvals();   
        // Find residuals, update convergence 
        residuals();

        fprintf(outfile, "   %4d  %10d  %10d  %11.3E\n", iteration_, nconverged_, 
            nsubspace_, convergence_);
        fflush(outfile);       
 
        // Check for convergence
        if (converged_) break;

        // Find correctors 
        correctors();
        // Collapse subspace if needed
        subspaceCollapse();
        // Orthogonalize/add significant correctors 
        subspaceExpansion();
        // Compute new sigma vectors 
        sigma();

    } while (iteration_ < maxiter_ && !converged_);

    if (print_) {
        fprintf(outfile, "\n");
    }

    if (!converged_) {
        throw PSIEXCEPTION("DLRXSolver did not converge");
    } else if (print_) {
        fprintf(outfile, "    DLRXSolver converged.\n\n");
    }

    fflush(outfile);
}
void DLRXSolver::finalize()
{
    b_.clear();
    s_.clear();
    G_.reset();
    a_.reset();
    l_.reset();
    r_.clear();
    n_.clear();
    d_.clear();
    diag_.reset();
} 
void DLRXSolver::guess()
{
    for (int i = 0; i < nguess_; ++i) {
        std::stringstream ss;
        ss << "Subspace Vector " << i;
        b_.push_back(boost::shared_ptr<Vector>(new Vector(ss.str(), diag_->nirrep(), diag_->dimpi())));
    } 

    for (int h = 0; h < diag_->nirrep(); ++h) {
        int n = diag_->dimpi()[h] / 2;
        if (!n) continue;

        std::vector<std::pair<double, int> > d;
        for (int i = 0; i < n; ++i) {
            d.push_back(make_pair(diag_->get(h,i),i));
        }
        std::sort(d.begin(), d.end());

        for (int i = 0; (i < nguess_) && (i < n); ++i) {
            b_[i]->set(h,d[i].second,1.0);    
        } 
    }

    nsubspace_ = nguess_;

    if (debug_) {
        fprintf(outfile, "   > Guess <\n\n");
        diag_->print();
        for (int i = 0; i < b_.size(); ++i) {
            b_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRXSolver::sigma()
{
    int n = b_.size() - s_.size();
    int offset = s_.size();
    for (int i = 0; i < n; i++) {   
        std::stringstream s;
        s << "Sigma Vector " << (i + offset);
        s_.push_back(boost::shared_ptr<Vector>(new Vector(s.str(), diag_->nirrep(), diag_->dimpi())));
    }
    
    std::vector<boost::shared_ptr<Vector> > x;
    std::vector<boost::shared_ptr<Vector> > b;

    for (int i = offset; i < offset + n; i++) {
        x.push_back(b_[i]);
        b.push_back(s_[i]);
    }

    H_->product(x,b);

    if (debug_) { 
        fprintf(outfile, "   > Sigma <\n\n");
        for (int i = 0; i < s_.size(); i++) {
            s_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRXSolver::subspaceHamiltonian()
{
    int n = s_.size();
    int nirrep = diag_->nirrep();
    int* npi = new int[nirrep];
    for (int h = 0; h < nirrep; ++h) {
        npi[h] = 2*n;
    }

    G_ = SharedMatrix (new Matrix("Subspace Hamiltonian",nirrep,npi,npi));
    delete[] npi;

    for (int h = 0; h < nirrep; ++h) {

        int dimension = diag_->dimpi()[h] / 2;

        if (!dimension) continue;

        double** Gp = G_->pointer(h);

        // (b_+)' H (b_+) and (b_-)' H (b_-)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                Gp[i][j] = C_DDOT(2*dimension,b_[i]->pointer(h),1,s_[j]->pointer(h),1);
                Gp[i + n][j + n] = -Gp[i][j];
            }
        }

        // (b_+)' H (b_-) and (b_-)'
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                Gp[i][j + n] = -C_DDOT(dimension,&b_[i]->pointer(h)[0],1,&s_[j]->pointer(h)[dimension],1)
                               -C_DDOT(dimension,&b_[i]->pointer(h)[dimension],1,&s_[j]->pointer(h)[0],1);

                Gp[i + n][j] = -Gp[i][j + n];
            }
        }
    }

    if (debug_) { 
        fprintf(outfile, "   > SubspaceHamiltonian <\n\n");
        G_->print();
        fflush(outfile);
    }
}
void DLRXSolver::subspaceDiagonalization()
{
    int n = s_.size();
    int nirrep = diag_->nirrep();
    int* npi = new int[nirrep];
    for (int h = 0; h < nirrep; ++h) {
        npi[h] = 2*n;
    }

    // Reals
    a_ = SharedMatrix (new Matrix("Subspace Right Eigenvectors",nirrep,npi,npi));
    l_ = boost::shared_ptr<Vector> (new Vector("Real Subspace Eigenvalues",nirrep,npi));
    delete[] npi;

    // Temps
    SharedMatrix G2(G_->clone());
    SharedMatrix atemp(new Matrix("Right Eigenvectors Temp", 2*n, 2*n)); 
    SharedVector lrtemp(new Vector("Real Eigenvalue Temp", 2*n)); 
    SharedVector litemp(new Vector("Imaginary Eigenvalue Temp", 2*n)); 

    // Diagonalize
    for (int h = 0; h < nirrep; h++) {
        
        // Temp pointers
        double** gp = G2->pointer(h);        
        double** ap = atemp->pointer();        
        double* lrp = lrtemp->pointer();        
        double* lip = litemp->pointer();        

        // Real pointers
        double** evecp = a_->pointer(h);
        double*  evalp = l_->pointer(h);

        // Workspace (never throws)
        int info;
        double dwork;
        info = C_DGEEV('V','N', 2*n, gp[0], 2*n, lrp, lip, ap[0], 2*n, NULL, 1, &dwork, -1);
        int lwork = (int) dwork;
        double* work = new double[lwork];
        info = C_DGEEV('V','N', 2*n, gp[0], 2*n, lrp, lip, ap[0], 2*n, NULL, 1, work, lwork);
        delete[] work;

        if (info != 0) {
            throw PSIEXCEPTION("DLXSolver: Subspace DGEEV failed");
        }
        
        // Check for imaginary eigenvalues
        for (int i = 0; i < 2*n; i++) {
            if (lip[i] != 0.0) {
                std::cout << lip[i];
                throw PSIEXCEPTION("DLXSolver: Imaginary eigenvalue found. Result is physically meaningless.");
            }
        }

        // Sort to order as -/+, -/+, ....
        std::vector<std::pair<double, int> > pass1;
        for (int i = 0; i < 2*n; i++) {
            pass1.push_back(make_pair(fabs(lrp[i]), i));
        }

        std::sort(pass1.begin(), pass1.end());
        
        // Maybe we need a more advanced algorithm to resolve degeneracies?
        // Methinks we should explicitly lock the - subspace coefs based on the + subspace coefs
        std::vector<int> pass2;
        for (int i = 0; i < n; i++) {
            if (lrp[pass1[2*i].second] < lrp[pass1[2*i+1].second]) {
                pass2.push_back(pass1[2*i].second);
                pass2.push_back(pass1[2*i+1].second);
            }  else {  
                pass2.push_back(pass1[2*i+1].second);
                pass2.push_back(pass1[2*i].second);
            }    
        }

        for (int i = 0; i < 2*n; i++) {
            int index = pass2[i]; 
            evalp[i] = lrp[index];
            C_DCOPY(2*n, ap[index], 1, &evecp[0][i], 2*n);
        }
    }
   
    //// Resort to remove false zeros for cases with too small of irreps 
    for (int h = 0; h < nirrep; h++) {

        int dim = diag_->dimpi()[h] / 2;

        int nfalse = n - dim;

        if (nfalse <= 0) continue;

        double** ap = a_->pointer(h);    
        double*  lp = l_->pointer(h);        

        for (int m = 0; m < 2*(n - nfalse); m++) {
            lp[m] = lp[m + 2*nfalse];
            C_DCOPY(n,&ap[0][m + 2*nfalse], n, &ap[0][m], n);
        }

        for (int m = 0; m < 2*nfalse; m++) {
            lp[n - m - 1] = 0;
            C_DSCAL(n,0.0,&ap[0][n - m - 1], n);
        }
    }

    if (debug_) { 
        fprintf(outfile, "   > SubspaceDiagonalize <\n\n");
        a_->print();
        l_->print();
        fflush(outfile);
    }
}
void DLRXSolver::eigenvecs()
{
    if (c_.size() != nroot_) {
        c_.clear();
        for (int m = 0; m < nroot_; ++m) {
            std::stringstream s;
            s << "Eigenvector " << m;
            boost::shared_ptr<Vector> c(new Vector(s.str().c_str(),diag_->nirrep(), diag_->dimpi()));
            c_.push_back(c);
        }
    }

    for (int h = 0; h < diag_->nirrep(); ++h) {

        int dimension = diag_->dimpi()[h] / 2;
        
        if (!dimension) continue;

        double** ap = a_->pointer(h);
        for (int m = 0; m < nroot_; m++) {
            double* cp = c_[m]->pointer(h);
            ::memset((void*) cp, '\0', 2L*dimension*sizeof(double));

            for (int i = 0; i < b_.size(); i++) {
                double* bp = b_[i]->pointer(h);
                C_DAXPY(dimension,ap[i][2*m+1],&bp[0],1,&cp[0],1);
                C_DAXPY(dimension,ap[i][2*m+1],&bp[dimension],1,&cp[dimension],1);
                C_DAXPY(dimension,ap[i+b_.size()][2*m+1],&bp[dimension],1,&cp[0],1);
                C_DAXPY(dimension,ap[i+b_.size()][2*m+1],&bp[0],1,&cp[dimension],1);
            }       

        } 
    }

    if (debug_) { 
        fprintf(outfile, "   > Eigenvectors <\n\n");
        for (int m = 0; m < c_.size(); m++) {
            c_[m]->print();
        }
        fflush(outfile);
    }
}
void DLRXSolver::eigenvals()
{
    E_.clear();
    E_.resize(nroot_);

    for (int h = 0; h < diag_->nirrep(); ++h) {
        for (int k = 0; k < nroot_; k++) {
            E_[k].push_back(l_->get(h,2*k+1));
        }
    }

    if (debug_) { 
        fprintf(outfile, "   > Eigenvalues <\n\n");
        for (int m = 0; m < E_.size(); m++) {
            for (int h = 0; h < E_[0].size(); ++h) {
                fprintf(outfile, "    Eigenvalue %d, Irrep %d = %24.16E\n", m, h, E_[m][h]);
            }
        }
        fprintf(outfile, "\n");
        fflush(outfile);
    }
}
void DLRXSolver::residuals()
{
    n_.resize(nroot_);
    nconverged_ = 0;

    if (r_.size() != nroot_) {
        r_.clear();
        for (int k = 0; k < nroot_; ++k) {
            // Residual k
            std::stringstream s;
            s << "Residual Vector " << k;
            r_.push_back(boost::shared_ptr<Vector> (new Vector(s.str().c_str(),diag_->nirrep(),diag_->dimpi())));
        }
    }

    for (int k = 0; k < nroot_; k++) {

        double R2 = 0.0;

        for (int h = 0; h < diag_->nirrep(); ++h) {
        
        int dimension = diag_->dimpi()[h]/2;
        if (!dimension) continue;
    
            double** ap = a_->pointer(h);
            double*  lp = l_->pointer(h);
            double*  rp = r_[k]->pointer(h);
            double*  cp = c_[k]->pointer(h);

            ::memset((void*)rp, '\0', 2L*dimension*sizeof(double));

            for (int i = 0; i < b_.size(); i++) {
                double* sp = s_[i]->pointer(h);
                C_DAXPY(dimension,ap[i][2*k+1],&sp[0],1,&rp[0],1);
                C_DAXPY(dimension,ap[i][2*k+1],&sp[dimension],1,&rp[dimension],1);
                C_DAXPY(dimension,-ap[i+b_.size()][2*k+1],&sp[dimension],1,&rp[0],1);
                C_DAXPY(dimension,-ap[i+b_.size()][2*k+1],&sp[0],1,&rp[dimension],1);
            }       

            C_DAXPY(dimension*2L,-lp[2*k+1],cp,1,rp,1);

            R2 += C_DDOT(dimension*2L,rp,1,rp,1);

        }

        // Residual norm k
        double rnorm = sqrt(R2);
        n_[k] = rnorm;
        if (rnorm < criteria_) {
            nconverged_++;
        }
    }

    // Global convergence check
    convergence_ = 0.0;
    for (int k = 0; k < nroot_; k++) {
        if (convergence_ < n_[k]) 
            convergence_ = n_[k]; 
    }

    if (nconverged_ == nroot_) converged_ = true; 
    if (debug_) { 
        fprintf(outfile, "   > Residuals <\n\n");
        for (int i = 0; i < r_.size(); i++) {
            r_[i]->print();
        }
        for (int i = 0; i < n_.size(); i++) {
            fprintf(outfile, "    Residual %d = %24.16E\n", i, n_[i]);
        }
        fprintf(outfile,"\n");
        fprintf(outfile,"    %d of %d roots converged, we are %s\n\n",
            nconverged_, nroot_, (converged_ ? "converged": "not converged"));
        fflush(outfile);
    }
}
void DLRXSolver::correctors()
{
    // Only add correctors for roots that are not converged
    d_.clear();

    for (int k = 0; k < nroot_; k++) {

        // Do not attempt to add a corrector if root is already converged
        if (n_[k] < criteria_) continue;

        std::stringstream s;
        s << "Corrector Vector " << k;
        boost::shared_ptr<Vector> d(new Vector(s.str().c_str(),diag_->nirrep(),diag_->dimpi()));

        for (int h = 0; h < diag_->nirrep(); ++h) {

            int dimension = diag_->dimpi()[h];
            if (!dimension) continue;

            double* hp = diag_->pointer(h);
            double lambda = E_[k][h];
            double* dp = d->pointer(h);
            double* rp = r_[k]->pointer(h);

            for (int m = 0; m < dimension/2; m++) {
                dp[m] = rp[m] / (lambda - hp[m]);
            }
            for (int m = dimension/2+1; m < dimension; m++) {
                dp[m] = rp[m] / (lambda + hp[m]);
            }

            // Substitute r for this vector, if norm is bad
            double norm = sqrt(C_DDOT(dimension, dp, 1, dp, 1));
            if (norm != norm || std::isinf(norm) || norm < criteria_) {
                C_DCOPY(dimension,rp,1,dp,1);
                norm = sqrt(C_DDOT(dimension, dp, 1, dp, 1));
            } 

            double scale = 1.0 / norm;
            if (scale != scale || std::isinf(scale)) {
                scale = 0.0;
            }

            // Normalize the correctors
            C_DSCAL(dimension, scale, dp, 1); 
        }

        d_.push_back(d);
    }
    if (debug_) { 
        fprintf(outfile, "   > Correctors <\n\n");
        for (int i = 0; i < d_.size(); i++) {
            d_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRXSolver::subspaceExpansion()
{
    if (debug_) {
        fprintf(outfile, "   > SubspaceExpansion <\n\n");
    }

    // Which vectors are significant?
    std::vector<bool> sig(d_.size());
    for (int i = 0; i < d_.size(); ++i) {
        sig[i] = false;
    }

    // Orthonormalize d_ via Modified Gram-Schmidt
    for (int h = 0; h < diag_->nirrep(); ++h) {

        int dimension = diag_->dimpi()[h]/2;
        if (!dimension) continue;

        // Remove the projection of d+ on b+ from d+
        for (int i = 0; i < d_.size(); ++i) {
            for (int j = 0; j < b_.size(); ++j) {
                double* dp = d_[i]->pointer(h);
                double* bp = b_[j]->pointer(h);
                
                double r_ji = C_DDOT(2*dimension,dp,1,bp,1);
                C_DAXPY(2*dimension,-r_ji,bp,1,dp,1);
            } 
        }
        // Remove the projection of d+ on b- from d+
        for (int i = 0; i < d_.size(); ++i) {
            for (int j = 0; j < b_.size(); ++j) {
                double* dp = d_[i]->pointer(h);
                double* bp = b_[j]->pointer(h);
                
                double r_ji = C_DDOT(dimension,dp,1,&bp[dimension],1)+C_DDOT(dimension,&dp[dimension],1,bp,1);
                C_DAXPY(dimension,-r_ji,&bp[dimension],1,dp,1);
                C_DAXPY(dimension,-r_ji,bp,1,&dp[dimension],1);
            } 
        }

        // Remove low-norm vectors
        std::vector<int> sigfigs;
        for (int i = 0; i < d_.size(); ++i) {
            double* dip = d_[i]->pointer(h);
            double r_ii = sqrt(C_DDOT(2L*dimension,dip,1,dip,1));
            C_DSCAL(dimension,(r_ii > norm_ ? 1.0 / r_ii : 0.0), dip,1);
            if (r_ii > norm_) {
                sig[i] = sig[i] | true;    
                sigfigs.push_back(i);
            } 
        }

        int neff = sigfigs.size();
        SharedMatrix S(new Matrix("Overlap", 2*neff,2*neff));

        // TODO build S

        S->power(-1.0/2.0,0.0);

        std::vector<SharedVector> dtemp;
        for (int i = 0; i < neff; i++) {
            dtemp.push_back(SharedVector(new Vector("d temp", 2L*dimension)));
        }

        // TODO build contributions to d2
        for (int i = 0; i < neff; i++) {
                
        }
        
        for (int i = 0; i < neff; i++) {
            ::memcpy((void*) d_[sigfigs[i]]->pointer(h), (void*) dtemp[i]->pointer(), sizeof(double)*2L*dimension); 
        }

    }

    // Add significant vectors
    for (int i = 0; i < d_.size(); ++i) {
        if (sig[i]) {
            b_.push_back(d_[i]);
        }
    } 

    nsubspace_ = b_.size();

    if (debug_) {
        fprintf(outfile, "Final subspace after addition\n\n");
        for (int i = 0; i < b_.size(); i++) {
            b_[i]->print();
        }
        fflush(outfile);
    }
}
void DLRXSolver::subspaceCollapse()
{
    if (nsubspace_ <= max_subspace_) return;

    std::vector<boost::shared_ptr<Vector> > s2;
    std::vector<boost::shared_ptr<Vector> > b2;

    for (int k = 0; k < min_subspace_; ++k) {
        std::stringstream bs;
        bs << "Subspace Vector " << k; 
        b2.push_back(boost::shared_ptr<Vector>(new Vector(bs.str(), diag_->nirrep(), diag_->dimpi())));
        std::stringstream ss;
        ss << "Sigma Vector " << k; 
        s2.push_back(boost::shared_ptr<Vector>(new Vector(ss.str(), diag_->nirrep(), diag_->dimpi())));
    }

    int n = a_->rowspi()[0]/2;
    for (int k = 0; k < min_subspace_; ++k) {
        for (int h = 0; h < diag_->nirrep(); ++h) {
            int dimension = diag_->dimpi()[h]/2;
            if (!dimension) continue;
            
            double** ap = a_->pointer(h);
            double*  b2p = b2[k]->pointer(h);
            double*  s2p = s2[k]->pointer(h);

            for (int i = 0; i < n; ++i) {
                double*  bp = b_[i]->pointer(h);
                double*  sp = s_[i]->pointer(h);
                
                // This is cumulative. Han Solo would say "She'll hold together"
                C_DAXPY(dimension,ap[i][2*k+1],&sp[0],1,&s2p[0],1);
                C_DAXPY(dimension,ap[i][2*k+1],&sp[dimension],1,&s2p[dimension],1);
                C_DAXPY(dimension,-ap[i+n][2*k+1],&sp[dimension],1,&s2p[0],1);
                C_DAXPY(dimension,-ap[i+n][2*k+1],&sp[0],1,&s2p[dimension],1);

                C_DAXPY(dimension,ap[i][2*k+1],&bp[0],1,&b2p[0],1);
                C_DAXPY(dimension,ap[i][2*k+1],&bp[dimension],1,&b2p[dimension],1);
                C_DAXPY(dimension,ap[i+n][2*k+1],&bp[dimension],1,&b2p[0],1);
                C_DAXPY(dimension,ap[i+n][2*k+1],&bp[0],1,&b2p[dimension],1);
            }
        }        
    }

    s_ = s2;
    b_ = b2;
    nsubspace_ = b_.size();

    if (debug_) {
        fprintf(outfile, "   > SubspaceCollapse <\n\n");
        for (int i = 0; i < b_.size(); i++) {
            b_[i]->print();
        }
        for (int i = 0; i < s_.size(); i++) {
            s_[i]->print();
        }
    }
}

}
