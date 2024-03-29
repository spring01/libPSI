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

#include "libmatrix.h"

#include <libplugin/plugin.h>
#include <psi4-dec.h>
#include <libparallel/parallel.h>
#include <liboptions/liboptions.h>
#include <libmints/mints.h>
#include <libdist_matrix/dist_mat.h>
#include <libpsio/psio.hpp>

INIT_PLUGIN

namespace psi { namespace libmatrix {

extern "C"
int read_options(std::string name, Options& options)
{
    if (name == "TEMPLATE_PLUGIN"|| options.read_globals()) {
    }

    return true;
}

extern "C"
PsiReturnType template_plugin(Options& options)
{
    boost::shared_ptr<Molecule> molecule = Process::environment.molecule();

    // Form basis object:
    // Create a basis set parser object.
    boost::shared_ptr<BasisSetParser> parser(new Gaussian94BasisSetParser());
    // Construct a new basis set.
    boost::shared_ptr<BasisSet> aoBasis = BasisSet::construct(parser, molecule, "BASIS");

    // The integral factory oversees the creation of integral objects
    boost::shared_ptr<IntegralFactory> integral(new IntegralFactory(aoBasis, aoBasis, aoBasis, aoBasis));

    // N.B. This should be called after the basis has been built, because the geometry has not been
    // fully initialized until this time.
    molecule->print();
    int nbf[] = { aoBasis->nbf() };
    double nucrep = molecule->nuclear_repulsion_energy();
    fprintf(outfile, "\n    Nuclear repulsion energy: %16.8f\n\n", nucrep);

    // The matrix factory can create matrices of the correct dimensions...
    boost::shared_ptr<MatrixFactory> factory(new MatrixFactory);
    factory->init_with(1, nbf, nbf);

    // Form the one-electron integral objects from the integral factory
    boost::shared_ptr<OneBodyAOInt> sOBI(integral->ao_overlap());
    boost::shared_ptr<OneBodyAOInt> tOBI(integral->ao_kinetic());
    boost::shared_ptr<OneBodyAOInt> vOBI(integral->ao_potential());
    // Form the one-electron integral matrices from the matrix factory
    boost::shared_ptr<Matrix> sMat(factory->create_matrix("Overlap"));
    boost::shared_ptr<Matrix> tMat(factory->create_matrix("Kinetic"));
    boost::shared_ptr<Matrix> vMat(factory->create_matrix("Potential"));
    boost::shared_ptr<Matrix> hMat(factory->create_matrix("One Electron Ints"));
    // Compute the one electron integrals, telling each object where to store the result
    sOBI->compute(sMat);

    //
    // BEGIN TESTING OF LIBMATRIX
    //

    // NOTE: This will only initialize the "default" matrix type.
    matrix_globals::initialize(Process::arguments.argc(), Process::arguments.argv());

    printf("Using %s matrix interface\n", matrix_globals::interface_name.c_str());

    Dimension m(1), n(1);
    m[0] = n[0] = aoBasis->nbf();

    // Generate default matrices.
    //    Either dist_matrix, serial_matrix, or elemental_matrix
    //    decided at configure/compile time.
    //
    matrix overlap = create_matrix("Overlap", m, n);
    overlap.set(sMat);

    sMat->print(stdout);
    overlap.print();

    matrix vec = create_matrix("vec", m, n);
    vector val = create_vector("val", n);
    overlap.diagonalize(vec, val);

    vec.print();
    val.print();

    // TESTING GENERAL MATRIX CREATION AND GEMM
    {
//        matrix a = create_matrix("A", m, n);
//        matrix b = create_matrix("B", m, n);
//        matrix c = create_matrix("C", m, n);
//        matrix vx= create_matrix("C", m, n);
//        matrix copy = create_matrix("Copy", m, n);
//        vector vw= create_vector("V", n);
//        a.fill(1.0);
//        a.print();
//        b.fill(2.0);
//        b.print();
//        c.gemm(false, false, 1.0, a, b, 0.0);
//        printf("Result of A*B\n");
//        c.print();
//
//        copy.set(c);
//        copy.print();
//        c.diagonalize(vx, vw);
//        vx.print();
//        vw.print();
    }

    // Create a dist_matrix:
    //     Generates compile error if madness is not available.
    //     NOTE: First error generated by the following line makes it obvious what the problem is.

//    dist_matrix c = create_specific_matrix<dist_matrix>("C", m, n);
//    c.print();

    // Create a serial_matrix
    //     serial_matrix is always available. Compiler errors should never be generated.
    {
        //  printf("Using serial_matrix objects\n");
        //  serial_matrix d = create_specific_matrix<serial_matrix>("D", m, n);
        //  serial_matrix e = create_specific_matrix<serial_matrix>("E", m, n);
        //  serial_matrix f = create_specific_matrix<serial_matrix>("F", m, n);
        //  serial_matrix g = create_specific_matrix<serial_matrix>("G", m, n);
        //  serial_vector h = create_specific_vector<serial_vector>("H", n);
        //  d.fill(1.0);
        //  e.fill(3.0);
        //  f.gemm(false, false, 1.0, d, e, 0.0);
        //  f.print();
        //  f.diagonalize(g, h);
        //  g.print();
        //  h.print();
        //
        //    d.add(c);    // throws at runtime
    }
    // Create an elmental matrix
    //     Generates compile error if elemental is not available.
//    elemental_matrix e = create_specific_matrix<elemental_matrix>("E", m, n);

    return Success;
}

}} // End namespaces

