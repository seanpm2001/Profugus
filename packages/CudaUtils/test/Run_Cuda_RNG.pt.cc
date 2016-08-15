//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_utils/test/Cuda_RNG_Kernel.pt.cc
 * \author Stuart Slattery
 * \date   Tue Nov 24 14:09:29 2015
 * \brief
 * \note   Copyright (C) 2013 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "../cuda_utils/Pseudo_Cuda.hh"
#include "../cuda_utils/Definitions.hh"
#include "../cuda_utils/Launch_Args.t.hh"
#include "Run_Cuda_RNG.t.hh"

typedef cuda_utils::arch::Host Host;
template void run_cuda_rng<Host>( cuda_utils::Host_Vector<int>&,
				  cuda_utils::Host_Vector<cuda_utils::Cuda_RNG>&,
				  cuda_utils::Host_Vector<double>& );

//---------------------------------------------------------------------------//
//                        end of Cuda_RNG_Kernel.pt.cc
//---------------------------------------------------------------------------//
