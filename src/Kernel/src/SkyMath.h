#ifndef SKYMATH_H
#define SKYMATH_H

#include <QtGlobal>

namespace SkyMath {

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template <typename T> T normalise180(T y0, T y1)
{
    T y1n;
    T s;

    s = sgn(y0);
    if (sgn(y1) != s) {
        y1n = y1;
        if ((y1 - y0) > T(180)) {
            y1n = s * (T(360) - qAbs(y1));
        } else {
            y1n = y1;
        }
    } else {
        y1n = y1;
    }

    return y1n;
}

/*!
 * Interpolates circular values in a range of [-180, 180[ using Hermite
 * (cubic) interpolation.
 *
 *  Tension: 1 is high, 0 normal, -1 is low
 *  Bias: 0 is even,
 *        positive is towards first segment,
 *        negative towards the other
 *
 * Also refer to: http://paulbourke.net/miscellaneous/interpolation/
 */
template <typename T> T interpolateHermite180(
   T y0, T y1,
   T y2, T y3,
   T mu,
   T tension = T(0),
   T bias = T(0))
{
    T y0n, y1n, y2n, y3n;
    T m0, m1, mu2, mu3;
    T a0, a1, a2, a3;

    // Normalise sample points y0, y1, y2 and y3
    y0n = y0;
    y1n = normalise180(y0, y1);
    y2n = normalise180(y1, y2);
    y3n = normalise180(y2, y3);

    mu2 = mu * mu;
    mu3 = mu2 * mu;
    m0  = (y1n - y0n) * (1 + bias) * (1 - tension) / 2;
    m0 += (y2n - y1n) * (1 - bias) * (1 - tension) / 2;
    m1  = (y2n - y1n) * (1 + bias) * (1 - tension) / 2;
    m1 += (y3n - y2n) * (1 - bias) * (1 - tension) / 2;

    a0 =  2 * mu3 - 3 * mu2 + 1;
    a1 =      mu3 - 2 * mu2 + mu;
    a2 =      mu3 -     mu2;
    a3 = -2 * mu3 + 3 * mu2;

   T v = (a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
   if (v < - T(180)) {
       v += T(360);
   } else if (v > T(180)) {
       v -= T(360);
   }
   return v;
}

} // namespace

#endif // SKYMATH_H
