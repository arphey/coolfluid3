// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#define BOOST_HASH_NO_IMPLICIT_CASTS
#include <boost/functional/hash.hpp>

#include <boost/static_assert.hpp>
#include <set>

#include "common/Log.hpp"
#include "common/CBuilder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StreamHelpers.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionArray.hpp"
#include "common/CreateComponentDataType.hpp"
#include "common/OptionT.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Actions/CGlobalNumberingNodes.hpp"
#include "mesh/CCellFaces.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/CFaceCellConnectivity.hpp"
#include "mesh/CNodeElementConnectivity.hpp"
#include "mesh/CNodeFaceCellConnectivity.hpp"
#include "mesh/CCells.hpp"
#include "mesh/CSpace.hpp"
#include "mesh/CMesh.hpp"
#include "Math/Functions.hpp"
#include "Math/Consts.hpp"
#include "mesh/ElementData.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {

  using namespace common;
  using namespace Math::Functions;
  using namespace Math::Consts;

  create_component_data_type( std::vector<std::size_t> , Mesh_Actions_API , CVector_size_t , "CVector<size_t>" );

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CGlobalNumberingNodes, CMeshTransformer, LibActions> CGlobalNumberingNodes_Builder;

//////////////////////////////////////////////////////////////////////////////

CGlobalNumberingNodes::CGlobalNumberingNodes( const std::string& name )
: CMeshTransformer(name),
  m_debug(false)
{

  m_properties["brief"] = std::string("Construct global node and element numbering based on coordinates hash values");
  std::string desc;
  desc =
    "  Usage: CGlobalNumberingNodes Regions:array[uri]=region1,region2\n\n";
  m_properties["description"] = desc;

  m_options.add_option<OptionT<bool> >("debug", m_debug)
      ->description("Perform checks on validity")
      ->pretty_name("Debug")
      ->link_to(&m_debug);

  m_options.add_option<OptionT<bool> >("combined", true)
      ->description("Combine nodes and elements in one global numbering")
      ->pretty_name("Combined");
}

/////////////////////////////////////////////////////////////////////////////

