#include <vector>
#include <cmath>
#include "LBFGS.h"
#include "GP.h"
#include "GP_with_constraint.h"

#include "iksolver/intern/IK_QJacobian.h"
#include "iksolver/intern/IK_QTask.h"

using namespace Eigen;
using namespace LBFGSpp;

class VRIKSolver
{
public:
  VectorXd m_x;
  double m_fx;
  
  GP gp;
  GPConstraint gpconstraint;
  
  LBFGSParam<double> param;
  LBFGSSolver<double> solver;

  VRIKSolver() : solver(param) {
  }

};