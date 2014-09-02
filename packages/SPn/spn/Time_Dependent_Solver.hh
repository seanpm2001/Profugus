//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   spn/Time_Dependent_Solver.hh
 * \author Thomas M. Evans
 * \date   Fri Apr 04 00:09:50 2014
 * \brief  Time_Dependent_Solver class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef spn_Time_Dependent_Solver_hh
#define spn_Time_Dependent_Solver_hh

#include "comm/Timer.hh"
#include "solvers/StratimikosSolver.hh"
#include "Solver_Base.hh"

namespace profugus
{

//===========================================================================//
/*!
 * \class Time_Dependent_Solver
 * \brief Solve a time-dependent SPN problem.
 *
 * This is just a stub for future time-dependent development.
 */
/*!
 * \example spn/test/tstTime_Dependent_Solver.cc
 *
 * Test of Time_Dependent_Solver.
 */
//===========================================================================//

class Time_Dependent_Solver : public Solver_Base
{
  public:
    //@{
    //! Typedefs.
    typedef Solver_Base                      Base;
    typedef Linear_System_t::External_Source External_Source;
    typedef Linear_System_t::RCP_Timestep    RCP_Timestep;
    typedef Epetra_MultiVector               MV;
    typedef Epetra_Operator                  OP;
    typedef StratimikosSolver<MV,OP>         Linear_Solver_t;
    //@}

  private:
    // >>> DATA

    // Solver.
    Linear_Solver_t d_solver;

    // Solution vector.
    RCP_Vector d_lhs;

    // Timestep controller.
    RCP_Timestep d_dt;

  public:
    // Constructor.
    explicit Time_Dependent_Solver(RCP_ParameterList db);

    // Set up the solver.
    void setup(RCP_Dimensions dim, RCP_Mat_DB mat, RCP_Mesh mesh,
               RCP_Indexer indexer, RCP_Global_Data data);

    // Solve the SPN equations.
    void solve(const External_Source &q);

    // Write the scalar-flux into the state.
    void write_state(State_t &state);

    // >>> ACCESSORS

    //! Get LHS solution vector (in transformed \e u space).
    const Vector_t& get_LHS() const { return *d_lhs; }

  private:
    // >>> IMPLEMENTATION

    // Timer.
    profugus::Timer d_timer;
};

} // end namespace profugus

#endif // spn_Time_Dependent_Solver_hh

//---------------------------------------------------------------------------//
//                 end of Time_Dependent_Solver.hh
//---------------------------------------------------------------------------//
