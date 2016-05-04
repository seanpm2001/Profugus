//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_mc/Uniform_Source_Tester.hh
 * \author Steven Hamilton
 * \date   Wed Jan 20 14:57:16 2016
 * \brief  Uniform_Source_Tester class definition.
 * \note   Copyright (C) 2016 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef cuda_mc_Uniform_Source_Tester_hh
#define cuda_mc_Uniform_Source_Tester_hh

//===========================================================================//
/*!
 * \class Uniform_Source_Tester
 * \brief Wrapper to kernel launch to test Uniform_Source class.
 */
//===========================================================================//

class Uniform_Source_Tester
{

  public:

      static void test_source();

      static void test_host_api();
};

#endif // cuda_mc_Uniform_Source_Tester_hh

//---------------------------------------------------------------------------//
//                 end of Uniform_Source_Tester.hh
//---------------------------------------------------------------------------//
