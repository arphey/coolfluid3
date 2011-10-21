// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "mesh/VTKXML/LibVTKXML.hpp"

namespace cf3 {
namespace mesh {
namespace VTKXML {

cf3::common::RegistLibrary<LibVTKXML> libVTKXML;

////////////////////////////////////////////////////////////////////////////////

void LibVTKXML::initiate_impl()
{
}

void LibVTKXML::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // mesh
} // cf3
