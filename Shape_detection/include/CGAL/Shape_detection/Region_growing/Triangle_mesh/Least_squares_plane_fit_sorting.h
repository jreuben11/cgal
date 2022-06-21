// Copyright (c) 2018 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Florent Lafarge, Simon Giraudot, Thien Hoang, Dmitry Anisimov
//

#ifndef CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H
#define CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H

#include <CGAL/license/Shape_detection.h>

// Internal includes.
#include <CGAL/Shape_detection/Region_growing/internal/property_map.h>

namespace CGAL {
namespace Shape_detection {
namespace Triangle_mesh {

  /*!
    \ingroup PkgShapeDetectionRGOnMesh

    \brief Sorting of triangle mesh faces with respect to the local plane fit quality.

    Indices of faces in a triangle mesh are sorted with respect to the quality of the
    least squares plane fit applied to the vertices of incident faces of each face.

    \tparam GeomTraits
    a model of `Kernel`

    \tparam TriangleMesh
    a model of `FaceListGraph`

    \tparam NeighborQuery
    a model of `NeighborQuery`

    \tparam FaceRange
    a model of `ConstRange` whose iterator type is `RandomAccessIterator` and
    value type is the face type of a triangle mesh

    \tparam VertexToPointMap
    a model of `ReadablePropertyMap` whose key type is the vertex type of a triangle mesh and
    value type is `Kernel::Point_3`
  */
  template<
  typename GeomTraits,
  typename TriangleMesh,
  typename NeighborQuery,
  typename FaceRange = typename TriangleMesh::Face_range,
  typename VertexToPointMap = typename property_map_selector<TriangleMesh, CGAL::vertex_point_t>::const_type>
  class Least_squares_plane_fit_sorting {

  public:
    /// \name Types
    /// @{

    /// \cond SKIP_IN_MANUAL
    using Traits = GeomTraits;
    using Face_graph = TriangleMesh;
    using Neighbor_query = NeighborQuery;
    using Face_range = FaceRange;
    using Vertex_to_point_map = VertexToPointMap;

    using Item = typename boost::graph_traits<TriangleMesh>::face_descriptor;
    using Region = std::vector<Item>;
    using Seed_range = std::vector<Item>;
    /// \endcond

    #ifdef DOXYGEN_NS
      /*!
        a model of `ReadablePropertyMap` whose key and value type is `std::size_t`.
        This map provides an access to the ordered indices of input faces.
      */
      typedef unspecified_type Seed_map;
    #endif

    /// @}

  private:
    using FT = typename Traits::FT;
    using Compare_scores = internal::Compare_scores<FT>;

  public:
    /// \name Initialization
    /// @{

    /*!
      \brief initializes all internal data structures.

      \tparam NamedParameters
      a sequence of \ref bgl_namedparameters "Named Parameters"

      \param pmesh
      an instance of `TriangleMesh` that represents a triangle mesh

      \param neighbor_query
      an instance of `NeighborQuery` that is used internally to
      access face's neighbors

      \param np
      a sequence of \ref bgl_namedparameters "Named Parameters"
      among the ones listed below

      \cgalNamedParamsBegin
        \cgalParamNBegin{vertex_point_map}
          \cgalParamDescription{an instance of `VertexToPointMap` that maps a triangle mesh
          vertex to `Kernel::Point_3`}
          \cgalParamDefault{`boost::get(CGAL::vertex_point, tmesh)`}
        \cgalParamNEnd
        \cgalParamNBegin{geom_traits}
          \cgalParamDescription{an instance of `GeomTraits`}
          \cgalParamDefault{`GeomTraits()`}
        \cgalParamNEnd
      \cgalNamedParamsEnd

      \pre `faces(pmesh).size() > 0`
    */
    template<typename CGAL_NP_TEMPLATE_PARAMETERS>
    Least_squares_plane_fit_sorting(
      const TriangleMesh& tmesh,
      NeighborQuery& neighbor_query,
      const CGAL_NP_CLASS& np = parameters::default_values()) :
    m_face_graph(tmesh),
    m_neighbor_query(neighbor_query),
    m_face_range(faces(m_face_graph)),
    m_vertex_to_point_map(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::vertex_point), get_const_property_map(CGAL::vertex_point, tmesh))),
    m_traits(parameters::choose_parameter(parameters::get_parameter(
      np, internal_np::geom_traits), GeomTraits())) {

      CGAL_precondition(m_face_range.size() > 0);

      m_ordered.resize(m_face_range.size());

      std::size_t index = 0;
      for (auto it = m_face_range.begin(); it != m_face_range.end(); it++)
        m_ordered[index++] = *it;
      m_scores.resize(m_face_range.size());
    }

    /// @}

    /// \name Sorting
    /// @{

    /*!
      \brief sorts indices of input faces.
    */
    void sort() {
      compute_scores();
      CGAL_precondition(m_scores.size() > 0);
      Compare_scores cmp(m_scores);

      std::vector<std::size_t> order(m_face_range.size());
      std::iota(order.begin(), order.end(), 0);
      std::sort(order.begin(), order.end(), cmp);

      std::vector<Item> tmp(m_face_range.size());
      for (std::size_t i = 0; i < m_face_range.size(); i++)
        tmp[i] = m_ordered[order[i]];

      m_ordered.swap(tmp);
    }

    /// @}

    /// \name Access
    /// @{

    /*!
      \brief returns an instance of `Seed_map` to access the ordered indices
      of input faces.
    */
    const Seed_range &ordered() {
      return m_ordered;
    }

    /// @}

  private:
    const Face_graph& m_face_graph;
    Neighbor_query& m_neighbor_query;
    const Face_range m_face_range;
    const Vertex_to_point_map m_vertex_to_point_map;
    const Traits m_traits;
    Seed_range m_ordered;
    std::vector<FT> m_scores;

    void compute_scores() {

      std::vector<Item> neighbors;
      std::size_t idx = 0;
      for (auto it = m_face_range.begin(); it != m_face_range.end(); it++) {
        neighbors.clear();
        m_neighbor_query(*it, neighbors);
        neighbors.push_back(*it);
        m_scores[idx++] = internal::create_plane_from_faces(
          m_face_graph, neighbors, m_vertex_to_point_map, m_traits).second;
      }
    }
  };

} // namespace Triangle_mesh
} // namespace Shape_detection
} // namespace CGAL

#endif // CGAL_SHAPE_DETECTION_REGION_GROWING_TRIANGLE_MESH_LEAST_SQUARES_PLANE_FIT_SORTING_H
