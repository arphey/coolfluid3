// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibMesh_hpp
#define cf3_LibMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

#include "Math/Defs.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Mesh_API
/// @note build system defines COOLFLUID_MESH_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_MESH_EXPORTS
#   define Mesh_API      CF3_EXPORT_API
#   define Mesh_TEMPLATE
#else
#   define Mesh_API      CF3_IMPORT_API
#   define Mesh_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

/// Basic Classes for %Mesh applications used by %COOLFluiD
namespace mesh {

/// Class defines the initialization and termination of the library mesh
///
/// @todo Some new mesh transformers: CScale, CRotate, CTranslate, CMirror, CCopy, CMergeMesh
/// @todo New field functionality - base fields on other fields defined by analytical functions (vectorial functions)
///       and add a update() function that can be used to trigger dependent (linked) fields for update
///
/// @author Tiago Quintino
class Mesh_API LibMesh :  public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibMesh> Ptr;
  typedef boost::shared_ptr<LibMesh const> ConstPtr;

  /// Constructor
  LibMesh ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh"; }


  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Mesh"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the mesh manipulation API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibMesh"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibMesh

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#include "mesh/Tags.hpp"

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibMesh_hpp
