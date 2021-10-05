// Copyright (c) 2015-2021 GeometryFactory (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jane Tournois


#ifndef CGAL_POLYGON_MESH_PROCESSING_TANGENTIAL_RELAXATION_H
#define CGAL_POLYGON_MESH_PROCESSING_TANGENTIAL_RELAXATION_H

#include <CGAL/license/Polygon_mesh_processing/meshing_hole_filling.h>

#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/property_map.h>

#include <CGAL/boost/graph/Named_function_parameters.h>
#include <CGAL/boost/graph/named_params_helper.h>

#include <tuple>
#include <type_traits>

#ifdef DOXYGEN_RUNNING
#define CGAL_PMP_NP_TEMPLATE_PARAMETERS NamedParameters
#define CGAL_PMP_NP_CLASS NamedParameters
#endif


namespace CGAL {
namespace Polygon_mesh_processing {

/*!
* \ingroup PMP_meshing_grp
* applies an iterative area-based tangential smoothing to the given range of vertices.
* Each vertex `v` is relocated to its gravity-weighted centroid, and the relocation vector
* is projected back to the tangent plane to the surface at `v`, iteratively.
* The connectivity remains unchanged.
*
* @tparam TriangleMesh model of `MutableFaceGraph`.
*         The descriptor types `boost::graph_traits<TriangleMesh>::%face_descriptor`
*         and `boost::graph_traits<TriangleMesh>::%halfedge_descriptor` must be
*         models of `Hashable`.
* @tparam VertexRange range of `boost::graph_traits<TriangleMesh>::%face_descriptor`,
*         model of `Range`. Its iterator type is `ForwardIterator`.
* @tparam NamedParameters a sequence of \ref bgl_namedparameters "Named Parameters"
*
* @param vertices the range of vertices which will be relocated by relaxation
* @param tm the triangle mesh to which `vertices` belong
* @param np an optional sequence of \ref bgl_namedparameters "Named Parameters" among the ones listed below
*
* \cgalNamedParamsBegin
*   \cgalParamNBegin{vertex_point_map}
*     \cgalParamDescription{a property map associating points to the vertices of `tm`}
*     \cgalParamType{a class model of `ReadWritePropertyMap` with `boost::graph_traits<PolygonMesh>::%vertex_descriptor`
*                    as key type and `%Point_3` as value type}
*     \cgalParamDefault{`boost::get(CGAL::vertex_point, tm)`}
*     \cgalParamExtra{If this parameter is omitted, an internal property map for `CGAL::vertex_point_t`
*                     must be available in `PolygonMesh`.}
*   \cgalParamNEnd
*
*   \cgalParamNBegin{geom_traits}
*     \cgalParamDescription{an instance of a geometric traits class}
*     \cgalParamType{a class model of `Kernel`}
*     \cgalParamDefault{a \cgal Kernel deduced from the `Point_3` type, using `CGAL::Kernel_traits`}
*     \cgalParamExtra{The geometric traits class must be compatible with the vertex `Point_3` type.}
*     \cgalParamExtra{Exact constructions kernels are not supported by this function.}
*   \cgalParamNEnd
*
*   \cgalParamNBegin{number_of_iterations}
*     \cgalParamDescription{the number of iterations smoothing iterations}
*     \cgalParamType{unsigned int}
*     \cgalParamDefault{`1`}
*   \cgalParamNEnd
*
*   \cgalParamNBegin{edge_is_constrained_map}
*     \cgalParamDescription{a property map containing the constrained-or-not status of each edge of `tm`.
*                           The endpoints of a constrained edge cannot be moved by relaxation.}
*     \cgalParamType{a class model of `ReadWritePropertyMap` with `boost::graph_traits<PolygonMesh>::%edge_descriptor`
*                    as key type and `bool` as value type. It must be default constructible.}
*     \cgalParamDefault{a default property map where no edges are constrained}
*     \cgalParamExtra{Patch boundary edges (i.e. incident to only one face in the range) are always considered as constrained edges.}
*   \cgalParamNEnd
*
*   \cgalParamNBegin{vertex_is_constrained_map}
*     \cgalParamDescription{a property map containing the constrained-or-not status of each vertex of `tm`.
*                           A constrained vertex cannot be modified during relaxation.}
*     \cgalParamType{a class model of `ReadWritePropertyMap` with `boost::graph_traits<PolygonMesh>::%vertex_descriptor`
*                    as key type and `bool` as value type. It must be default constructible.}
*     \cgalParamDefault{a default property map where no vertices are constrained}
*   \cgalParamNEnd
*
*   \cgalParamNBegin{relax_constraints}
*     \cgalParamDescription{If `true`, the end vertices of the edges set as constrained
*                           in `edge_is_constrained_map` and boundary edges move along the
*                           constrained polylines they belong to.}
*     \cgalParamType{Boolean}
*     \cgalParamDefault{`false`}
*   \cgalParamNEnd
*
* \cgalNamedParamsEnd
*
* \todo check if it should really be a triangle mesh or if a polygon mesh is fine
*/
  template <typename VertexRange, class TriangleMesh, class NamedParameters>
  void tangential_relaxation(const VertexRange& vertices,
                             TriangleMesh& tm,
                             const NamedParameters& np)
  {
  typedef typename boost::graph_traits<TriangleMesh>::vertex_descriptor vertex_descriptor;
  typedef typename boost::graph_traits<TriangleMesh>::halfedge_descriptor halfedge_descriptor;
  typedef typename boost::graph_traits<TriangleMesh>::edge_descriptor edge_descriptor;

  using parameters::get_parameter;
  using parameters::choose_parameter;

  typedef typename GetGeomTraits<TriangleMesh, NamedParameters>::type GT;
  GT gt = choose_parameter<GT>(get_parameter(np, internal_np::geom_traits));

  typedef typename GetVertexPointMap<TriangleMesh, NamedParameters>::type VPMap;
  VPMap vpm = choose_parameter(get_parameter(np, internal_np::vertex_point),
                               get_property_map(vertex_point, tm));

  typedef Static_boolean_property_map<edge_descriptor, false> Default_ECM;
  typedef typename internal_np::Lookup_named_param_def <
      internal_np::edge_is_constrained_t,
      NamedParameters,
      Static_boolean_property_map<edge_descriptor, false> // default (no constraint)
    > ::type ECM;
  ECM ecm = choose_parameter(get_parameter(np, internal_np::edge_is_constrained),
                             Default_ECM());

  typedef typename internal_np::Lookup_named_param_def <
      internal_np::vertex_is_constrained_t,
      NamedParameters,
      Static_boolean_property_map<vertex_descriptor, false> // default (no constraint)
    > ::type VCM;
  VCM vcm = choose_parameter(get_parameter(np, internal_np::vertex_is_constrained),
                             Static_boolean_property_map<vertex_descriptor, false>());

  const bool relax_constraints = choose_parameter(get_parameter(np, internal_np::relax_constraints), false);
  const unsigned int nb_iterations = choose_parameter(get_parameter(np, internal_np::number_of_iterations), 1);

  typedef typename GT::Vector_3 Vector_3;
  typedef typename GT::Point_3 Point_3;

  auto check_normals = [&](vertex_descriptor v)
  {
    bool first_run = true;
    Vector_3 prev = NULL_VECTOR, first = NULL_VECTOR;
    halfedge_descriptor first_h = boost::graph_traits<TriangleMesh>::null_halfedge();
    for (halfedge_descriptor hd : CGAL::halfedges_around_target(v, tm))
    {
      if (is_border(hd, tm)) continue;

      Vector_3 n = compute_face_normal(face(hd, tm), tm);
      if (n == CGAL::NULL_VECTOR) //for degenerate faces
        continue;

      if (first_run)
      {
        first_run = false;
        first = n;
        first_h = hd;
      }
      else
      {
        if (!get(ecm, edge(hd, tm)))
          if (to_double(n * prev) <= 0)
            return false;
      }
      prev = n;
    }
    if (!get(ecm, edge(first_h, tm)))
      if (to_double(first * prev) <= 0)
        return false;

    return true;
  };

  for (unsigned int nit = 0; nit < nb_iterations; ++nit)
  {
#ifdef CGAL_PMP_TANGENTIAL_RELAXATION_VERBOSE
    std::cout << "\r\t(Tangential relaxation iteration " << (nit + 1) << " / ";
    std::cout << nb_iterations << ") ";
    std::cout.flush();
#endif

    typedef std::tuple<vertex_descriptor, Vector_3, Point_3> VNP;
    std::vector< VNP > barycenters;
    // at each vertex, compute vertex normal
    // at each vertex, compute barycenter of neighbors
    for(vertex_descriptor v : vertices)
    {
      if (get(vcm, v) || halfedge(v, tm)==boost::graph_traits<TriangleMesh>::null_halfedge())
        continue;

      std::vector <halfedge_descriptor> interior_hedges, border_halfedges;

      // collect hedges to detect if we have to handle boundary cases
      for(halfedge_descriptor h : halfedges_around_target(v, tm))
      {
        if (is_border_edge(h, tm) || get(ecm, edge(h, tm)))
          border_halfedges.push_back(h);
        else
          interior_hedges.push_back(h);
      }

      if (border_halfedges.empty())
      {
        // \todo: shall we want to have a way to compute once for all vertices (per loop)
        //        this would avoid recompute face normals
        Vector_3 vn = compute_vertex_normal(v, tm,
                                            parameters::vertex_point_map(vpm)
                                                       .geom_traits(gt));
        Vector_3 move = CGAL::NULL_VECTOR;
        unsigned int star_size = 0;
        for(halfedge_descriptor h :interior_hedges)
        {
          move = move + Vector_3(get(vpm, v), get(vpm, source(h, tm)));
          ++star_size;
        }
        CGAL_assertion(star_size > 0); //isolated vertices have already been discarded
        move = (1. / (double)star_size) * move;

        barycenters.push_back( VNP(v, vn, get(vpm, v) + move) );
      }
      else
      {
        if (!relax_constraints) continue;
        Vector_3 vn(NULL_VECTOR);

        if (border_halfedges.size() == 2)// corners are constrained
        {
          vertex_descriptor ph0 = source(border_halfedges[0], tm);
          vertex_descriptor ph1 = source(border_halfedges[1], tm);
          double dot = to_double(Vector_3(get(vpm, v), get(vpm, ph0))
                                 * Vector_3(get(vpm, v), get(vpm, ph1)));
          // \todo shouldn't it be an input parameter?
          //check squared cosine is < 0.25 (~120 degrees)
          if (0.25 < dot*dot / ( squared_distance(get(vpm,ph0), get(vpm, v)) *
                                 squared_distance(get(vpm,ph1), get(vpm, v))) )
            barycenters.push_back( VNP(v, vn, barycenter(get(vpm, ph0), 0.25, get(vpm, ph1), 0.25, get(vpm, v), 0.5)) );
        }
      }
    }

    // compute moves
    typedef std::pair<vertex_descriptor, Point_3> VP_pair;
    std::vector< std::pair<vertex_descriptor, Point_3> > new_locations;
    new_locations.reserve(barycenters.size());
    for(const VNP& vnp : barycenters)
    {
      vertex_descriptor v = std::get<0>(vnp);
      Point_3 pv = get(vpm, v);
      const Vector_3& nv = std::get<1>(vnp);
      const Point_3& qv = std::get<2>(vnp); //barycenter at v

      new_locations.push_back( std::make_pair(v, qv + (nv * Vector_3(qv, pv)) * nv) );
    }

    // perform moves
    for(const VP_pair& vp : new_locations)
    {
      const Point_3 initial_pos = get(vpm, vp.first);
      const Vector_3 move(initial_pos, vp.second);

      put(vpm, vp.first, vp.second);

      //check that no inversion happened
      double frac = 1.;
      while (frac > 0.03 //5 attempts maximum
         && !check_normals(vp.first)) //if a face has been inverted
      {
        frac = 0.5 * frac;
        put(vpm, vp.first, initial_pos + frac * move);//shorten the move by 2
      }
      if (frac <= 0.02)
        put(vpm, vp.first, initial_pos);//cancel move
    }
  }//end for loop (nit == nb_iterations)

#ifdef CGAL_PMP_TANGENTIAL_RELAXATION_VERBOSE
  std::cout << "\rTangential relaxation : "
    << nb_iterations << " iterations done." << std::endl;
#endif
}

template <typename VertexRange, class TriangleMesh>
void tangential_relaxation(const VertexRange& vertices, TriangleMesh& tm)
{
  tangential_relaxation(vertices, tm, parameters::all_default());
}

/*!
* \ingroup PMP_meshing_grp
* applies `tangential_relaxation()` to all the vertices of `tm`.
*/
template <class TriangleMesh,
          typename CGAL_PMP_NP_TEMPLATE_PARAMETERS>
void tangential_relaxation(TriangleMesh& tm, const CGAL_PMP_NP_CLASS& np)
{
  tangential_relaxation(vertices(tm), tm, np);
}

template <class TriangleMesh>
void tangential_relaxation(TriangleMesh& tm)
{
  tangential_relaxation(vertices(tm), tm, parameters::all_default());
}



} } // CGAL::Polygon_mesh_processing

#endif //CGAL_POLYGON_MESH_PROCESSING_TANGENTIAL_RELAXATION_H
