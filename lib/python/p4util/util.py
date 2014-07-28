#
#@BEGIN LICENSE
#
# PSI4: an ab initio quantum chemistry software package
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#@END LICENSE
#

"""Module with utility functions for use in input files."""
import psi4
import sys
import os
import math
from psiexceptions import *

def oeprop(*args, **kwargs):
    oe = psi4.OEProp()
    if 'title' in kwargs:
        oe.set_title(kwargs['title'])
    for prop in args:
        oe.add(prop)
    oe.compute()

def set_memory(bytes):
    """Function to reset the total memory allocation."""
    psi4.set_memory(bytes)


def get_memory():
    """Function to return the total memory allocation."""
    return psi4.get_memory()


def set_num_threads(nthread):
    """Function to reset the number of threads to parallelize across."""
    psi4.set_nthread(nthread)


def get_num_threads():
    """Function to return the number of threads to parallelize across."""
    return psi4.nthread()


def success(label):
    """Function to print a '*label*...PASSED' line to screen.
    Used by :py:func:`util.compare_values` family when functions pass.

    """
    print('\t{0:.<66}PASSED'.format(label))
    sys.stdout.flush()


# Test functions
def compare_values(expected, computed, digits, label):
    """Function to compare two values. Prints :py:func:`util.success`
    when value *computed* matches value *expected* to number of *digits*.
    Performs a system exit on failure. Used in input files in the test suite.

    """
    if (abs(expected - computed) > 10 ** (-digits)):
        print(("\t%s: computed value (%." + str(digits+1) + "f) does not match (%." + str(digits+1) + "f) to %d decimal places.") % (label, computed, expected, digits))
        sys.exit(1)
    if ( math.isnan(computed) ):
        print("\t%s: computed value (%f) does not match (%f) to %d decimal places.\n" % (label, computed, expected, digits))
        print("\tprobably because the computed value is nan.")
        sys.exit(1)
    success(label)


def compare_integers(expected, computed, label):
    """Function to compare two integers. Prints :py:func:`util.success`
    when value *computed* matches value *expected*.
    Performs a system exit on failure. Used in input files in the test suite.

    """
    if (expected != computed):
        print("\t%s: computed value (%d) does not match (%d)." % (label, computed, expected))
        sys.exit(1)
    success(label)


def compare_strings(expected, computed, label):
    """Function to compare two strings. Prints :py:func:`util.success`
    when string *computed* exactly matches string *expected*.
    Performs a system exit on failure. Used in input files in the test suite.

    """
    if(expected != computed):
        print("\t%s: computed value (%s) does not match (%s)." % (label, computed, expected))
        sys.exit(1)
    success(label)


def compare_matrices(expected, computed, digits, label):
    """Function to compare two matrices. Prints :py:func:`util.success`
    when elements of matrix *computed* match elements of matrix *expected* to
    number of *digits*. Performs a system exit on failure to match symmetry
    structure, dimensions, or element values. Used in input files in the test suite.

    """
    if (expected.nirrep() != computed.nirrep()):
        print("\t%s has %d irreps, but %s has %d\n." % (expected.name(), expected.nirrep(), computed.name(), computed.nirrep()))
        sys.exit(1)
    if (expected.symmetry() != computed.symmetry()):
        print("\t%s has %d symmetry, but %s has %d\n." % (expected.name(), expected.symmetry(), computed.name(), computed.symmetry()))
        sys.exit(1)
    nirreps = expected.nirrep()
    symmetry = expected.symmetry()
    for irrep in range(nirreps):
        if(expected.rows(irrep) != computed.rows(irrep)):
            print("\t%s has %d rows in irrep %d, but %s has %d\n." % (expected.name(), expected.rows(irrep), irrep, computed.name(), computed.rows(irrep)))
            sys.exit(1)
        if(expected.cols(irrep ^ symmetry) != computed.cols(irrep ^ symmetry)):
            print("\t%s has %d columns in irrep, but %s has %d\n." % (expected.name(), expected.cols(irrep), irrep, computed.name(), computed.cols(irrep)))
            sys.exit(1)
        rows = expected.rows(irrep)
        cols = expected.cols(irrep ^ symmetry)
        failed = 0
        for row in range(rows):
            for col in range(cols):
                if(abs(expected.get(irrep, row, col) - computed.get(irrep, row, col)) > 10 ** (-digits)):
                    print("\t%s: computed value (%s) does not match (%s)." % (label, expected.get(irrep, row, col), computed.get(irrep, row, col)))
                    failed = 1
                    break

        if(failed):
            print("Check your output file for reporting of the matrices.")
            psi4.print_out("The Failed Test Matrices\n")
            psi4.print_out("Computed Matrix (2nd matrix passed in)\n")
            computed.print_out()
            psi4.print_out("Expected Matrix (1st matrix passed in)\n")
            expected.print_out()
            sys.exit(1)
    success(label)


