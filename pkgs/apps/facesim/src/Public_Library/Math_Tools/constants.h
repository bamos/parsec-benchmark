//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __constants__
#define __constants__

#include <math.h>
namespace PhysBAM
{

const double pi = 4 * atan (1.);
const double two_pi = 2 * pi;
const double two_thirds_pi = 2. / 3 * pi;
const double four_thirds_pi = 4. / 3 * pi;
const double one_over_four_pi = .25 / pi;

const double one_third = 1. / 3;
const double two_thirds = 2. / 3;
const double five_thirds = 5. / 3;
const double one_sixth = 1. / 6;
const double one_ninth = 1. / 9;
const double one_twelfth = 1. / 12;
const double one_twenty_seventh = 1. / 27;
const double thirteen_over_twelve = 13. / 12;
const double root_two = sqrt (2.);
const double root_three = sqrt (3.);
const double root_six = sqrt (6.);
const double root_two_thirds = sqrt (2. / 3);
const double one_over_root_two = 1. / sqrt (2.);

const double speed_of_light = 2.99792458e8; // m/s
const double plancks_constant = 6.6260755e-34; // J*s
const double boltzmanns_constant = 1.380658e-23; // J/K

}
#endif

