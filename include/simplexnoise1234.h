// SimplexNoise1234
// Copyright © 2003-2005, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
		\brief Header for "simplexnoise1234.c" for producing Perlin simplex noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin Simplex noise function.
 * It is a stand-alone compilation unit with no external dependencies,
 * highly reusable without source code modifications.
 *
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. Having both versions could be useful.
 */

/** 1D, 2D, 3D and 4D float Perlin noise
 */
    float snoise1( float x );
    float snoise2( float x, float y );
    float snoise3( float x, float y, float z );
    float snoise4( float x, float y, float z, float w );

/** 1D, 2D, 3D and 4D float Perlin noise, with a specified integer period
 */
    float psnoise1( float x, int px );
    float psnoise2( float x, float y, int px, int py );
    float psnoise3( float x, float y, float z, int px, int py, int pz );
    float psnoise4( float x, float y, float z, float w,
                              int px, int py, int pz, int pw );