def compare_vectors(expected, computed, digits, label):
    """Function to compare two vectors. Prints :py:func:`util.success`
    when elements of vector *computed* match elements of vector *expected* to
    number of *digits*. Performs a system exit on failure to match symmetry
    structure, dimension, or element values. Used in input files in the test suite.

    """
    if (expected.nirrep() != computed.nirrep()):
        print("\t%s has %d irreps, but %s has %d\n." % (expected.name(), expected.nirrep(), computed.name(), computed.nirrep()))
        sys.exit(1)
    nirreps = expected.nirrep()
    for irrep in range(nirreps):
        if(expected.dim(irrep) != computed.dim(irrep)):
            print("\tThe reference has %d entries in irrep %d, but the computed vector has %d\n." % (expected.dim(irrep), irrep, computed.dim(irrep)))
            sys.exit(1)
        dim = expected.dim(irrep)
        failed = 0
        for entry in range(dim):
            if(abs(expected.get(irrep, entry) - computed.get(irrep, entry)) > 10 ** (-digits)):
                print("\t%s: computed value (%s) does not match (%s)." % (label, computed.get(irrep, entry), expected.get(irrep, entry)))
                failed = 1
                break

        if(failed):
            psi4.print_out("The computed vector\n")
            computed.print_out()
            psi4.print_out("The reference vector\n")
            expected.print_out()
            sys.exit(1)
    success(label)

def copy_file_to_scratch(filename, prefix, namespace, unit, move = False):

    """Function to move file into scratch with correct naming
    convention.

    Arguments:

    @arg filename  full path to file
    @arg prefix    computation prefix, usually 'psi'
    @arg namespace context namespace, usually molecule name
    @arg unit      unit number, e.g. 32
    @arg move      copy or move? (default copy)

    Example:

    Assume PID is 12345 and SCRATCH is /scratch/parrish/

    copy_file_to_scratch('temp', 'psi', 'h2o', 32):
        -cp ./temp /scratch/parrish/psi.12345.h2o.32
    copy_file_to_scratch('/tmp/temp', 'psi', 'h2o', 32):
        -cp /tmp/temp /scratch/parrish/psi.12345.h2o.32
    copy_file_to_scratch('/tmp/temp', 'psi', '', 32):
        -cp /tmp/temp /scratch/parrish/psi.12345.32
    copy_file_to_scratch('/tmp/temp', 'psi', '', 32, True):
        -mv /tmp/temp /scratch/parrish/psi.12345.32

    """

    pid = str(os.getpid())
    scratch = psi4.IOManager.shared_object().get_file_path(int(unit))

    cp = '/bin/cp';
    if move:
        cp = '/bin/mv';

    unit = str(unit)

    target = ''
    target += prefix
    target += '.'
    target += pid
    if len(namespace):
        target += '.'
        target += namespace
    target += '.'
    target += unit

    command = ('%s %s %s/%s' % (cp, filename, scratch, target))

    os.system(command)
    #print command

def copy_file_from_scratch(filename, prefix, namespace, unit, move = False):

    """Function to move file out of scratch with correct naming
    convention.

    Arguments:

    @arg filename  full path to target file
    @arg prefix    computation prefix, usually 'psi'
    @arg namespace context namespace, usually molecule name
    @arg unit      unit number, e.g. 32
    @arg move      copy or move? (default copy)

    Example:

    Assume PID is 12345 and SCRATCH is /scratch/parrish/

    copy_file_to_scratch('temp', 'psi', 'h2o', 32):
        -cp /scratch/parrish/psi.12345.h2o.32 .temp
    copy_file_to_scratch('/tmp/temp', 'psi', 'h2o', 32):
        -cp /scratch/parrish/psi.12345.h2o.32 /tmp/temp
    copy_file_to_scratch('/tmp/temp', 'psi', '', 32):
        -cp /scratch/parrish/psi.12345.32 /tmp/temp
    copy_file_to_scratch('/tmp/temp', 'psi', '', 32, True):
        -mv /scratch/parrish/psi.12345.32 /tmp/temp

    """

    pid = str(os.getpid())
    scratch = psi4.IOManager.shared_object().get_file_path(int(unit))

    cp = '/bin/cp';
    if move:
        cp = '/bin/mv';

    unit = str(unit)

    target = ''
    target += prefix
    target += '.'
    target += pid
    if len(namespace):
        target += '.'
        target += namespace
    target += '.'
    target += unit

    command = ('%s %s/%s %s' % (cp, scratch, target, filename))

    os.system(command)
    #print command

