// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/constants.hpp>
#include <math/vector_3d.hpp>

#include <cmath>

namespace math
{
  struct radians;

  struct degrees
  {
    explicit degrees (float x) : _ (x) {}
    degrees (radians);

    float _;

    using vec3 = vector_3d_base<degrees>;
  };

  struct radians
  {
    explicit radians (float x) : _ (x) {}
    radians (degrees);

    float _;

    using vec3 = vector_3d_base<radians>;
  };

  inline degrees::degrees (radians x) : _ (x._ * 180.0f / math::constants::pi) {}
  inline radians::radians (degrees x) : _ (x._ * math::constants::pi / 180.0f) {}

  inline float sin (radians x)
  {
    return std::sin (x._);
  }
  inline float cos (radians x)
  {
    return std::cos (x._);
  }
  inline float tan (radians x)
  {
    return std::tan (x._);
  }
  inline float acos (radians x)
  {
    return std::acos (x._);
  }
}
