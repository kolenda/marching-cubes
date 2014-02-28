/*
    MarchingCubes - main class for all Marching Cubes computations
    Author: Karol Herda
    Web:    http://kolenda.vipserv.org/algorithmic-marching-cubes/
    Date:   12-03-2013
*/

#include <set>
#include <map>
#include <stdio.h>
#include <string.h>
#include "MarchingCubes.h"


int MarchingCubes::fillInTriangles( MarchingCubes::TriangleF tris[8] )
{
    MarchingCubesCase &cubeCase = getCaseFromValues();
    usageStats[cubeCase.index]++;

    int triNum = 0;
    for( ; triNum < cubeCase.numTri; triNum++ ) {
        int e1 = cubeCase.tris[triNum][0];
        int e2 = cubeCase.tris[triNum][1];
        int e3 = cubeCase.tris[triNum][2];
        Vector3F vec1 = getVertexFromEdge( e1 );
        Vector3F vec2 = getVertexFromEdge( e2 );
        Vector3F vec3 = getVertexFromEdge( e3 );

        tris[triNum].v[0].pos.f[0] = vec1.f[0];
        tris[triNum].v[0].pos.f[1] = vec1.f[1];
        tris[triNum].v[0].pos.f[2] = vec1.f[2];

        tris[triNum].v[1].pos.f[0] = vec2.f[0];
        tris[triNum].v[1].pos.f[1] = vec2.f[1];
        tris[triNum].v[1].pos.f[2] = vec2.f[2];

        tris[triNum].v[2].pos.f[0] = vec3.f[0];
        tris[triNum].v[2].pos.f[1] = vec3.f[1];
        tris[triNum].v[2].pos.f[2] = vec3.f[2];

        Vector3F  delta1 = vec2 - vec1;
        Vector3F  delta2 = vec3 - vec1;

        Vector3F  normal;
        getCrossProduct( delta1.f, delta2.f, normal.f );
        tris[triNum].v[0].norm = normal;
        tris[triNum].v[1].norm = normal;
        tris[triNum].v[2].norm = normal;
    }
    return triNum;
}


int MarchingCubes::fillInTrianglesIndexed( MarchingCubes::Vertex* vert, int maxVert, MarchingCubes::TriangleI* tris, int maxTris, int& vertexNum, int& triNum )
{
	std::map<int,int>	capPlaneCache;

	_cacheAlloc( field.getSizeX(), field.getSizeY(), field.getSizeZ() );
	_cacheClear();

    currTriangle = 0;
    currVert    = 0;

    for( int x = 0; x < field.getSizeX()-1; x++ )
    for( int y = 0; y < field.getSizeY()-1; y++ )
    for( int z = 0; z < field.getSizeZ()-1; z++ )
    {
    	if( x == 3 && y == 6 && z == 6 ) {
			int x = 5;
    	}

        Cube2 cube = field.getCube( x, y, z );
		cube.setGridSize( field.getSizeX(), field.getSizeY(), field.getSizeZ() );

        if( cube.notEmpty() ) {
            if( currTriangle < maxTris - 25 ) {
                TriangleF     tmpTris[25];
                setValues( cube );

                MarchingCubesCase &cubeCase = getCaseFromValues();
						usageStats[cubeCase.index]++;

                int triNum = 0;
                // for each triangle
                for( ; triNum < cubeCase.numTri; triNum++ ) {
					// get triangle edges
					// get cache indexes for all 3 edges
					// if we had cache initialized - use this value
                    int e1 = cubeCase.tris[triNum][0];
                    int e2 = cubeCase.tris[triNum][1];
                    int e3 = cubeCase.tris[triNum][2];

                    int index1 = _cacheVertex( vert, x,y,z, e1 );
                    int index2 = _cacheVertex( vert, x,y,z, e2 );
                    int index3 = _cacheVertex( vert, x,y,z, e3 );

					// get 3 resulting vertices
                    Vector3F vec1 = vert[index1].pos;
                    Vector3F vec2 = vert[index2].pos;
                    Vector3F vec3 = vert[index3].pos;

                    Vector3F  delta1 = vec2 - vec1;
                    Vector3F  delta2 = vec3 - vec1;

					//	compute face normal
                    Vector3F  normal;
                    getCrossProduct( delta1.f, delta2.f, normal.f );
//                    normal.normalise();

					// add normal to the cache
                    vert[index1].norm += normal;
                    vert[index2].norm += normal;
                    vert[index3].norm += normal;

                    tris[currTriangle].i[0] = index1;
                    tris[currTriangle].i[1] = index2;
                    tris[currTriangle].i[2] = index3;
                    currTriangle++;
                }

				if( cubeCase.capPlanes ) {
					for( int plane = 0; plane < 6; plane++ ) {
						int p = cubeCase.capPlanesTab[plane];
						if( p ) {

							int offset = _cacheOffsetFromPlane( x, y, z, plane );
							if( capPlaneCache.find(offset) != capPlaneCache.end() ) {
								int p2 = capPlaneCache.find(offset)->second;
								capPlaneCache.erase(offset);

								if( p2 != p ) {
//											printf( "x:%d y:%d z:%d p: %d, p2: %d\n", x,y,z, p,p2 );
									if( p == 1 )
										_capPlane( vert, tris, x,y,z, plane, 0 );
									else if( p == 2 )
										_capPlane( vert, tris, x,y,z, plane, 1 );
								}
							}
							else {
								capPlaneCache.insert( std::pair<int,int>(offset, p) );
							}
						}
					}
				}
			}	// cur tri
		}
    }	//	for

	int	lenVector[10] = {0};

    for( int v = 0; v < currVert; v++ ) {
            if( vert[v].norm.length() < 0.5 ) {
                int x = 5;
            }
            if( vert[v].used < 10 ) {
				lenVector[ vert[v].used ] ++;
            }
        vert[v].norm.normalise();
    }

    vertexNum = currVert;
    triNum = currTriangle;

static  int ii = 0;	if( !ii ) {	ii ++;	printf( "currTriangle:%d currVert:%d", currTriangle, currVert );}

    return currTriangle;
}


