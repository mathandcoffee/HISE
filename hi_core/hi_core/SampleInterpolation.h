/*  ===========================================================================
*   This is where the interpolation modes will live
*	this will be shared
*   ===========================================================================
*/

#pragma once

enum SampleInterpolation
{
    NearestNeighbor = 0,
    Linear,
    SNESGaussian,
    Cubic,
    PS1Gaussian,
    numInterpolationModes
};