/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2015, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Joseph Mirabel */

#ifndef HPP_FCL_INTERNAL_TOOLS_H
#define HPP_FCL_INTERNAL_TOOLS_H

#include <hpp/fcl/fwd.hh>

#include <cmath>
#include <iostream>
#include <limits>

#include <hpp/fcl/data_types.h>

namespace hpp {
namespace fcl {

template <typename Derived>
static inline typename Derived::Scalar
triple(const Eigen::MatrixBase<Derived> &x, const Eigen::MatrixBase<Derived> &y,
       const Eigen::MatrixBase<Derived> &z) {
  return x.derived().dot(y.derived().cross(z.derived()));
}

template <typename Derived1, typename Derived2, typename Derived3>
void generateCoordinateSystem(const Eigen::MatrixBase<Derived1> &_w,
                              const Eigen::MatrixBase<Derived2> &_u,
                              const Eigen::MatrixBase<Derived3> &_v) {
  typedef typename Derived1::Scalar T;

  Eigen::MatrixBase<Derived1> &w =
      const_cast<Eigen::MatrixBase<Derived1> &>(_w);
  Eigen::MatrixBase<Derived2> &u =
      const_cast<Eigen::MatrixBase<Derived2> &>(_u);
  Eigen::MatrixBase<Derived3> &v =
      const_cast<Eigen::MatrixBase<Derived3> &>(_v);

  T inv_length;
  if (std::abs(w[0]) >= std::abs(w[1])) {
    inv_length = (T)FCL_REAL(1.0) / sqrt(w[0] * w[0] + w[2] * w[2]);
    u[0] = -w[2] * inv_length;
    u[1] = (T)0;
    u[2] = w[0] * inv_length;
    v[0] = w[1] * u[2];
    v[1] = w[2] * u[0] - w[0] * u[2];
    v[2] = -w[1] * u[0];
  } else {
    inv_length = (T)FCL_REAL(1.0) / sqrt(w[1] * w[1] + w[2] * w[2]);
    u[0] = (T)0;
    u[1] = w[2] * inv_length;
    u[2] = -w[1] * inv_length;
    v[0] = w[1] * u[2] - w[2] * u[1];
    v[1] = -w[0] * u[2];
    v[2] = w[0] * u[1];
  }
}

/* ----- Start Matrices ------ */
template <typename Derived, typename OtherDerived>
void relativeTransform(const Eigen::MatrixBase<Derived> &R1,
                       const Eigen::MatrixBase<OtherDerived> &t1,
                       const Eigen::MatrixBase<Derived> &R2,
                       const Eigen::MatrixBase<OtherDerived> &t2,
                       const Eigen::MatrixBase<Derived> &R,
                       const Eigen::MatrixBase<OtherDerived> &t) {
  const_cast<Eigen::MatrixBase<Derived> &>(R) = R1.transpose() * R2;
  const_cast<Eigen::MatrixBase<OtherDerived> &>(t) = R1.transpose() * (t2 - t1);
}

/// @brief compute the eigen vector and eigen vector of a matrix. dout is the
/// eigen values, vout is the eigen vectors
template <typename Derived, typename Vector>
void eigen(const Eigen::MatrixBase<Derived> &m,
           typename Derived::Scalar dout[3], Vector *vout) {
  typedef typename Derived::Scalar Scalar;
  Derived R(m.derived());
  int n = 3;
  int j, iq, ip, i;
  Scalar tresh, theta, tau, t, sm, s, h, g, c;
  int nrot;
  HPP_FCL_UNUSED_VARIABLE(nrot);
  Scalar b[3];
  Scalar z[3];
  Scalar v[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  Scalar d[3];

  for (ip = 0; ip < n; ++ip) {
    b[ip] = d[ip] = R(ip, ip);
    z[ip] = 0;
  }

  nrot = 0;

  for (i = 0; i < 50; ++i) {
    sm = 0;
    for (ip = 0; ip < n; ++ip)
      for (iq = ip + 1; iq < n; ++iq)
        sm += std::abs(R(ip, iq));
    if (sm == FCL_REAL(0.)) {
      vout[0] << v[0][0], v[0][1], v[0][2];
      vout[1] << v[1][0], v[1][1], v[1][2];
      vout[2] << v[2][0], v[2][1], v[2][2];
      dout[0] = d[0];
      dout[1] = d[1];
      dout[2] = d[2];
      return;
    }

    if (i < 3)
      tresh = FCL_REAL(0.2) * sm / (n * n);
    else
      tresh = FCL_REAL(0.);

    for (ip = 0; ip < n; ++ip) {
      for (iq = ip + 1; iq < n; ++iq) {
        g = FCL_REAL(100.0) * std::abs(R(ip, iq));
        if (i > 3 && std::abs(d[ip]) + g == std::abs(d[ip]) &&
            std::abs(d[iq]) + g == std::abs(d[iq]))
          R(ip, iq) = FCL_REAL(0.);
        else if (std::abs(R(ip, iq)) > tresh) {
          h = d[iq] - d[ip];
          if (std::abs(h) + g == std::abs(h))
            t = (R(ip, iq)) / h;
          else {
            theta = FCL_REAL(0.5) * h / (R(ip, iq));
            t = FCL_REAL(1.0) /
                (std::abs(theta) + std::sqrt(FCL_REAL(1.0) + theta * theta));
            if (theta < FCL_REAL(0.))
              t = -t;
          }
          c = FCL_REAL(1.0) / std::sqrt(1 + t * t);
          s = t * c;
          tau = s / (FCL_REAL(1.0) + c);
          h = t * R(ip, iq);
          z[ip] -= h;
          z[iq] += h;
          d[ip] -= h;
          d[iq] += h;
          R(ip, iq) = FCL_REAL(0.);
          for (j = 0; j < ip; ++j) {
            g = R(j, ip);
            h = R(j, iq);
            R(j, ip) = g - s * (h + g * tau);
            R(j, iq) = h + s * (g - h * tau);
          }
          for (j = ip + 1; j < iq; ++j) {
            g = R(ip, j);
            h = R(j, iq);
            R(ip, j) = g - s * (h + g * tau);
            R(j, iq) = h + s * (g - h * tau);
          }
          for (j = iq + 1; j < n; ++j) {
            g = R(ip, j);
            h = R(iq, j);
            R(ip, j) = g - s * (h + g * tau);
            R(iq, j) = h + s * (g - h * tau);
          }
          for (j = 0; j < n; ++j) {
            g = v[j][ip];
            h = v[j][iq];
            v[j][ip] = g - s * (h + g * tau);
            v[j][iq] = h + s * (g - h * tau);
          }
          nrot++;
        }
      }
    }
    for (ip = 0; ip < n; ++ip) {
      b[ip] += z[ip];
      d[ip] = b[ip];
      z[ip] = FCL_REAL(0.);
    }
  }

  std::cerr << "eigen: too many iterations in Jacobi transform." << std::endl;

  return;
}

template <typename Derived, typename OtherDerived>
bool isEqual(const Eigen::MatrixBase<Derived> &lhs,
             const Eigen::MatrixBase<OtherDerived> &rhs,
             const FCL_REAL tol = std::numeric_limits<FCL_REAL>::epsilon() *
                                  100) {
  return ((lhs - rhs).array().abs() < tol).all();
}

} // namespace fcl
} // namespace hpp

#endif
