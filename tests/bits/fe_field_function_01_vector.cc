// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2007 - 2021 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------



// Test the FEFieldFunction class

#include "../tests.h"

// all include files you need here
#include <deal.II/base/function_lib.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria.h>

#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/vector_tools.h>


template <int dim>
class F : public Function<dim>
{
public:
  F()
    : Function<dim>(2)
  {}
  virtual void
  vector_value(const Point<dim> &p, Vector<double> &v) const
  {
    v    = 0;
    v[0] = p.square();
  }
};

double
abs_zero(double a)
{
  if (std::abs(a) < 1e-10)
    return 0;
  else
    return a;
}

template <int dim>
void
test()
{
  Triangulation<dim> tria;
  GridGenerator::hyper_cube(tria);
  tria.refine_global(8 / dim);

  FESystem<dim>   fe(FE_Q<dim>(2), 2);
  DoFHandler<dim> dh(tria);

  dh.distribute_dofs(fe);
  deallog << "Ndofs :" << dh.n_dofs() << std::endl;



  Vector<double> v1(dh.n_dofs()), v2(dh.n_dofs());

  VectorTools::interpolate(dh, F<dim>(), v1);
  deallog << "V norm: " << v1.l2_norm() << std::endl;

  Functions::FEFieldFunction<dim, Vector<double>> fef(dh, v1);


  VectorTools::interpolate(dh, fef, v2);

  v2.add(-1, v1);
  deallog << "Interpolation error: " << abs_zero(v2.l2_norm()) << std::endl;

  Vector<double> error(tria.n_active_cells());
  QGauss<dim>    quad(2);
  VectorTools::integrate_difference(
    dh, v1, F<dim>(), error, quad, VectorTools::L2_norm);
  deallog << "L2 Interpolation error: " << abs_zero(error.l2_norm())
          << std::endl;
  error = 0;
  VectorTools::integrate_difference(
    dh, v1, fef, error, quad, VectorTools::L2_norm);
  deallog << "L2 Interpolation error with fef: " << abs_zero(error.l2_norm())
          << std::endl;
}

int
main()
{
  initlog();

  test<1>();
  test<2>();
  test<3>();

  return 0;
}
