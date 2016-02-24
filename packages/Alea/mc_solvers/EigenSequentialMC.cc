//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   EigenSequentialMC.cc
 * \author Massimiliano Lupo Pasni
 * \brief  Perform Sequential Monte Carlo on eigenvalue system
 */
//---------------------------------------------------------------------------//

#include <iterator>
#include <string>

#include "EigenSequentialMC.hh"
#include "EigenSolverFactory.hh"
#include "harness/DBC.hh"

namespace alea
{

//---------------------------------------------------------------------------//
/*!
 * \brief Constructor
 *
 * \param A  Problem matrix
 * \param pl ParameterList
 *
 * Behavior is controlled by following PL entries on the nested "power iteration"
 * sublist:
 *  - max_iterations(int)    : >0 (1000)
 *  - tolerance(SCALAR)      : >0.0 (1.0e-6)
 *  - verbosity(string)      : "none", ("low"), "medium", "high"
 */
//---------------------------------------------------------------------------//
EigenSequentialMC::EigenSequentialMC(
        Teuchos::RCP<const MATRIX> A,
        Teuchos::RCP<Teuchos::ParameterList> pl )
  : AleaSolver(A,pl)
{
    // Get belos pl
    Teuchos::RCP<Teuchos::ParameterList> power_pl =
        Teuchos::sublist(pl,"PowerMethod");

    d_divergence_tol = power_pl->get("divergence_tolerance",1.0e4);

    // Override default parameters if present on sublist
    this->setParameters(power_pl);

    b_label = "SequentialEigenMC";

    Teuchos::RCP<AleaSolver> alea_solver = EigenSolverFactory::buildSolver( "monte_carlo", A, pl );
    d_mc_solver = Teuchos::rcp_static_cast<MonteCarloEigenSolver>(alea_solver);

}

//---------------------------------------------------------------------------//
// PRIVATE MEMBER FUNCTIONS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
/*!
 * \brief Perform Richardson iteration.
 */
//---------------------------------------------------------------------------//
void EigenSequentialMC::applyImpl(const MV &x, MV &y) const
{
    // For now we only support operating on a single vector
    REQUIRE( x.getNumVectors() == 1 );

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 2);

    SCALAR eig_old = dis(gen);
    SCALAR eig_new = 0.0;

    // Compute initial estimation of the eigenvector
    MV r(y.getMap(),1);
    r.update(1.0,x,0.0);
    y.update(1.0,x,0.0);
    b_num_iters = 0;

    Teuchos::ArrayRCP<SCALAR> r_data = r.getDataNonConst(0);
    Teuchos::ArrayRCP<SCALAR> y_data = y.getDataNonConst(0);

    Teuchos::ArrayRCP<MAGNITUDE> y_norm(1);
    y.norm2(y_norm());
    unsigned int N = y_data.size();

    for(unsigned int i = 0; i!=N; ++i)
	y_data[i] /= y_norm[0];
 
    //Copy initial guess in a new auxiliary vector
    MV ig(y.getMap(),1);
    ig.update(1.0,y,0.0);

    SCALAR old_res_norm = 1e+6;

    while( true )
    {
        b_num_iters++;

        d_mc_solver->apply(ig,y);
	
	//normalization of the previous approximatin of the eigenvector
        y.norm2(y_norm());

    	for(unsigned int i = 0; i!=N; ++i)
		y_data[i] /= y_norm[0];

    	ig.update(1.0,y,0.0);
        eig_new = 0.0;

        b_A->apply(y,r);
        for(unsigned int i = 0; i!=N; ++i)
		eig_new += r_data[i]*y_data[i];

	SCALAR res_norm = 0.0;

        for(unsigned int i = 0; i!=N; ++i)
		res_norm += (r_data[i] - eig_new * y_data[i]) * (r_data[i] - eig_new * y_data[i]);

	res_norm = std::sqrt(res_norm);
	
	SCALAR rel_res_norm = res_norm / std::abs(eig_new);

        if( b_verbosity >= HIGH )
        {
            std::cout << "Approximated eigenvalue at Sequential MC iteration " << b_num_iters << " is " << eig_new << std::endl;
	    std::cout<<"relative residual norm: "<< std::setprecision(15) <<rel_res_norm<<std::endl;
	}

        // Check for convergence
        if( rel_res_norm < b_tolerance )
        {
            if( b_verbosity >= LOW )
            {
                std::cout << "Sequential converged after "
                    << b_num_iters << " iterations." << std::endl;
            }
            break;
        }

        // Check for max iterations
        if( b_num_iters >= b_max_iterations )
        {
            if( b_verbosity >= LOW )
            {
                std::cout << "Sequential reached maximum iteration "
                    << "count with relative residual of "
                    << rel_res_norm << std::endl;
            }
            b_num_iters = -1;
            break;
        }

        // Check for divergence
        if( rel_res_norm > d_divergence_tol)
        {
            if( b_verbosity >= LOW )
            {
                std::cout << "Sequential diverged after "
                    << b_num_iters << " iterations." << std::endl;
            }
            b_num_iters = -b_num_iters;
            break;
        }
        
	if( d_mc_solver->refinement() )	
		d_mc_solver->refineStandardDeviation(0.8);
	
        eig_old = eig_new;
	old_res_norm = res_norm;
    }

}

} // namespace alea

