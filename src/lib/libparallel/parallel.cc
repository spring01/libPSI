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

#include <psi4-dec.h>
#include <libparallel/parallel.h>
#include <libparallel/openmp.h>

namespace psi {

boost::shared_ptr<worldcomm> WorldComm;

boost::shared_ptr<worldcomm> initialize_communicator(const int &argc, char **argv) {
    return boost::shared_ptr<worldcomm>(initialize_specific_communicator<worldcomm>(argc, argv));
}

void init_openmp() {
#ifdef _OPENMP
    // Disable nested threads in OpenMP
    omp_set_nested(0);
#endif

    // If no OMP thread variable is set, set nthreads to default to 1
    if (Process::environment("OMP_NUM_THREADS") == "")
        Process::environment.set_n_threads(1);
}

}
