//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   MonteCarloSolver.hh
 * \author Steven Hamilton
 * \brief  Perform adjoint MC histories to solve linear system.
 */
//---------------------------------------------------------------------------//

#ifndef mc_solver_MonteCarloSolver_hh
#define mc_solver_MonteCarloSolver_hh

#include "Kokkos_View.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_ArrayRCP.hpp"
#include "Teuchos_ArrayView.hpp"

#include "Ifpack2_Preconditioner.hpp"
#include "Ifpack2_Details_CanChangeMatrix.hpp"

#include "AleaSolver.hh"
#include "AleaTypedefs.hh"
#include "MC_Data.hh"
#include "DeviceTraits.hh"

namespace alea
{

//---------------------------------------------------------------------------//
/*!
 * \class MonteCarloSolver
 * \brief Monte Carlo based linear solver.
 *
 * This class solves a linear system of equations using the forward or
 * adjoint Monte Carlo method.  Contributions to tallies will be modified
 * by polynomial coefficients generated by a Polynomial object and the basis
 * matrix used to generate the probability and weight transition matrices
 * can be specified through the MC_Data class.  The actual Monte Carlo kernels
 * are contained in the ForwardMcKernel and AdjointMcKernel classes.
 *
 * A note on the diamond inheritance pattern.  Both Tpetra::Operator
 * (the base class of both AleaSolver and Ifpack2::Preconditioner)
 * and Ifpack2::Preconditioner are pure virtual classes so there should be
 * no risk of ambiguity.  The inheritance from AleaSolver is desirable
 * because it allows this class to be returned directly by the
 * LinearSolverFactory without changing the interface.  Inheritance from
 * Ifpack2::Preconditioner is desirable because it allows this class to be
 * used as a local domain solver an an Additive Schwarz approach.
 */
//---------------------------------------------------------------------------//
class MonteCarloSolver : public AleaSolver,
                         public Ifpack2::Preconditioner<SCALAR,LO,GO,NODE>,
                         public Ifpack2::Details::CanChangeMatrix<MATRIX>
{
  public:

    typedef Kokkos::View<      SCALAR *,DEVICE> view_type;
    typedef Kokkos::View<const SCALAR *,DEVICE> const_view_type;
    typedef Kokkos::View<      LO     *,DEVICE> ord_view;
    typedef Kokkos::View<const LO     *,DEVICE> const_ord_view;

    MonteCarloSolver(Teuchos::RCP<const MATRIX>           A,
                     Teuchos::RCP<Teuchos::ParameterList> pl);

    virtual ~MonteCarloSolver(){}

    // Ifpack2 interface functions

    void setParameters(const Teuchos::ParameterList &list) override
    {
        b_pl->setParameters(list);
    }
    void initialize() override;
    bool isInitialized() const override {return d_initialized;}
    void compute() override { if( !d_initialized ) initialize(); }
    bool isComputed() const override {return d_initialized;}

    magnitude_type computeCondEst(Ifpack2::CondestType CT=Ifpack2::Cheap,
        LO MaxIters=1550,magnitude_type tol=1e-9,
        const Teuchos::Ptr<const MATRIX> &matrix=Teuchos::null) override
    {
        return -1.0;
    }

    magnitude_type getCondEst() const override { return -1.0; }

    Teuchos::RCP<const MATRIX> getMatrix() const override { return b_A; }
    int getNumInitialize() const override { return d_init_count; }
    int getNumCompute() const override { return d_init_count; }
    int getNumApply() const override { return d_apply_count; }
    double getInitializeTime() const override { return 0.0; }
    double getComputeTime() const override { return 0.0; }
    double getApplyTime() const override { return 0.0; }

    // Ifpack2 CanChangeMatrix mixin interface
    void setMatrix(const Teuchos::RCP<const MATRIX> &A) override
    {
        // A can be null, don't check until initialization
        b_A = A;
    }

    // Tpetra::Operator functions that must be redefined due to diamond
    // inheritance

    //! Return domain map of solver (this is range map of operator.)
    virtual Teuchos::RCP<const MAP> getDomainMap() const override
    {
        return b_A->getRangeMap();
    }
    //! Return range map of solver (this is domain map of operator)
    virtual Teuchos::RCP<const MAP> getRangeMap() const override
    {
        return b_A->getDomainMap();
    }

    //! Is transpose apply available.
    virtual bool hasTransposeApply() const override { return false; }

    // Apply
    virtual void apply(const MV &x, MV &y,
                       Teuchos::ETransp mode=Teuchos::NO_TRANS,
                       SCALAR alpha=SCALAR_TRAITS::one(),
                       SCALAR beta=SCALAR_TRAITS::zero()) const override
    {
        // Dispatch to AleaSolver::apply to handle mode,alpha,beta
        AleaSolver::apply(x,y,mode,alpha,beta);
    }

  private:

    // Implementation of apply
    void applyImpl(const MV &x, MV &y) const;

    void convertMatrices(Teuchos::RCP<const MATRIX> H,
                         Teuchos::RCP<const MATRIX> P,
                         Teuchos::RCP<const MATRIX> W);

    Teuchos::RCP<MC_Data> d_mc_data;

    view_type d_H;
    view_type d_P;
    view_type d_W;
    ord_view  d_inds;
    ord_view  d_offsets;
    view_type d_coeffs;

    enum MC_TYPE { FORWARD, ADJOINT };

    MC_TYPE d_type;
    bool    d_use_expected_value;
    GO      d_num_histories;
    SCALAR  d_weight_cutoff;
    SCALAR  d_start_wt_factor;

    int d_num_threads;

    int  d_init_count;
    bool d_initialized;
    mutable int d_apply_count;
};

} // namespace alea

#endif // mc_solver_MonteCarloSolver_hh

