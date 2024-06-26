// ------------------------------------------------------------------------
//
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2020 - 2021 by the deal.II authors
//
// This file is part of the deal.II library.
//
// Part of the source code is dual licensed under Apache-2.0 WITH
// LLVM-exception OR LGPL-2.1-or-later. Detailed license information
// governing the source code and code contributions can be found in
// LICENSE.md and CONTRIBUTING.md at the top level directory of deal.II.
//
// ------------------------------------------------------------------------


// Show positions of quadrature points with various degrees of MappingQ
// and quadrature formulas, including the collocation case where quadrature
// points coincide with the mapping support points and going to the tensor
// product and non-tensor product path of MappingQ

#include <deal.II/base/quadrature_lib.h>

#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/mapping_q.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/tria.h>

#include "../tests.h"


template <int dim>
void
test(const unsigned int degree, const unsigned int n_q_points)
{
  MappingQ<dim>      mapping(degree);
  FE_Nothing<dim>    dummy;
  QGaussLobatto<dim> quadrature(n_q_points);
  Quadrature<dim>    quadrature_copy(quadrature.get_points());

  Triangulation<dim> tria;
  GridGenerator::hyper_ball(tria);

  deallog << "Checking " << dim << "D with mapping degree " << degree
          << " with " << n_q_points << " points per coordinate direction"
          << std::endl;

  // for QGaussLobatto, MappingQ will choose the tensor product code
  // path, whereas for the copy it will not as we do not know the tensor
  // product property on general points
  FEValues<dim> fe_val(mapping, dummy, quadrature, update_quadrature_points);
  FEValues<dim> fe_val_copy(mapping,
                            dummy,
                            quadrature_copy,
                            update_quadrature_points);

  for (const auto &cell : tria.active_cell_iterators())
    {
      fe_val.reinit(cell);
      fe_val_copy.reinit(cell);

      for (unsigned int q = 0; q < quadrature.size(); ++q)
        {
          deallog << "GL: " << fe_val.quadrature_point(q)
                  << "  copy: " << fe_val_copy.quadrature_point(q)
                  << "  difference: "
                  << fe_val.quadrature_point(q) -
                       fe_val_copy.quadrature_point(q)
                  << std::endl;
        }
      deallog << std::endl;
    }
}



int
main()
{
  initlog();
  deallog << std::setprecision(8) << std::fixed;

  for (unsigned int degree = 1; degree < 5; ++degree)
    {
      test<2>(degree, degree + 1);
      test<2>(degree, degree + 2);
    }

  for (unsigned int degree = 1; degree < 4; ++degree)
    test<3>(degree, degree + 1);

  return 0;
}
