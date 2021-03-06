/*
    MarchingCubes - main class for all Marching Cubes computations
    Author: Karol Herda
    Web:    http://kolenda.vipserv.org/algorithmic-marching-cubes/
    Date:   12-03-2013
*/

//#include <set>
#include <stdio.h>
#include <string.h>
#include "MarchingCubes.h"


// if there's no allocated vertex in given position - create one
int MarchingCubes::_cacheVertex( MarchingCubes::Vertex* vert, int x, int y, int z, int e )
{
	int res = -1;

	int cache1 = _cacheOffsetFromCubeEdge( x,y,z, e );
	if( cacheField[cache1] >= 0 ) {
		res = cacheField[cache1];
						vert[res].used++;
	}
	else {
		// allocate new vertex in vertex table
		cacheField[cache1] = currentVertex;
		Vector3F vertPos = getVertexFromEdge( e );
		vertPos.f[0] += x;
		vertPos.f[1] += y;
		vertPos.f[2] += z;
		vert[currentVertex].pos = vertPos;
		vert[currentVertex].norm.setValue( 0.0f, 0.0f, 0.0f );
		vert[currentVertex].used = 0;
		res = currentVertex++;
	}
	return res;
}

int MarchingCubes::_cacheOffsetFromCubeEdge( int x, int y, int z, int e )
{
//	printf( "---_cacheOffsetFromCubeEdge: x,y,z, e: %d,%d,%d, %d", x,y,z, e );
	// if the edge has 'smaller' reflection, take the smaller one and move to the next cube
    if( _getEdgeBySymmetry(e,0) < e ) {
        x++;
        e = _getEdgeBySymmetry(e,0);
    }
    if( _getEdgeBySymmetry(e,1) < e ) {
        y++;
        e = _getEdgeBySymmetry(e,1);
    }
    if( _getEdgeBySymmetry(e,2) < e ) {
        z++;
        e = _getEdgeBySymmetry(e,2);
    }

    int res = x + y * cacheSizeX + z*cacheSizeX*cacheSizeY;
    res = (res * 4) + e;

    return res;
}

int MarchingCubes::_cacheOffsetFromPlane( int x, int y, int z, int plane )
{
	int axis = _planeToAxis( plane );
	int sign = _planeToSign( plane );

	if( sign > 0 ) {
		if( axis == 0 )
			x++;
		else if( axis == 1 )
			y++;
		else if( axis == 2 )
			z++;
		else
			throw "_cacheOffsetFromPlane error!";

		plane = _planeFromAxisSign( axis, 0 );
	}
//	int offset = _cacheOffsetFromCubeEdge( x, y, z, plane );
/*	bool mirrored = false;
	if( plane == 5 ) {
		mirrored = true;
		plane--;
		z++;
	}
	else if( plane == 3 ) {
		mirrored = true;
		plane--;
		y++;
	}
	else if( plane == 1 ) {
		mirrored = true;
		plane--;
		x++;
	}*/

    int res = x + y * cacheSizeX + z*cacheSizeX*cacheSizeY;
    res = (res * 6) + plane;
    return res;
}

int* MarchingCubes::_cacheAlloc( int fieldX, int fieldY, int fieldZ )
{
    _cacheFree();

	// get number of bits needed to store the x,y,z value
    int x = _getBitNum( fieldX );
    int y = _getBitNum( fieldY );
    int z = _getBitNum( fieldZ );

	// 2^numBits
	// size of the field aligned to 2^n
    cacheSizeX = 1 << (x);
    cacheSizeY = 1 << (y);
    cacheSizeZ = 1 << (z);

	// for each of the x,y,z position we store 4 int's
	// 		3 of them corresponds to 3 edges going forward from this position
	//		4'th is not used, added just for alignment
    cacheSize = cacheSizeX*cacheSizeY*cacheSizeZ * 4;
    cacheField = new int[ cacheSize ];

    return cacheField;
}

void MarchingCubes::_cacheFree()
{
    if( cacheField ) {
        delete[] cacheField;
        cacheField = NULL;
    }
	cacheSizeX = 0;
	cacheSizeY = 0;
	cacheSizeZ = 0;
	cacheSize = 0;
}

void MarchingCubes::_cacheClear()
{
    if( cacheField ) {
        for( int i = 0; i < cacheSize; i++ )
            cacheField[i] = -1;
    }
}
