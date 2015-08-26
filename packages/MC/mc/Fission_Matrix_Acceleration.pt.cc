//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mc/Fission_Matrix_Acceleration.pt.cc
 * \author Thomas M. Evans
 * \date   Wed Nov 12 14:54:34 2014
 * \brief  Fission_Matrix_Acceleration member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "solvers/LinAlgTypedefs.hh"
#include "Fission_Matrix_Acceleration.t.hh"

namespace profugus
{

template class Fission_Matrix_Acceleration_Impl<EpetraTypes>;
template class Fission_Matrix_Acceleration_Impl<TpetraTypes>;

} // end namespace profugus

//---------------------------------------------------------------------------//
//                 end of Fission_Matrix_Acceleration.pt.cc
//---------------------------------------------------------------------------//
