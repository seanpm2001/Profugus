//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_mc_driver/Manager.pt.cc
 * \author Steven Hamilton
 * \date   Wed Nov 25 11:42:29 2015
 * \brief  Manager template instantiations
 * \note   Copyright (C) 2015 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "Manager.t.hh"
#include "cuda_geometry/Mesh_Geometry.hh"

namespace cuda_mc
{

template class Manager<cuda_profugus::Mesh_Geometry>;

} // end namespace cuda_mc

//---------------------------------------------------------------------------//
//                 end of Manager.pt.cc
//---------------------------------------------------------------------------//
