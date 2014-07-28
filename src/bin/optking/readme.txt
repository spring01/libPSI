** Very incomplete notes on optking ** 
** -RAK, Feb. 2011 

**** User Notes ****
The optking program performs optimization of molecular geometries using
analytic gradients, and possibly the second derivative.  The default minimization 
procedure employs redundant internal coordinates, an RFO step, BFGS Hessian update, 

Optimizations are peformed in redundant internal coordinates.  The principal literature
references include the introduction of redundant internal coordinates in:
C. Peng, P.Y. Ayala,  H.B. Schlegel, and M.J. Frisch, J. Comput. Chem., 17, 49 (1996).

The general algorithm employed in this code is the "model Hessian" plus "RF method"
described and tested in:
V. Bakken and T. Helgaker, J. Chem. Phys., 117, 9160 (2002).

For separated fragments, we connect by only the single, nearest atoms
We choose not to employ their "extra-redundant" internal coordinates defined by
"auxiliary interfragment" bonds.

The initial Hessian guess is either "SCHLEGEL" defined in:
Schlegel, Theor. Chim. Acta, 66, 333 (1984)   _or_
or "FISCHER" defined in:
Fischer and Almlof, J. Phys. Chem., 96, 9770 (1992).
For the interfragment coordinates (if present), the initial guess can be computed
using the Fischer formulas (INTERFRAGMENT_H == FISCHER_LIKE).  By default,
the interfragment coordinates that are stretches, bends, and dihedral angles are given
the force constants 0.007, 0.003, and 0.001 au, respectively.

Hydrogen bonds are identified as not bonded according to the covalent criteria, and
including the bonding pattern [N,O,F,Cl]-H ... [N,O,F,Cl].  The distance must be
less than 4.3 au (changeable by "MAXIMUM_H_BOND_DISTANCE"), and the X-H ... Y angle must
be greater than 90 degrees.  If an interfragment stretch is (or includes) a hydrogen bond, 
the the default guess at the force constants that are stretches, bends, and dihedral angles
are larger: 0.03, 0.007, and 0.002 respectively.


The default Hessian update scheme is the popular BFGS scheme.  Other options include
the MS, POWELL, and BOFILL updates (no update scheme is NO).  The number of previous steps'
data to be used in the update can be controlled by the user (H_update_use_last).

If H_UPDATE_LIMIT = true, then changes to the Hessian from updating are limited in magnitude.
Specifically, if changes cannot exceed the maximum of H_UPDATE_LIMIT_SCALE times the previous
value of the Hessian element, or H_UPDATE_LIMIT_MAX (in au). 

See Schlegel _Ab Initio Methods in Quantum Chemistry_ (1987) for definitions, as well as
J. M. Bofill, J. Comp. Chem., Vol. 15, pages 1-11 (1994).


** Format of intco.dat file **

F fragment definition header

R stretch
H hydrogen-bond

B bend
D dihedral
L linear bend complement to an ordinary bend

Fragment[F]  1st_atom_of_fragment 2nd_atom_of_fragment
Coordinate_Type[S,H,B,D]  
representing stretch, hydrogen-bond, bend, dihedral
S atom1 atom2
H atom1 atom2
B atom1 atom2 atom3
D atom1 atom2 atom3 atom4

[I]nterfragment 1st_fragment 2nd_fragment
six 1's or 0's to indicate which of the interfragment coordinates are defined and to be used
The necessary reference points are listed next, followed by one or more atoms.  The reference point
is defined as an evenly weighted linear combination of the listed atoms.
A1
A2
A3
B1
B2
B3
The coordinate type is followed by a "*" for a frozen coordinate.
Frozen interfragment coordinates are selected by placing a "*" after the "1" of the
corresponding coordinate.

Currently, all interfragment stretches are R(AB) coordinates.
 If INTERFRAGMENT_DISTANCE_INVERSE=true, then 1/R(AB) coordinates are used for all interfragment stretches.

