//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_mc_driver/Geometry_Builder.pt.cc
 * \author Steven Hamilton
 * \date   Wed Nov 25 12:58:58 2015
 * \brief  Geometry_Builder member definitions.
 * \note   Copyright (C) 2015 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "Geometry_Builder.t.hh"

#include "cuda_geometry/Mesh_Geometry.hh"

namespace cuda_mc
{

template class Geometry_Builder<cuda_profugus::Mesh_Geometry>;

} // end namespace cuda_mc

//---------------------------------------------------------------------------//
//                 end of Geometry_Builder.pt.cc
//---------------------------------------------------------------------------//
