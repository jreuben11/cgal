// Copyright (c) 2008 ETH Zurich (Switzerland)
// Copyright (c) 2008-2009 INRIA Sophia-Antipolis (France)
// Copyright (c) 2011 GeometryFactory Sarl (France)
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Andreas Fabri, Laurent Rineau


#ifndef CGAL_INTERNAL_STATIC_FILTERS_DO_INTERSECT_3_H
#define CGAL_INTERNAL_STATIC_FILTERS_DO_INTERSECT_3_H

#include <CGAL/Bbox_3.h>
#include <CGAL/Profile_counter.h>
#include <CGAL/internal/Static_filters/Static_filter_error.h>
#include <CGAL/internal/Static_filters/tools.h>

#include <CGAL/Intersections_3/Bbox_3_Segment_3.h>
// for CGAL::internal::do_intersect_bbox_segment_aux

#include <iostream>

// inspired from http://cag.csail.mit.edu/~amy/papers/box-jgt.pdf

namespace CGAL {

namespace internal {

namespace Static_filters_predicates {


template < typename K_base, typename SFK >
class Do_intersect_3
  : public K_base::Do_intersect_3
{
  typedef typename K_base::Point_3   Point_3;
  typedef typename K_base::Ray_3     Ray_3;
  typedef typename K_base::Segment_3 Segment_3;
  typedef typename K_base::Triangle_3 Triangle_3;
  typedef typename K_base::Tetrahedron_3 Tetrahedron_3;
  typedef typename K_base::Sphere_3 Sphere_3;
  typedef typename K_base::Do_intersect_3 Base;

public:

  typedef typename Base::result_type  result_type;

  using Base::operator();

  Sign sign_with_error(const double x, const double error) const {
    if(x > error) return POSITIVE;
    else if( x < - error) return NEGATIVE;
    else return ZERO;
  }


  // The internal::do_intersect(..) function
  // only performs orientation tests on the vertices
  // of the triangle and the segment
  // By calling the do_intersect function with
  // the  statically filtered kernel we avoid
  // that doubles are put into Interval_nt
  // to get taken out again with fit_in_double
  result_type
  operator()(const Segment_3 &s, const Triangle_3& t) const
  {
    return Intersections::internal::do_intersect(t,s, SFK());
  }

  result_type
  operator()(const Triangle_3& t, const Segment_3 &s) const
  {
    return Intersections::internal::do_intersect(t,s, SFK());
  }

  result_type
  operator()(const Triangle_3 &t0, const Triangle_3& t1) const
  {
    return Intersections::internal::do_intersect(t0,t1, SFK());
  }


  result_type
  operator()(const Bbox_3& b, const Segment_3 &s) const
  {
    return this->operator()(s, b);
  }

  result_type
  operator()(const Segment_3 &s, const Bbox_3& b) const
  {
    CGAL_BRANCH_PROFILER_3(std::string("semi-static failures/attempts/calls to   : ") +
                           std::string(CGAL_PRETTY_FUNCTION), tmp);

    Get_approx<Point_3> get_approx; // Identity functor for all points
                                    // but lazy points
    const Point_3& p = s.source();
    const Point_3& q = s.target();

    double px, py, pz, qx, qy, qz;
    if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
        fit_in_double(get_approx(p).z(), pz) &&
        fit_in_double(get_approx(q).x(), qx) && fit_in_double(get_approx(q).y(), qy) &&
        fit_in_double(get_approx(q).z(), qz) )
    {
      CGAL_BRANCH_PROFILER_BRANCH_1(tmp);

      const Uncertain<result_type> ub =
        Intersections::internal::do_intersect_bbox_segment_aux
        <double,
         true, // bounded at t=0
         true, // bounded at t=1
         true> // do use static filters
        (px, py, pz,
         qx, qy, qz,
         b);

      if(!is_indeterminate(ub)) return ub.sup();
      CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
    }
    return Base::operator()(s,b);
  }



  result_type
  operator()(const Tetrahedron_3 &t, const Bbox_3& b) const
  {
    Get_approx<Point_3> get_approx;
    double px, py, pz;

    if(! do_overlap(t.bbox(),b)){
      return false;
    }
    {
      const Point_3& p = t[0];
      if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
          fit_in_double(get_approx(p).z(), pz) )
        {
          if( (px >= b.xmin() && px <= b.xmax()) && (py >= b.ymin() && py <= b.ymax())  && (pz >= b.zmin() && pz <= b.zmax()) ){
            return true;
          }
        }else{
        return Base::operator()(t,b);
      }
    }

