//---------------------------------*-CUDA-*----------------------------------//
/*!
 * \file   MC/cuda_rtk/test/RTK_Geometry_Tester.cu
 * \author Tom Evans
 * \date   Fri Feb 03 09:50:55 2017
 * \brief  RTK_Geometry_Tester member and kernel definitions.
 * \note   Copyright (c) 2017 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include <cmath>
#include <curand_kernel.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include "RTK_Geometry_Tester.hh"

#include "CudaUtils/cuda_utils/Utility_Functions.hh"
#include "../RTK_Geometry.cuh"

//---------------------------------------------------------------------------//
// TYPES
//---------------------------------------------------------------------------//

using Core_Geometry         = cuda_profugus::Core;
using Core_Geometry_Manager = cuda_profugus::Core_DMM;
using State                 = Core_Geometry::Geo_State_t;
using Space_Vector          = Core_Geometry::Space_Vector;

//---------------------------------------------------------------------------//
// CORE
//---------------------------------------------------------------------------//

__global__
void heuristic_kernel(
    unsigned long *dseeds,
    curandState_t *rngs,
    Core_Geometry  geometry,
    int           *bins)
{
    // Get the thread id
    auto tid = cuda::utility::thread_id();

    // Initialize the random number states
    curand_init(dseeds[tid], 0, 0, rngs + tid);

    // Number of histories per thread
    int Np = 2000;

    // geometry variables
    double       costheta, sintheta, phi;
    Space_Vector r, omega;
    State        state;

    double two_pi = 6.283185307179586;

    // Sample Np tracks per thread
    for (int n = 0; n < Np; ++n)
    {
        // sample x,y,z randomly
        r[0] = curand_uniform_double(rngs + tid) * 7.56;
        r[1] = curand_uniform_double(rngs + tid) * 7.56;
        r[2] = curand_uniform_double(rngs + tid) * 28.56;

        // sample omega
        costheta = 1.0 - 2.0 * curand_uniform_double(rngs + tid);
        phi      = two_pi * curand_uniform_double(rngs + tid);
        sintheta = std::sqrt(1.0 - costheta * costheta);

        omega[0] = sintheta * cos(phi);
        omega[1] = sintheta * sin(phi);
        omega[2] = costheta;

        // initialize track
        geometry.initialize(r, omega, state);

        while (geometry.boundary_state(state) == profugus::geometry::INSIDE)
        {
            // get distance-to-boundary
            geometry.distance_to_boundary(state);

            // update position of particle and cross the surface
            geometry.move_to_surface(state);
        }

        if (state.escaping_face == State::MINUS_X)
            ++bins[0 + 6 * tid];
        else if (state.escaping_face == State::PLUS_X)
            ++bins[1 + 6 * tid];
        else if (state.escaping_face == State::MINUS_Y)
            ++bins[2 + 6 * tid];
        else if (state.escaping_face == State::PLUS_Y)
            ++bins[3 + 6 * tid];
        else if (state.escaping_face == State::MINUS_Z)
            ++bins[4 + 6 * tid];
        else if (state.escaping_face == State::PLUS_Z)
            ++bins[5 + 6 * tid];
    }
}

//---------------------------------------------------------------------------//

void Core::heuristic()
{
    using std::cout; using std::endl; using std::setw;

    // Build the manager
    Core_Geometry_Manager dmm(*geometry);

    // Get the host object
    auto device_geo = dmm.device_instance();

    // Send over random number seeds and states
    thrust::device_vector<unsigned long> dseeds(seeds.begin(), seeds.end());
    thrust::device_vector<curandState_t> rngs(num_threads);

    // Testing data
    thrust::device_vector<int> bins(num_threads * num_blocks * 6, 0);

    dim3 blocks  = {num_blocks};
    dim3 threads = {num_threads};
    heuristic_kernel<<<blocks,threads>>>(dseeds.data().get(),
                                         rngs.data().get(),
                                         device_geo,
                                         bins.data().get());

    thrust::host_vector<int> rbins(bins.begin(), bins.end());

    // Sum up bin results
    std::vector<double> face_bin(6, 0.0);
    for (int t = 0; t < num_threads * num_blocks; ++t)
    {
        for (int b = 0; b < 6; ++b)
        {
            face_bin[b] += rbins[b + 6 * t];
        }
    }

    double Npx = static_cast<double>(2000 * num_threads * num_blocks);

    EXPECT_EQ(Npx, face_bin[0] + face_bin[1] + face_bin[2] +
              face_bin[3] + face_bin[4] + face_bin[5]);

    double xyf  = 28.56 * 7.56;
    double zf   = 7.56 * 7.56;
    double area = 4 * xyf + 2 * zf;
    double lox  = face_bin[0] / Npx;
    double hix  = face_bin[1] / Npx;
    double loy  = face_bin[2] / Npx;
    double hiy  = face_bin[3] / Npx;
    double loz  = face_bin[4] / Npx;
    double hiz  = face_bin[5] / Npx;

    EXPECT_SOFTEQ(lox, xyf / area, 0.005);
    EXPECT_SOFTEQ(hix, xyf / area, 0.005);
    EXPECT_SOFTEQ(loy, xyf / area, 0.005);
    EXPECT_SOFTEQ(hiy, xyf / area, 0.005);
    EXPECT_SOFTEQ(loz, zf / area,  0.030);
    EXPECT_SOFTEQ(hiz, zf / area,  0.030);

    cout.precision(5);
    cout << endl;
    cout << "Low  X leakage = "
         << setw(8) << lox << " ("
         << setw(8) << xyf / area << ")" << endl;
    cout << "High X leakage = "
         << setw(8) << hix << " ("
         << setw(8) << xyf / area << ")" << endl;
    cout << "Low  Y leakage = "
         << setw(8) << loy << " ("
         << setw(8) << xyf / area << ")" << endl;
    cout << "High Y leakage = "
         << setw(8) << hiy << " ("
         << setw(8) << xyf / area << ")" << endl;
    cout << "Low  Z leakage = "
         << setw(8) << loz << " ("
         << setw(8) << zf / area << ")" << endl;
    cout << "High Z leakage = "
         << setw(8) << hiz << " ("
         << setw(8) << zf / area << ")" << endl;
    cout << endl;
}

//---------------------------------------------------------------------------//
// end of MC/cuda_rtk/test/RTK_Geometry_Tester.cu
//---------------------------------------------------------------------------//