int MarchingCubes::_capPlane( MarchingCubes::Vertex* vert, MarchingCubes::TriangleI* tris,
								int x, int y, int z, int plane, int side )
{
	int* edges = planeToEdge[plane];

	// TODO: just dirty hack for now
	if( plane == 2 || plane == 3 )
		side = 1 - side;

	int index[4];	// = {-1};
	for( int i = 0; i < 4; i++ )
		index[i] = _cacheVertex( vert, x,y,z, edges[i] );

	Vector3F vec1 = vert[ index[0] ].pos;
	Vector3F vec2 = vert[ index[1] ].pos;
	Vector3F vec3 = vert[ index[2] ].pos;
	Vector3F vec4 = vert[ index[3] ].pos;

//	Vector3F  delta_1 = vec2 - vec1;
//	Vector3F  delta_2 = vec3 - vec1;

	Vector3F  delta1 = vec2 - vec1;
	Vector3F  delta2 = vec3 - vec1;

	Vector3F  delta21 = vec4 - vec2;
	Vector3F  delta22 = vec4 - vec3;

	//	compute face normal
	Vector3F  normal, normal2;

	if( side == 0 ) {
		getCrossProduct( delta1.f, delta2.f, normal.f );
		getCrossProduct( delta21.f, delta22.f, normal2.f );
	}
	else if( side == 1 ) {
		getCrossProduct( delta2.f, delta1.f, normal.f );
		getCrossProduct( delta22.f, delta21.f, normal2.f );
	}
//	normal.normalise();
//	normal2.normalise();

	//*	// add normal to the cache

//	Vector3F normalSum = normal + normal2;
//    for( int ii = 0; ii < 4; ii++ )
//		vert[ index[ii] ].norm += normalSum;

	vert[ index[0] ].norm += normal;
	vert[ index[1] ].norm += normal;
	vert[ index[2] ].norm += normal;

	vert[ index[1] ].norm += normal2;
	vert[ index[2] ].norm += normal2;
	vert[ index[3] ].norm += normal2;


	if( side == 0 ) {
		tris[currTriangle].i[0] = index[0];
		tris[currTriangle].i[1] = index[2];
		tris[currTriangle].i[2] = index[1];
		currTriangle++;

		tris[currTriangle].i[0] = index[1];
		tris[currTriangle].i[1] = index[2];
		tris[currTriangle].i[2] = index[3];
		currTriangle++;
	}
	else if( side == 1 ) {
		tris[currTriangle].i[0] = index[0];
		tris[currTriangle].i[1] = index[1];
		tris[currTriangle].i[2] = index[2];
		currTriangle++;

		tris[currTriangle].i[0] = index[1];
		tris[currTriangle].i[1] = index[3];
		tris[currTriangle].i[2] = index[2];
		currTriangle++;
	}
}
