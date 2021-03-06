// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CriterionMilestoneTime_hpp
#define cf3_solver_CriterionMilestoneTime_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Criterion.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// CriterionMilestoneTime
/// @author Willem Deconinck
class solver_API CriterionMilestoneTime : public Criterion {

public: // functions

  /// Contructor
  /// @param name of the component
  CriterionMilestoneTime ( const std::string& name );

  /// Virtual destructor
  virtual ~CriterionMilestoneTime();

  /// Get the class name
  static std::string type_name () { return "CriterionMilestoneTime"; }

  /// Simulates this model
  virtual bool operator()();

private:

  Handle<Time> m_time;

  Real m_tolerance;

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CriterionMilestoneTime_hpp
