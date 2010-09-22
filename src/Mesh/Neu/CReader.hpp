// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Neu_CReader_hpp
#define CF_Mesh_Neu_CReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/fstream.hpp>

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CArray.hpp"

#include "Mesh/Neu/LibNeu.hpp"
#include "Mesh/Neu/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CElements;
  class CRegion;
namespace Neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Neu_API CReader : public CMeshReader, public Shared
{
public: // typedefs

  typedef boost::shared_ptr<CReader> Ptr;
  typedef boost::shared_ptr<CReader const> ConstPtr;

private: // typedefs

  typedef std::pair<boost::shared_ptr<CElements>,Uint> Region_TableIndex_pair;

public: // functions
  /// constructor
  CReader( const CName& name );

  /// Gets the Class name
  static std::string type_name() { return "CReader"; }

  static void defineConfigProperties ( CF::Common::PropertyList& options ) {}

  virtual std::string get_format() { return "Neu"; }

  virtual std::vector<std::string> get_extensions();

private: // functions

  void read_headerData(std::fstream& file);

  void read_coordinates(std::fstream& file);

  void read_connectivity(std::fstream& file);

  void read_groups(std::fstream& file);

  void read_boundaries(std::fstream& file);

  virtual void read_from_to(boost::filesystem::path& fp, const CMesh::Ptr& mesh);

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data

  // map< global index , pair< temporary table, index in temporary table > >
  std::vector<Region_TableIndex_pair> m_global_to_tmp;

  CMesh::Ptr m_mesh;
  CRegion::Ptr m_region;
  CArray::Ptr m_coordinates;
  CRegion::Ptr m_tmp;
  std::string m_file_basename;

  struct HeaderData
  {
    // NUMNP    Total number of nodal points in the mesh
    // NELEM    Total number of elements in the mesh
    // NGPRS    Number of element groups
    // NBSETS   Number of boundary condition sets
    // NDFCD    Number of coordinate directions (2 or 3)
    // NDFVL    Number of velocity components (2 or 3)
    Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
    void print()
    {
      CFinfo << NUMNP << " " << NELEM << " " << NGRPS << " " << NBSETS << " " << NDFCD << " " << NDFVL << CFendl;
    }
  } m_headerData;

  struct GroupData
  {
    // NGP      Element group number
    // NELGP    Number of elements in group
    // MTYP     Material type (0=Undefined, 1=Conjugate, 2=Fluid, 3=Porous, 4=Solid, 5=Deformable)
    // NFLAGS   Number of solver-dependent flags
    // ELMMAT   Identifying name of element group (or entity or zone)
    // ELEM     Vector of element indices
    Uint NGP, NELGP, MTYP, NFLAGS;
    std::string ELMMAT;
    std::vector<Uint> ELEM;
    void print ()
    {
      CFinfo << NGP << " " << NELGP << " " << MTYP << " " << NFLAGS << " " << ELMMAT << CFendl;
    }
  };

  struct BCData
  {
    // NAME     Name of boundary-condition set
    // ITYPE    Data type (0 = node; 1 = element/cell)
    // NENTRY   Number of data records in boundary-condition set
    // NVALUES  Number of values for each data record
    // IBCODE1  (Optional) Boundary condition code 1
    std::string NAME;
    Uint ITYPE, NENTRY, NVALUES, IBCODE1;
    void print ()
    {
      CFinfo << NAME << " " << ITYPE << " " << NENTRY << " " << NVALUES << " " << IBCODE1 << CFendl;
    }
  };

}; // end CReader


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CReader_hpp
