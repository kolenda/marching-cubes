/*
    MarchingCubes - main class for all Marching Cubes computations
    Author: Karol Herda
    Web:    http://kolenda.vipserv.org/algorithmic-marching-cubes/
    Date:   12-03-2013
*/

#include <set>
#include <stdio.h>
#include <string.h>
#include "MarchingCubes.h"

//	tools
inline bool MarchingCubes::_differentSign( int a, int b )
{
    if( a >= 0 && b >= 0 )
        return false;
    if( a < 0 && b < 0 )
        return false;
    return true;
}

//inline
bool MarchingCubes::_sameSign( int a, int b )
{
    return !_differentSign( a, b );
}

int MarchingCubes::_getBitNum( unsigned short number )
{
    unsigned short bits = 0;
    while( number > 0 ) {
        unsigned short mask = ~(1<<bits);
        number &= mask;
        bits++;
    }
    return bits;
}

void MarchingCubes::_codeToSignTable( int code, int* tab )
{
    for( int counter = 0; counter < 8; counter++ ) {
        int i = 1 << counter;
        if( code & i )
            tab[counter] = 1;
        else
            tab[counter] = -1;
    }
}

bool MarchingCubes::_vertexIsAtAxisSide( int v, int axis, int sign )
{
	bool res = false;

	if( sign )
		res = v & (1<<axis);
	else
		res = !(v & (1<<axis));
		//return
	return res;
}

bool MarchingCubes::_vertexIsNegByAxis( int v, int axis )
{
    if( v & (1<<axis) )
        return false;
    return true;
}


bool MarchingCubes::_oneBitDiff( int v1, int v2 )
{
    int diff = v1 ^ v2;    //  XOR
    if( diff == 1 || diff == 2 || diff == 4 )
        return true;
    return false;
}
bool MarchingCubes::_twoBitsDiff( int v1, int v2 )
{
    int diff = v1 ^ v2;    //  XOR
    if( diff == 3 || diff == 5 || diff == 6 )
        return true;
    return false;
}



int MarchingCubes::_findEdge( int v1, int v2 )
{
    for( int edge = 0; edge < 12; edge++ ) {
        if( v1 == edgeToVertex[edge][0] &&
            v2 == edgeToVertex[edge][1] )
           return edge;
        if( v2 == edgeToVertex[edge][0] &&
            v1 == edgeToVertex[edge][1] )
           return edge;
    }
    return -1;
}


MarchingCubes::MarchingCubes( VoxelField& f ) : field(f)
{
    memset( usageStats, 0, sizeof(usageStats) );
}

void MarchingCubes::init()
{
    _fillVertices();
    _fillEdges();
    _fillPlanes();
    generateTriangles();

	int o1 = _cacheOffsetFromPlane( 0,0,0, 1 );
	int o2 = _cacheOffsetFromPlane( 0,0,0, 4 );

    printTable();
}



int MarchingCubes::_bitsToCode( float verts[8] )
{
    int res = 0;
    for( int counter = 0; counter < 8; counter++ ) {
        if( verts[counter] >= 0 )
        {
            int i = 1 << counter;
            res |= i;
        }
    }
    return res;
}

void MarchingCubes::printTable()
{
    printf( "Vertex offset:\n" );
    for( int v = 0; v < 8; v++ ) {
        float* vert = vertexOffset[v].f;
        printf( "i:%d", v );

        for( int comp = 0; comp < 3; comp++ )
            printf( " %.1f", vert[comp] );
        printf( "\n" );
    }
    printf( "\n" );
    printf( "Edge vertex:\n" );
    for( int e = 0; e < 12; e++ ) {
        int* edge = edgeToVertex[e];
        printf( "i:%d edge: %d - %d\n", e, edge[0], edge[1] );
    }

    printf( "\n" );
    printf( "Triangle table:\n" );
    int rowCount = 0;
    int triCount = 0;
    for( int i = 0; i < 256; i++ )
	{
        MarchingCubesCase& cubeCase = triangleTable[i];

        printf( "i:%d t:%d", i, cubeCase.numTri );
        if( cubeCase.numTri > 0 )
            rowCount++;
        for( int tri = 0; tri < cubeCase.numTri; tri++ ) {
            printf( " tri:(" );
            int *v = cubeCase.tris[tri].i;
            for( int tvert = 0; tvert < 3; tvert++ )
                printf( "%d ", v[tvert] );
            printf( ")" );
        }
        printf(" capPlanes:");
		for( int i = 0; i < 6; i++ )
			printf( "%d", cubeCase.capPlanesTab[i] );
        printf("\n");

		triCount += cubeCase.numTri;
    }
    printf( "tri count:%d\n", triCount );
    printf( "case count:%d\n", rowCount );
}



void MarchingCubes::setValues( Cube2& cube )	//float vert[8] )
{
    for( int v = 0; v < 8; v++ )
        vertex[v] = cube.getVec(v);
}

//	TODO: to remove - use cf
void MarchingCubes::setOffsets( float sizex, float sizey, float sizez )
{
	sizeX = sizex;
	sizeY = sizey;
	sizeZ = sizez;

	sizePlane = sizeX * sizeY;
}

MarchingCubes::Vector3F MarchingCubes::getHalfEdge( int edgeNum )
{
    int v1 = edgeToVertex[edgeNum][0];
    int v2 = edgeToVertex[edgeNum][1];

    Vector3F result;

    float v1x = vertexOffset[v1].f[0];
    float v1y = vertexOffset[v1].f[1];
    float v1z = vertexOffset[v1].f[2];
    float v2x = vertexOffset[v2].f[0];
    float v2y = vertexOffset[v2].f[1];
    float v2z = vertexOffset[v2].f[2];

    result.f[0] = (v1x+v2x) / 2;
    result.f[1] = (v1y+v2y) / 2;
    result.f[2] = (v1z+v2z) / 2;

    return result;
}


int MarchingCubes::_getEdgeAxis( int edge )
{
    // Let's take both ends of an edge
    int v1 = edgeToVertex[edge][0];
    int v2 = edgeToVertex[edge][1];
    int diff = v1 ^ v2;     // We do XOR on both vertex indices so we get a bit, where they differ
    // simple if's to translate int value to bit number
    if( diff == 1 )
        return 0;
    if( diff == 2 )
        return 1;
    if( diff == 4 )
        return 2;
    return -1;
}
