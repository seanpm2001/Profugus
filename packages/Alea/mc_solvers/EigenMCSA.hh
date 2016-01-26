//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   EigenMCSA.hh
 * \author Massimiliano Lupo Pasini
 * \brief  Perform Monte Carlo Synthetic Acceleration for dominant eigenvalue.
 */
//---------------------------------------------------------------------------//

#ifndef mc_solvers_EigenMCSA_hh
#define mc_solvers_EigenMCSA_hh

#include "AleaSolver.hh"

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"

#include "AleaTypedefs.hh"

namespace alea
{

//---------------------------------------------------------------------------//
/*!
 * \class EigenMCSA
 * \brief Solve eigen problem using power iteration.
 *
 * This class solves an eigenvalue problem of equations using power iterations.
 */
//---------------------------------------------------------------------------//
class EigenMCSA : public AleaSolver
{
  public:

    EigenMCSA(Teuchos::RCP<const MATRIX> A,
                        Teuchos::RCP<Teuchos::ParameterList> pl);

  private:

    // Implementation of apply
    void applyImpl(const MV &x, MV &y) const;

    SCALAR d_divergence_tol;

    Teuchos::RCP<AleaSolver> d_mc_solver;
};

}

#endif // mc_solvers_EigenSequentialMC_hh