****  Programmer Notes ****
*******************
Package specific routines 
*******************
package.h
OPTKING_PACKAGE_PSI/QCHEM specifies package

*io.h
contains declarations for wrapper functions to read and write from the binary i/o file

*globals.h
PSI/QCHEM : declares text output file (opt::outfile)
PSI :
defines set names of files for input/output
includes "psi4-dec.h"

*optking.cc:
PSI/QCHEM :
 define return type OptReturnType for optking function
 defines the possible optking return values (at top of file)

 functions to open/set and close opt::outfile file pointer

 opens/closes intco.dat file for writing
 opens/closes intco.dat file stream for reading

PSI: accepts psi::Options object as argument

* set_params.cc:
assign values to optimization keyword options

* geom_gradients_io.cc
read_natoms()     gets number of atoms and returns it
read_geom_grad()  gets geometry and gradient and inserts into molecule fragment

****
Libraries
****

The "v3d" library (in namespace opt::v3d) contains functions that
operate on arrays with 3 elements (3D-vectors).

The "mem" library (in namespace opt) provides memory allocation
and deallocation functions.

****
Internal Coordinate Classes
****

The "simple.h" header declares SIMPLE, a simple internal coordinate base class containing
INTCO_TYPE s_type  - the type of simple coordinate (stre, bend, etc.)
int s_natom   - the # of atoms in definition
int *s_atom   - indices of atoms in definition (numbering within the fragment)
bool s_frozen - whether the coordinate is frozen

These variables are retrieved using the public functions:
g_type(); g_natom(); g_atom(int a); is_frozen();

Frozenness is toggled by freeze() , unfreeze() .

Some derived class functions are
virtual void fix_near_180(); // for torions
virtual bool is_hbond();     // for stretches - whether H-bond
virtual bool is_inverse_stre() // for stretches - whether 1/R

Contains the following virtual functions which must be provided by derived classes:
// function to print coordinate definitions to file
virtual void print_intco_dat(FILE *fp, int atom_offset=0) const = 0;
// return value of internal coordinate
virtual double value(GeomType geom) const = 0;
// compute s vector (dq/dx, dq/dy, dq/dz)
virtual double ** DqDx(GeomType geom) const = 0;
// compute second derivative (B' matrix elements)
// dq_i dq_j / dx^2, dydx, dy^2, dzdx, dzdy, dz^2 
virtual double ** Dq2Dx2(GeomType geom) const = 0;
// print coordinates and value to output file
virtual void print(FILE *fp, GeomType geom, int atom_offset=0) const = 0;
// print coordinates and displacements 
virtual void print_disp(FILE *fp, const double old_q, const double f_q,
const double dq, const double new_q, int atom_offset = 0) const = 0;
// for debugging, print s vectors to output file
virtual void print_s(FILE *fp, GeomType geom) const = 0;
// an equality operator
virtual bool operator==(const SIMPLE & s2) const  = 0;

The derived classes are STRE, BEND, TORS.

****
FRAG class
****
The frag.h file declares a fragment class whose members include:
 number of atoms
 atomic numbers
 geometry 
 gradient
 nuclear masses 
 connectivity 
 // a vector of pointers to the intrafragment internal coordinates
 vector<SIMPLE *> simples

****
INTERFRAG class
****
The interfrag.h file declares a class for a set of interfragment coordinates.  The
private members include:
  pointers to the fragments connected
  index numbers for the fragments connected (in the MOLECULE)
  number of reference points used on each fragment
  weights applied to each reference points on each fragment
  a pseudo-fragment FRAG which contains only the location of the reference atoms
  booleans to indicate which of the 6 possible interfragment coodinates are present

INTERFRAG is a friend class to FRAG, so these can manipulate the fragments they connect.

****
MOLECULE Class
****
molecule.h declares the MOLECULE class which contains:
 a vector of pointers to fragments
 a vector of pointers to interfragment coordinate sets
 (possibly) the energy