    {
      const Point_3& p = t[1];
      if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
          fit_in_double(get_approx(p).z(), pz) )
        {
          if( (px >= b.xmin() && px <= b.xmax()) && (py >= b.ymin() && py <= b.ymax())  && (pz >= b.zmin() && pz <= b.zmax()) ){
              return true;
          }
        }else{
        return Base::operator()(t,b);
      }
    }

    {
      const Point_3& p = t[2];
      if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
          fit_in_double(get_approx(p).z(), pz) )
        {
          if( (px >= b.xmin() && px <= b.xmax()) && (py >= b.ymin() && py <= b.ymax())  && (pz >= b.zmin() && pz <= b.zmax()) ){
              return true;
          }
        }else{
        return Base::operator()(t,b);
      }
    }



    {
      const Point_3& p = t[3];
      if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
          fit_in_double(get_approx(p).z(), pz) )
        {
          if( (px >= b.xmin() && px <= b.xmax()) && (py >= b.ymin() && py <= b.ymax())  && (pz >= b.zmin() && pz <= b.zmax()) ){
              return true;
          }
        }
    }

    return Base::operator()(t,b);
  }


  result_type
  operator()(const Bbox_3& b, const Ray_3 &r) const
  {
    return this->operator()(r, b);
  }

  result_type
  operator()(const Ray_3 &r, const Bbox_3& b) const
  {
    CGAL_BRANCH_PROFILER_3(std::string("semi-static failures/attempts/calls to   : ") +
                           std::string(CGAL_PRETTY_FUNCTION), tmp);

    Get_approx<Point_3> get_approx; // Identity functor for all points
    // but lazy points.
    const Point_3& p = r.source();
    const Point_3& q = r.second_point();

    double px, py, pz, qx, qy, qz;
    if (fit_in_double(get_approx(p).x(), px) && fit_in_double(get_approx(p).y(), py) &&
        fit_in_double(get_approx(p).z(), pz) &&
        fit_in_double(get_approx(q).x(), qx) && fit_in_double(get_approx(q).y(), qy) &&
        fit_in_double(get_approx(q).z(), qz) )
    {
      CGAL_BRANCH_PROFILER_BRANCH_1(tmp);

      const Uncertain<result_type> ub =
        Intersections::internal::do_intersect_bbox_segment_aux
        <double,
         true, // bounded at t=0
         false,// not bounded at t=1
         true> // do use static filters
        (px, py, pz,
         qx, qy, qz,
         b);

      if( !is_indeterminate(ub) ) return ub.sup();
      CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
    }
    return Base::operator()(r,b);
  }


  result_type
  operator()(const Bbox_3& b, const Triangle_3 &t) const
  {
    return this->operator()(t, b);
  }


  result_type
  operator()(const Triangle_3 &t, const Bbox_3& b) const
  {
    // check overlaps between bboxes
    if(! do_overlap(t.bbox(),b)){
      return false;
    }

    // check if at least one triangle point is inside the bbox
    Get_approx<Point_3> get_approx;
    std::array< std::array<double, 3>, 3> pts;

    for (int i=0; i<3; ++i)
    {
      const Point_3& p = t[i];
      if (fit_in_double(get_approx(p).x(), pts[i][0]) && fit_in_double(get_approx(p).y(), pts[i][1]) &&
          fit_in_double(get_approx(p).z(), pts[i][2]) )
      {
        if( (pts[i][0] >= b.xmin() && pts[i][0] <= b.xmax()) && (pts[i][1] >= b.ymin() && pts[i][1] <= b.ymax())  && (pts[i][2] >= b.zmin() && pts[i][2] <= b.zmax()) ){
          return true;
        }
      }
      else
      {
        return Base::operator()(t,b);
      }
    }

    // copy of the regular code with do_axis_intersect_aux_impl statically filtered
    auto do_axis_intersect_aux_impl = [](double alpha, double beta, double c_alpha, double c_beta) -> Uncertain<Sign>
    {
      Sign int_tmp_result;
      double double_tmp_result;
      double eps;
      double_tmp_result = ((-c_alpha * alpha) + (c_beta * beta));
      double max1 = fabs(c_alpha);
      if( (max1 < fabs(c_beta)) )
      {
          max1 = fabs(c_beta);
      }
      double max2 = fabs(alpha);
      if( (max2 < fabs(beta)) )
      {
          max2 = fabs(beta);
      }
      double lower_bound_1;
      double upper_bound_1;
      lower_bound_1 = max1;
      upper_bound_1 = max1;
      if( (max2 < lower_bound_1) )
      {
          lower_bound_1 = max2;
      }
      else
      {
          if( (max2 > upper_bound_1) )
          {
              upper_bound_1 = max2;
          }
      }
      if( (lower_bound_1 < 5.00368081960964746551e-147) )
      {
          return Uncertain<Sign>();
      }
      else
      {
          if( (upper_bound_1 > 1.67597599124282389316e+153) )
          {
              return Uncertain<Sign>();
          }
          eps = (8.88720573725927976811e-16 * (max1 * max2));
          if( (double_tmp_result > eps) )
          {
              int_tmp_result = POSITIVE;
          }
          else
          {
              if( (double_tmp_result < -eps) )
              {
                  int_tmp_result = NEGATIVE;
              }
              else
              {
                  return Uncertain<Sign>();
              }
          }
      }
      return int_tmp_result;
    };

    auto do_intersect_plane_bbox = [](const Triangle_3& t, const Bbox_3& bbox)
    {
      typename SFK::Orientation_3 orient = SFK().orientation_3_object();
      CGAL::Orientation side = orient(t[0], t[1],t[2], Point_3(bbox.xmin(), bbox.ymin(),bbox.zmin()));
      if(side == COPLANAR) return true;
      CGAL::Oriented_side s = orient(t[0], t[1],t[2], Point_3(bbox.xmax(), bbox.ymax(),bbox.zmax()));
      if(s != side) return true;
      s = orient(t[0], t[1],t[2], Point_3(bbox.xmin(), bbox.ymin(),bbox.zmax()));
      if(s != side) return true;
      s = orient(t[0], t[1],t[2], Point_3(bbox.xmax(), bbox.ymax(),bbox.zmin()));
      if(s != side) return true;
      s = orient(t[0], t[1],t[2], Point_3(bbox.xmin(), bbox.ymax(),bbox.zmin()));
      if(s != side) return true;
       s = orient(t[0], t[1],t[2], Point_3(bbox.xmax(), bbox.ymin(),bbox.zmax()));
      if(s != side) return true;
       s = orient(t[0], t[1],t[2], Point_3(bbox.xmin(), bbox.ymax(),bbox.zmax()));
      if(s != side) return true;
       s = orient(t[0], t[1],t[2], Point_3(bbox.xmax(), bbox.ymin(),bbox.zmin()));
      if(s != side) return true;
      return false;
    };

    if ( !do_intersect_plane_bbox(t,b) )
      return false;

    Uncertain<bool> res = Intersections::internal::do_intersect_bbox_or_iso_cuboid_impl<double>(pts, b, do_axis_intersect_aux_impl);
    if ( !is_indeterminate(res) )
      return make_certain(res);

    return Base::operator()(t,b);
  }

  result_type
  operator()(const Bbox_3& b, const Sphere_3 &s) const
  {
    return this->operator()(s, b);
  }

  // The parameter overestimate is used to avoid a filter failure in AABB_tree::closest_point()
  result_type
  operator()(const Sphere_3 &s, const Bbox_3& b, bool overestimate = false) const
  {
    CGAL_BRANCH_PROFILER_3(std::string("semi-static failures/attempts/calls to   : ") +
                           std::string(CGAL_PRETTY_FUNCTION), tmp);

    Get_approx<Point_3> get_approx; // Identity functor for all points
    const Point_3& c = s.center();

    double scx, scy, scz, ssr;
    double bxmin = b.xmin() , bymin = b.ymin() , bzmin = b.zmin() ,
           bxmax = b.xmax() , bymax = b.ymax() , bzmax = b.zmax() ;

    if (fit_in_double(get_approx(c).x(), scx) &&
        fit_in_double(get_approx(c).y(), scy) &&
        fit_in_double(get_approx(c).z(), scz) &&
        fit_in_double(s.squared_radius(), ssr))
    {
      CGAL_BRANCH_PROFILER_BRANCH_1(tmp);

      if ((ssr < 1.11261183279326254436e-293) || (ssr > 2.80889552322236673473e+306)){
        CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
        return Base::operator()(s,b);
      }
      double distance = 0;
      double max1 = 0;
      double double_tmp_result = 0;
      double eps = 0;
      if(scx < bxmin)
      {
        double bxmin_scx = bxmin - scx;
        max1 = bxmin_scx;

        distance = square(bxmin_scx);
        double_tmp_result = (distance - ssr);

        if( (max1 < 3.33558365626356687717e-147) || (max1 > 1.67597599124282407923e+153) ){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }
      else if(scx > bxmax)
      {
        double scx_bxmax = scx - bxmax;
        max1 = scx_bxmax;

        distance = square(scx_bxmax);
        double_tmp_result = (distance - ssr);

        if( (max1 < 3.33558365626356687717e-147) || (max1 > 1.67597599124282407923e+153)){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }


      if(scy < bymin)
      {
        double bymin_scy = bymin - scy;
        if(max1 < bymin_scy){
          max1 = bymin_scy;
        }

        distance += square(bymin_scy);
        double_tmp_result = (distance - ssr);

        if( (max1 < 3.33558365626356687717e-147) || ((max1 > 1.67597599124282407923e+153)) ){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }
      else if(scy > bymax)
      {
        double scy_bymax = scy - bymax;
        if(max1 < scy_bymax){
          max1 = scy_bymax;
        }
        distance += square(scy_bymax);
        double_tmp_result = (distance - ssr);

        if( ((max1 < 3.33558365626356687717e-147)) || ((max1 > 1.67597599124282407923e+153)) ){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }


      if(scz < bzmin)
      {
        double bzmin_scz = bzmin - scz;
        if(max1 < bzmin_scz){
          max1 = bzmin_scz;
        }
        distance += square(bzmin_scz);
        double_tmp_result = (distance - ssr);

        if( ((max1 < 3.33558365626356687717e-147)) || ((max1 > 1.67597599124282407923e+153))){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }
      else if(scz > bzmax)
      {
        double scz_bzmax = scz - bzmax;
        if(max1 < scz_bzmax){
          max1 = scz_bzmax;
        }

        distance += square(scz_bzmax);
        double_tmp_result = (distance - ssr);

        if( ((max1 < 3.33558365626356687717e-147)) || ((max1 > 1.67597599124282407923e+153)) ){
          if(overestimate){
            return true;
          }else{
            CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
            return Base::operator()(s,b);
          }
        }

        eps = 1.99986535548615598560e-15 * (std::max) (ssr, square(max1));

        if (double_tmp_result > eps){
          return false;
        }
      }

      // double_tmp_result and eps were growing all the time
      // no need to test for > eps as done earlier in at least one case
      if (double_tmp_result < -eps){
        return true;
      } else {
        if(overestimate){
          return true;
        }
        CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
        return Base::operator()(s,b);
      }

      CGAL_BRANCH_PROFILER_BRANCH_2(tmp);
    }
    return Base::operator()(s,b);
  }


  // Computes the epsilon for Bbox_3_Segment_3_do_intersect.
  static double compute_epsilon_bbox_segment_3()
  {
    typedef Static_filter_error F;
    F t1 = F(1);
    F f = ((t1 - t1) * (t1 - t1)) - ((t1 - t1) * (t1 - t1));
    F f1 = (t1 - t1);
    F f1bis = (t1 - t1) - (t1 - t1);
    F f2 = f1*f1;
    F f3 = f2 - f2;
    std::cerr << "epsilons:\n"
              << "  degre " << f1.degree() << ": " <<  f1.error() << "\n"
              << "  degre " << f1bis.degree() << ": " <<  f1bis.error() << "\n"
              << "  degre " << f2.degree() << ": " <<  f2.error() << "\n"
              << "  degre " << f3.degree() << ": " <<  f3.error() << "\n";

    double err = f.error();
    err += err * 2 *  F::ulp(); // Correction due to "eps * m * m".  Do we need 2 ?
    std::cerr << "*** epsilon for Do_intersect_3(Bbox_3, Segment_3) = "
              << err << std::endl;
    std::cerr << "\n"
              << "Now for underflow/overflows...\n"
              << "        min_double/eps = "
              << (std::numeric_limits<double>::min)() / err << std::endl
              << "  sqrt(min_double/eps) = "
              << CGAL::sqrt((std::numeric_limits<double>::min)() / err) << std::endl;
    return err;
  }

}; // class Do_intersect_3

} // end namespace Static_filters_predicates

} // end namespace internal


} // end namespace CGAL

#endif  // CGAL_INTERNAL_STATIC_FILTERS_DO_INTERSECT_3_H
