//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_utils/test/Run_Launch_Args.t.hh
 * \author Steven Hamilton
 * \date   Tue Nov 24 14:09:29 2015
 * \brief
 * \note   Copyright (C) 2013 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef cuda_utils_test_Run_Launch_Args_t_hh
#define cuda_utils_test_Run_Launch_Args_t_hh

#include "Run_Launch_Args.hh"

#include "Launch_Args_Kernel.hh"
#include "../cuda_utils/Launch_Args.hh"
#include "../cuda_utils/Host_Vector.hh"

//---------------------------------------------------------------------------//
// Launch_Args test functor.
template<typename Arch>
void run_launch_args(std::vector<double> &data)
{
    // Make launch args.
    cuda::Launch_Args<Arch> launch_args;
    launch_args.grid_size = 4;
    launch_args.block_size = 256;

    // Create a functor.
    double value = 2.3;
    int size = launch_args.block_size * launch_args.grid_size;
    Functor<Arch> functor( size, value );

    // Call the kernel launch.
    cuda::parallel_launch( functor, launch_args );

    // Get the data from the functor.
    cuda::Host_Vector<double> host_vec(size);
    functor.assign_data(host_vec);

    // Copy data into function argument
    data.clear();
    data.insert( data.end(), host_vec.begin(), host_vec.end() );
}

#endif // cuda_utils_test_Run_Launch_Args_t_hh

//---------------------------------------------------------------------------//
//                        end of Run_Launch_Args.t.hh
//---------------------------------------------------------------------------//
