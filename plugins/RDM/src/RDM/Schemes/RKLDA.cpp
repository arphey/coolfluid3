// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"


#include "mesh/CRegion.hpp"

#include "RDM/CellLoop.hpp"
#include "RDM/Schemes/RKLDA.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RKLDA, RDM::CellTerm, LibSchemes > RKLDA_Builder;

////////////////////////////////////////////////////////////////////////////////

RKLDA::RKLDA ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

RKLDA::~RKLDA() {}

void RKLDA::execute()
{

  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(mesh::CRegion::Ptr& region, m_loop_regions)
  {
    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
