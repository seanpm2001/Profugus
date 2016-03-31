//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_mc/Manager_Cuda.hh
 * \author Steven Hamilton
 * \date   Wed Jun 18 11:21:16 2014
 * \brief  Manager_Cuda class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef cuda_mc_Manager_Cuda_hh
#define cuda_mc_Manager_Cuda_hh

#include <sstream>
#include <string>
#include <memory>

#include "comm/P_Stream.hh"
#include "Box_Shape.cuh"
#include "Fixed_Source_Solver.hh"
//#include "Keff_Solver.hh"
#include "Source_Transporter.hh"
#include "cuda_xs/XS_Device.hh"
#include "cuda_utils/Shared_Device_Ptr.hh"
#include "mc_driver/Manager_Base.hh"

namespace cuda_mc
{

//===========================================================================//
/*!
 * \class Manager_Cuda
 * \brief Class that drives the cuda version of MC miniapp.
 */
//===========================================================================//

template <class Geometry>
class Manager_Cuda : public mc::Manager_Base
{
  private:
    // Typedefs.
    typedef Geometry                                    Geom_t;
    typedef Teuchos::RCP<Teuchos::ParameterList>        RCP_ParameterList;
    typedef Physics<Geom_t>                             Physics_t;
    typedef cuda::Shared_Device_Ptr<Physics_t>          SDP_Physics;
    typedef cuda::Shared_Device_Ptr<Geom_t>             SDP_Geometry;
    typedef cuda_profugus::XS_Device                    XS_Dev;
    typedef cuda::Shared_Device_Ptr<XS_Dev>             SDP_XS_Dev;
    typedef Solver<Geom_t>                              Solver_t;
    typedef std::shared_ptr<Solver_t>                   SP_Solver;
    //typedef cuda_mc::Keff_Solver<Geom_t>                Keff_Solver_t;
    //typedef std::shared_ptr<Keff_Solver_t>              SP_Keff_Solver;
    typedef Fixed_Source_Solver<Geom_t>                 Fixed_Source_Solver_t;
    typedef std::shared_ptr<Fixed_Source_Solver_t>      SP_Fixed_Source_Solver;
    typedef Tallier<Geom_t>                             Tallier_t;
    typedef cuda::Shared_Device_Ptr<Tallier_t>          SDP_Tallier;
    typedef cuda::Shared_Device_Ptr<Box_Shape>          SDP_Shape;
    typedef Source_Transporter<Geom_t>                  Transporter_t;
    typedef std::shared_ptr<Transporter_t>              SP_Transporter;
    //typedef profugus::Fission_Source<Geom_t>            Fission_Source_t;
    //typedef std::shared_ptr<Fission_Source_t>           SP_Fission_Source;
    typedef Teuchos::Array<double>                      Array_Dbl;
    typedef Teuchos::Array<int>                         Array_Int;
    typedef Teuchos::Array<std::string>                 Array_Str;

    // >>> DATA

    // Problem database.
    RCP_ParameterList d_db;

    // Geometry.
    SDP_Geometry d_geometry;

    // Physics.
    SDP_Physics d_physics;

    // On-device xs lib.
    SDP_XS_Dev d_xs_dev;

    // Source shape
    SDP_Shape d_shape;

    // Solvers.
    SP_Solver              d_solver;
    //SP_Keff_Solver         d_keff_solver;
    SP_Fixed_Source_Solver d_fixed_solver;


  public:
    // Constructor.
    Manager_Cuda();

    // Setup the problem.
    void setup(RCP_ParameterList master);

    // Solve the problem.
    void solve();

    // Output.
    void output();

  private:
    // >>> IMPLEMENTATION

    void build_geometry(RCP_ParameterList master_db);
    void build_physics( RCP_ParameterList master_db);

    // Processor.
    int d_node, d_nodes;

    // Problem name.
    std::string d_problem_name;

    //! Output messages in a common format.
#define SCREEN_MSG(stream)                            \
    {                                                 \
        std::ostringstream m;                         \
        m << ">>> " << stream;                        \
        profugus::pcout << m.str() << profugus::endl; \
    }

};

} // end namespace cuda_mc

#endif // cuda_mc_Manager_Cuda_hh

//---------------------------------------------------------------------------//
//                 end of Manager_Cuda.hh
//---------------------------------------------------------------------------//