std::string CGlobalNumberingNodes::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CGlobalNumberingNodes::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" + properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CGlobalNumberingNodes::execute()
{
  CMesh& mesh = *m_mesh.lock();

  CTable<Real>& coordinates = mesh.geometry().coordinates();

  if ( is_null( mesh.geometry().get_child_ptr("glb_node_hash") ) )
    mesh.geometry().create_component<CVector_size_t>("glb_node_hash");

  CVector_size_t& glb_node_hash =
      mesh.geometry().get_child("glb_node_hash").as_type<CVector_size_t>();

  glb_node_hash.data().resize(coordinates.size());


  Uint i(0);
  boost_foreach(CTable<Real>::ConstRow coords, coordinates.array() )
  {
    glb_node_hash.data()[i]=hash_value(to_vector(coords));
    if (m_debug)
      std::cout << "["<<PE::Comm::instance().rank() << "]  hashing node ("<< to_vector(coords).transpose() << ") to " << glb_node_hash.data()[i] << std::endl;
    ++i;
  }


  // In debug mode, check if no hashes are duplicated
  if (m_debug)
  {
    std::set<std::size_t> glb_set;
    for (Uint i=0; i<glb_node_hash.data().size(); ++i)
    {
      if (glb_set.insert(glb_node_hash.data()[i]).second == false)  // it was already in the set
        throw ValueExists(FromHere(), "node "+to_str(i)+" is duplicated");
    }
  }


  // now renumber

  //------------------------------------------------------------------------------
  // create node_glb2loc mapping
  std::map<std::size_t,Uint> node_glb2loc;
  Uint loc_node_idx(0);
  boost_foreach(std::size_t hash, glb_node_hash.data())
    node_glb2loc[hash]=loc_node_idx++;
  std::map<std::size_t,Uint>::iterator node_glb2loc_it;
  std::map<std::size_t,Uint>::iterator hash_not_found = node_glb2loc.end();


  //------------------------------------------------------------------------------
  // get tot nb of owned indexes and communicate

  Uint nb_ghost(0);
  Geometry& nodes = mesh.geometry();
  CList<Uint>& nodes_rank = mesh.geometry().rank();
  nodes_rank.resize(nodes.size());
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if (nodes.is_ghost(i))
      ++nb_ghost;
    else
      nodes_rank[i] = PE::Comm::instance().rank();
  }


  Uint tot_nb_owned_ids=nodes.size()-nb_ghost;
  if (m_debug) std::cout << "["<<PE::Comm::instance().rank()<<"] nodes owned: " << tot_nb_owned_ids << std::endl;
  if (m_debug) std::cout << "["<<PE::Comm::instance().rank()<<"] nb ghost: " << nb_ghost << std::endl;


  std::vector<Uint> nb_ids_per_proc(PE::Comm::instance().size());

  // avoid mpi call if PE not active
  if( PE::Comm::instance().is_active() )
    PE::Comm::instance().all_gather(tot_nb_owned_ids, nb_ids_per_proc);
  else
    nb_ids_per_proc[0] = tot_nb_owned_ids;

  std::vector<Uint> start_id_per_proc(PE::Comm::instance().size());

  Uint start_id=0;
  for (Uint p=0; p<nb_ids_per_proc.size(); ++p)
  {
    start_id_per_proc[p] = start_id;
    start_id += nb_ids_per_proc[p];
  }


  //------------------------------------------------------------------------------
  // add glb_idx to owned nodes, broadcast/receive glb_idx for ghost nodes

  std::vector<size_t> node_from(nodes.size()-nb_ghost);
  std::vector<Uint>   node_to(nodes.size()-nb_ghost);

  CList<Uint>& nodes_glb_idx = mesh.geometry().glb_idx();
  nodes_glb_idx.resize(nodes.size());

  Uint cnt=0;
  Uint glb_id = start_id_per_proc[PE::Comm::instance().rank()];
  for (Uint i=0; i<nodes.size(); ++i)
  {
    if ( ! nodes.is_ghost(i) )
    {
      nodes_glb_idx[i] = glb_id++;
      node_from[cnt] = glb_node_hash.data()[i];
      node_to[cnt]   = nodes_glb_idx[i];
      ++cnt;
    }
  }

  for (Uint root=0; root<PE::Comm::instance().size(); ++root)
  {
    //std::vector<std::size_t> rcv_node_from = MPI::broadcast(node_from, root);
    //std::vector<Uint>        rcv_node_to   = MPI::broadcast(node_to, root);
    // PECheckPoint(100,"001");
    std::vector<std::size_t> rcv_node_from(0);//node_from.size());
    PE::Comm::instance().broadcast(node_from,rcv_node_from,root);
    //PECheckPoint(100,"002");
    std::vector<Uint>        rcv_node_to(0);//node_to.size());
    PE::Comm::instance().broadcast(node_to,rcv_node_to,root);
    //PECheckPoint(100,"003");
    if (PE::Comm::instance().rank() != root)
    {
      for (Uint p=0; p<PE::Comm::instance().size(); ++p)
      {
        if (p == PE::Comm::instance().rank())
        {
          Uint rcv_idx(0);
          boost_foreach(const std::size_t node_hash, rcv_node_from)
          {
            node_glb2loc_it = node_glb2loc.find(node_hash);
            if ( node_glb2loc_it != hash_not_found )
            {
              if (m_debug)
                std::cout << "["<<PE::Comm::instance().rank() << "]  will change node "<< node_glb2loc_it->first << " (" << node_glb2loc_it->second << ") to " << rcv_node_to[rcv_idx] << std::endl;
              nodes_glb_idx[node_glb2loc_it->second]=rcv_node_to[rcv_idx];
              nodes_rank[node_glb2loc_it->second]=root;
            }
            ++rcv_idx;
          }
          break;
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

std::size_t CGlobalNumberingNodes::hash_value(const RealVector& coords)
{
  std::size_t seed=0;
  for (Uint i=0; i<coords.size(); ++i)
    boost::hash_combine(seed,coords[i]);
  return seed;
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // mesh
} // cf3
