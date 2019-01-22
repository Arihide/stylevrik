#include <vector>
#include <cmath>
#include "LBFGS/LBFGS.h"
#include "GP.h"
#include "GPIK.h"

// #include "iksolver/IK_QJacobian.h"
// #include "iksolver/IK_QTask.h"

using namespace Eigen;
using namespace LBFGSpp;

class VRIKSolver
{
public:
  VectorXd m_x;
  double m_fx;
  
  GP gp;
  GPIK gpik;
  
  LBFGSParam<double> param;
  LBFGSSolver<double> solver;

  VRIKSolver() : solver(param) {
  }

};