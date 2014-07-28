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

#ifndef libmints_gridblocker_H
#define libmints_gridblocker_H

#include <psi4-dec.h>
#include <psiconfig.h>
#include <libmints/vector3.h>

namespace psi {

class PSIO;
class BasisSet;
class Matrix;
class Vector;
class IntVector;
class Vector3;
class BasisExtents;
class BlockOPoints;

/**
 * Class to determine groupings of DFT grid points for 
 * efficient sparse evaluation of density
 */
class GridBlocker {

protected:
    
    int debug_;
    int print_;
    int bench_;

    // Reference to previous grid layout
    const int npoints_ref_;
    double const* x_ref_;  
    double const* y_ref_;  
    double const* z_ref_;  
    double const* w_ref_;  

    const int tol_max_points_;
    const int tol_min_points_;
    const double tol_max_radius_;
    boost::shared_ptr<BasisExtents> extents_;

    // New grid layout (built in blocks -- method specific)
    int npoints_;
    int max_points_;
    int max_functions_;
    double* x_;
    double* y_;
    double* z_;
    double* w_;
    std::vector<boost::shared_ptr<BlockOPoints> > blocks_;
    
public:
    
    GridBlocker(const int npoints_ref, double const* x_ref, double const* y_ref, double const* z_ref,
        double const* w_ref, const int max_points, const int min_points, const double max_radius,
        boost::shared_ptr<BasisExtents> extents);
    virtual ~GridBlocker();
    
    virtual void block() = 0;

    int npoints() const { return npoints_; }
    int max_points() const { return max_points_; }
    int max_functions() const { return max_functions_; } 
    double* x() const { return x_; }
    double* y() const { return y_; }
    double* z() const { return z_; }
    double* w() const { return w_; }
    const std::vector<boost::shared_ptr<BlockOPoints> >& blocks() const { return blocks_; }

    void set_print(int print) { print_ = print; }
    void set_debug(int debug) { debug_ = debug; }
    void set_bench(int bench) { bench_ = bench; }

};

/**
 * Naive stride-based blocking 
 */
class NaiveGridBlocker : public GridBlocker {

public:
    
    NaiveGridBlocker(const int npoints_ref, double const* x_ref, double const* y_ref, double const* z_ref,
        double const* w_ref, const int max_points, const int min_points, const double max_radius,
        boost::shared_ptr<BasisExtents> extents);
    virtual ~NaiveGridBlocker();
    
    virtual void block();
};

/**
 * Octree-based blocking 
 */
class OctreeGridBlocker : public GridBlocker {

public:
    
    OctreeGridBlocker(const int npoints_ref, double const* x_ref, double const* y_ref, double const* z_ref,
        double const* w_ref, const int max_points, const int min_points, const double max_radius,
        boost::shared_ptr<BasisExtents> extents);
    virtual ~OctreeGridBlocker();
    
    virtual void block();
};

}
#endif

