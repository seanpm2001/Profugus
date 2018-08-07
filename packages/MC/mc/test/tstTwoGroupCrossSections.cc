//---------------------------------*-C++-*-----------------------------------//
/*!
 * \file   MC/mc/test/tstTwoGroupCrossSections.cc
 * \author Steven Hamilton
 * \date   Tue Aug 07 10:06:57 2018
 * \brief  Tests for class TwoGroupCrossSections.
 * \note   Copyright (c) 2018 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "../TwoGroupCrossSections.hh"

#include "Utils/gtest/utils_gtest.hh"

using mc::TwoGroupCrossSections;

//---------------------------------------------------------------------------//
// Test fixture
//---------------------------------------------------------------------------//
class TwoGroupCrossSectionsTest : public ::testing::Test
{
  protected:
    // >>> TYPEDEFS
    using XS = TwoGroupCrossSections;

  protected:
    void SetUp()
    {
    }

  protected:
    // >>> DATA
    XS xs_calc;
};

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

TEST_F(TwoGroupCrossSectionsTest, basic)
{
    //
    // At reference temp/density
    //

    auto data = xs_calc.get_data(XS::FUEL, 1000.0, 0.75);
    EXPECT_SOFT_EQ(0.6109,  data.diffusion[0]);
    EXPECT_SOFT_EQ(0.2164,  data.diffusion[1]);
    EXPECT_SOFT_EQ(0.01607, data.absorption[0]);
    EXPECT_SOFT_EQ(0.3116,  data.absorption[1]);
    EXPECT_SOFT_EQ(0.01172, data.scatter);
    EXPECT_SOFT_EQ(0.01191, data.nu_fission[0]);
    EXPECT_SOFT_EQ(0.5007,  data.nu_fission[1]);

    data = xs_calc.get_data(XS::GUIDE, 1000.0, 0.75);
    EXPECT_SOFT_EQ(0.4916,    data.diffusion[0]);
    EXPECT_SOFT_EQ(0.1718,    data.diffusion[1]);
    EXPECT_SOFT_EQ(0.0005049, data.absorption[0]);
    EXPECT_SOFT_EQ(0.01996,   data.absorption[1]);
    EXPECT_SOFT_EQ(0.03082,   data.scatter);
    EXPECT_SOFT_EQ(0.0,       data.nu_fission[0]);
    EXPECT_SOFT_EQ(0.0,       data.nu_fission[1]);

    // At elevated temp, reduced density
    data = xs_calc.get_data(XS::FUEL, 1200.0, 0.70);
    EXPECT_SOFT_EQ(0.6109,     data.diffusion[0]);
    EXPECT_SOFT_EQ(0.2164,     data.diffusion[1]);
    EXPECT_SOFT_EQ(0.01586313, data.absorption[0]);
    EXPECT_SOFT_EQ(0.3107921,  data.absorption[1]);
    EXPECT_SOFT_EQ(0.01055124, data.scatter);
    EXPECT_SOFT_EQ(0.01191,    data.nu_fission[0]);
    EXPECT_SOFT_EQ(0.5007,     data.nu_fission[1]);

    data = xs_calc.get_data(XS::GUIDE, 1200.0, 0.70);
    EXPECT_SOFT_EQ(0.4916,    data.diffusion[0]);
    EXPECT_SOFT_EQ(0.1718,    data.diffusion[1]);
    EXPECT_SOFT_EQ(4.6913e-4, data.absorption[0]);
    EXPECT_SOFT_EQ(0.018597,  data.absorption[1]);
    EXPECT_SOFT_EQ(0.028609,  data.scatter);
    EXPECT_SOFT_EQ(0.0,       data.nu_fission[0]);
    EXPECT_SOFT_EQ(0.0,       data.nu_fission[1]);
}

//---------------------------------------------------------------------------//
// end of MC/mc/test/tstTwoGroupCrossSections.cc
//---------------------------------------------------------------------------//
