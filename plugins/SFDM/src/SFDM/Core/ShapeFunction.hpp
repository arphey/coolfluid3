// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_Core_ShapeFunction_hpp
#define CF_SFDM_Core_ShapeFunction_hpp

#include "Common/BoostArray.hpp"

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/Types.hpp"

#include "SFDM/Core/LibCore.hpp"

namespace CF {
namespace SFDM {
namespace Core {

/// @class Spectral Finite Difference shape function base class
/// SFD shape functions are comprised of 1D shape functions, in every direction of the
/// element dimensionality. The total shape function is then the tensorial product of these
/// 1D shape functions.
/// Therefore the only possible SFD element types are Lines (1D), Quadrilaterals(2D), Hexahedrals(3D)
class SFDM_Core_API ShapeFunction  : public Mesh::ShapeFunction {
public:

  typedef const boost::multi_array<Uint,3>& Points;
  typedef const boost::const_subarray_gen< boost::multi_array<Uint,3> ,2>::type Lines;
  typedef const boost::const_subarray_gen< boost::multi_array<Uint,3> ,1>::type LinePoints;

  /// Constructor
  ShapeFunction(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "ShapeFunction"; }

  /// @brief Get the index corresponding to an orientation, a line, and a index on that line
  /// points()[orientation][line_idx][line_point_idx]
  /// @param orientation     orientation of the line (KSI / ETA / ZTA)
  /// @param line_idx        index of the line following the given orientation
  /// @param line_point_idx  index inside the line
  /// Lines      points()[orientation]           for a view of the orientation
  /// LinePoints points()[orientation][line_idx] for a view of one line
  Points points() { return m_points; }

  /// @brief Get the index corresponding to an orientation, a line, and a index on that line
  /// points()[orientation][line_idx][side]
  /// @param orientation  orientation of the line (KSI / ETA / ZTA)
  /// @param line_idx     index of the line following the given orientation
  /// @param side         left or right side of the line (LEFT / RIGHT)
  /// Lines      points()[orientation]           for a view of the orientation
  /// LinePoints points()[orientation][line_idx] for a view of one line
  Points face_points() { return m_face_points; }

protected:

  /// lookup table for the function point
  boost::multi_array<Uint,3> m_points;

  /// lookup table for the function face_point
  boost::multi_array<Uint,3> m_face_points;

};

} // Core
} // SFDM
} // CF

#endif // CF_SFDM_Core_ShapeFunction_hpp