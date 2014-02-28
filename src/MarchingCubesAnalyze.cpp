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

int _findCommonAxis( int v1, int v2 )
{
    if( v1&1 == v2&1 )
        return 0;
    if( v1&2 == v2&2 )
        return 1;
    if( v1&4 == v2&4 )
        return 2;
    return -1;
}


void MarchingCubes::_fillVertices()
{
    for( int v = 0; v < 8; v++ ) {
        for( int compound = 0; compound < 3; compound++ )
            vertexOffset[v].f[compound] = (v&(1<<compound)) ? 1.0f : 0.0f;
    }
}

void MarchingCubes::_fillEdges()
{
    int edgesNum = 0;
    for( int v1 = 0; v1 < 7; v1++ ){
        for( int v2 = v1+1; v2 < 8; v2++ ){
            if( _oneBitDiff(v1,v2) ) {  //one bit difference
                edgeToVertex[edgesNum][0] = v1;
                edgeToVertex[edgesNum][1] = v2;
                edgesNum++;
            }
        }
    }
}

void MarchingCubes::_fillPlanes()
{
	for( int axis = 0; axis < 3; axis++ )
	{
		for( int sign = 0; sign < 2; sign++ )
		{
			int plane = axis * 2 + sign;
			int edgeCounter = 0;
			int vertexCounter = 0;

			for( int edge = 0; edge < 12; edge++ )
			{
				int v1 = edgeToVertex[edge][0];
				int v2 = edgeToVertex[edge][1];
				if( _vertexIsAtAxisSide( v1, axis, sign )
					&& _vertexIsAtAxisSide( v2, axis, sign ) )
							planeToEdge[plane][edgeCounter++] = edge;
			}
			assert( edgeCounter == 4 );

			for( int vert = 0; vert < 8; vert++ ) {
				if( _vertexIsAtAxisSide( vert, axis, sign ) )
					planeToVertex[plane][vertexCounter++] = vert;

			}
			assert( vertexCounter == 4 );
		}
	}
}


int MarchingCubes::generateTriangles()
{
    triangleTable[0].index = triangleTable[0].numTri = 0;
    triangleTable[255].index = 255;
    triangleTable[255].numTri = 0;

    for( int i = 1; i < 255; i++ ) {
        if( triangleTable[i].index < 1 )
		{
            triangleTable[i].index = i;

            int tris = 0;
            tris += _findSingleVertexTriangles( i );
            tris += _findEdgeTriangles( i );
            tris += _findHalfSplit( i );
            tris += _findTripleVertex( i );
            tris += _findFourVertex( i );
            tris += _findSnake( i );

			_selectCapPlanes( i );

            int fixed = _fixTrianglesNormals( i );
        }
    }
    return 1;
}


int MarchingCubes::_findSingleVertexTriangles( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    bool singleVertexMap[8] = {false};
    for( int v = 0; v < 8; v++ )
	{
        int vRefl[3];
        for( int ax = 0; ax < 3; ax++ )
            vRefl[ax] = _getVertexBySymmetry( v, ax );

        if( signTab[v] >= 0 &&
            signTab[vRefl[0]] < 0 &&
            signTab[vRefl[1]] < 0 &&
            signTab[vRefl[2]] < 0 )
		{
			MarchingCubesCase& cubeCase = triangleTable[code];
			cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7-v );
			cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v, vRefl[0] );
			cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v, vRefl[1] );
			cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v, vRefl[2] );

			singleVertexMap[v] = true;
			cubeCase.numTri++;
			counter++;
		}
	}
//*
    for( int v = 0; v < 8; v++ )
	{
        int vRefl[3];
        for( int ax = 0; ax < 3; ax++ )
            vRefl[ax] = _getVertexBySymmetry( v, ax );

        if( signTab[v] < 0 &&
            signTab[vRefl[0]] >= 0 &&
            signTab[vRefl[1]] >= 0 &&
            signTab[vRefl[2]] >= 0 )
		{
			if( singleVertexMap[ vRefl[0] ] ||
				singleVertexMap[ vRefl[1] ] ||
				singleVertexMap[ vRefl[2] ] )
				;
			else
			{
				MarchingCubesCase& cubeCase = triangleTable[code];
				cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
				cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v, vRefl[0] );
				cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v, vRefl[1] );
				cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v, vRefl[2] );

				singleVertexMap[v] = true;
				cubeCase.numTri++;
				counter++;

				for( int i = 0; i < v; i++ ) {
					if( singleVertexMap[i] && _twoBitsDiff(v,i) ) {
	;//					_capSingleVertexPlane( triangleTable[code], i, v );
					}
				}
			}
		}//*/
    }
    // ambigious case
    for( int i = 0; i < 7; i++ )
    for( int j = i+1; j < 8; j++ )
    {
//        if( singleVertexMap[i] && singleVertexMap[j] ) {
            if( _twoBitsDiff(i,j) ) {
                int axis = _findCommonAxis(i,j);

;//				_capSingleVertexPlane( triangleTable[code], i, j );
            }
//        }
    }
    return counter;
}

int MarchingCubes::_findEdgeTriangles( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int e = 0; e < 12; e++ ) {

        bool failed = false;
        int v1 = edgeToVertex[e][0];
        int v2 = edgeToVertex[e][1];
        int edgeAxis = _getEdgeAxis(e);

        if( signTab[v1] != signTab[v2] )
            failed = true;
        else {
            for( int i = 1; i < 3; i++ ) {
                int ax = (edgeAxis+i)%3;
                int v1Refl = _getVertexBySymmetry( v1, ax );
                int v2Refl = _getVertexBySymmetry( v2, ax );

                if( _sameSign( signTab[v1],signTab[v1Refl]) ||
                    _sameSign( signTab[v2],signTab[v2Refl]) ) {
                        failed = true;
                        break;
                }
            }
        }
        if( !failed ) {
            MarchingCubesCase& cubeCase = triangleTable[code];
            if( signTab[v1] < 0 )
//                cubeCase.normal = _getNormalFromBits(v1);
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits(v1);
            else
//                cubeCase.normal = _getNormalFromBits(7-v1);
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits(7-v1);

            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v1, _getVertexBySymmetry(v1,(edgeAxis+1)%3) );
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v1, _getVertexBySymmetry(v1,(edgeAxis+2)%3) );
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v2, _getVertexBySymmetry(v2,(edgeAxis+2)%3) );
            cubeCase.numTri++;

            if( signTab[v1] < 0 )
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits(v1);
            else
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits(7-v1);

            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v2, _getVertexBySymmetry(v2,(edgeAxis+2)%3) );
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v2, _getVertexBySymmetry(v2,(edgeAxis+1)%3) );
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v1, _getVertexBySymmetry(v1,(edgeAxis+1)%3) );
            cubeCase.numTri++;

            counter+=2;
        }
    }
    return counter;
}

int MarchingCubes::_findHalfSplit( int code )
{
    int counter = 0;

    int signTab[8];
    _codeToSignTable( code, signTab );
    // signTab - signs for all corners

    for( int axis = 0; axis < 3; axis++ ) {
        bool casePosFail = false;
        bool caseNegFail = false;

        for( int v = 0; v < 8; v++ ) {

            if( _vertexIsNegByAxis(v,axis) ) {
                if( signTab[v] < 0 ) {
                    caseNegFail = true;
                    if( casePosFail && caseNegFail )
                        break;
                }
                else {  //v.sign > 0
                    casePosFail = true;
                    if( casePosFail && caseNegFail )
                        break;
                }
            }
            else {  // v->isPos
                if( signTab[v] < 0 ) {
                    casePosFail = true;
                    if( casePosFail && caseNegFail )
                        break;
                }
                else {  //v.sign > 0
                    caseNegFail = true;
                    if( casePosFail && caseNegFail )
                        break;
                }
            }
        }
        // end for;
        if( !casePosFail ) {
            //axis = 0,1,2
            int edges[4];
            //  int     edgeToVertex[12][2];
            _getEdgesAlongAxis( axis, edges );

            int startEdge = edges[0];
            int edge1       = _getEdgeBySymmetry( startEdge, (axis+1)%3 );
            int edge2       = _getEdgeBySymmetry( startEdge, (axis+2)%3 );
            int endEdge     = _getEdgeBySymmetry( edge2, (axis+1)%3 );

            MarchingCubesCase& cubeCase = triangleTable[code];
//            cubeCase.normal = _getNormalFromBits( 0 );
            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 0 );
            cubeCase.tris[ cubeCase.numTri ][0] = startEdge;
            cubeCase.tris[ cubeCase.numTri ][1] = edge1;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;

            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 0 );
            cubeCase.tris[ cubeCase.numTri ][0] = edge1;
            cubeCase.tris[ cubeCase.numTri ][1] = endEdge;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;

            counter+=2;
        }
        if( !caseNegFail ) {
            //axis = 0,1,2
            int edges[4];
            _getEdgesAlongAxis( axis, edges );

            int startEdge = edges[0];
            int edge1       = _getEdgeBySymmetry( startEdge, (axis+1)%3 );
            int edge2       = _getEdgeBySymmetry( startEdge, (axis+2)%3 );
            int endEdge     = _getEdgeBySymmetry( edge2, (axis+1)%3 );

            MarchingCubesCase& cubeCase = triangleTable[code];
            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7 );
            cubeCase.tris[ cubeCase.numTri ][0] = startEdge;
            cubeCase.tris[ cubeCase.numTri ][1] = edge1;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;

            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7 );
            cubeCase.tris[ cubeCase.numTri ][0] = edge1;
            cubeCase.tris[ cubeCase.numTri ][1] = endEdge;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;

            counter+=2;
        }
    }
    return counter;
}

int MarchingCubes::_findTripleVertex( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int v = 0; v < 8; v++ ) {
        for( int axis = 0; axis < 3; axis++ ) {
            int ax1 = (axis+1)%3;
            int ax2 = (axis+2)%3;

            int vRefl1 = _getVertexBySymmetry( v, ax1 );
            int vRefl2 = _getVertexBySymmetry( v, ax2 );
            int vRefl12 = _getVertexBySymmetry( vRefl1, ax2 );

            int vReflA = _getVertexBySymmetry( v, axis );
            int vRefl1A = _getVertexBySymmetry( vRefl1, axis );
            int vRefl2A = _getVertexBySymmetry( vRefl2, axis );

            if( signTab[v] < 0 &&
                signTab[vRefl1] < 0 &&
                signTab[vRefl2] < 0
                &&
                signTab[vReflA] > 0 &&
                signTab[vRefl1A] > 0 &&
                signTab[vRefl2A] > 0 &&
                signTab[vRefl12] > 0 )
            {
                MarchingCubesCase& cubeCase = triangleTable[code];
//                cubeCase.normal = _getNormalFromBits( v );
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl1A);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl1, vRefl12);
                cubeCase.numTri++;

                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl2A);
                cubeCase.numTri++;

                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(v, vReflA);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl1, vRefl1A);
                cubeCase.numTri++;
                counter+=3;
                return counter;
            }
            else if( signTab[v] > 0 &&
                    signTab[vRefl1] > 0 &&
                    signTab[vRefl2] > 0
                    &&
                    signTab[vReflA] < 0 &&
                    signTab[vRefl1A] < 0 &&
                    signTab[vRefl2A] < 0 &&
                    signTab[vRefl12] < 0 )
            {
                MarchingCubesCase& cubeCase = triangleTable[code];
//                cubeCase.normal = _getNormalFromBits( 7-v );
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7-v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl1, vRefl1A);
                cubeCase.numTri++;

                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7-v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
                cubeCase.numTri++;

                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( 7-v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl1A);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(v, vReflA);
                cubeCase.numTri++;
                counter+=3;
                return counter;
            }
        }
    }
    return counter;
}

int MarchingCubes::_findFourVertex( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int v = 0; v < 8; v++ ) {
        int vRefl1 = _getVertexBySymmetry( v, 0 );
        int vRefl2 = _getVertexBySymmetry( v, 1 );
        int vRefl3 = _getVertexBySymmetry( v, 2 );
        int vRefl12 = _getVertexBySymmetry( vRefl1, 1 );
        int vRefl13 = _getVertexBySymmetry( vRefl1, 2 );
        int vRefl23 = _getVertexBySymmetry( vRefl2, 2 );
        int vRefl123 = _getVertexBySymmetry( vRefl12, 2 );

        if( signTab[v] < 0 &&
            signTab[vRefl1] < 0 &&
            signTab[vRefl2] < 0 &&
            signTab[vRefl3] < 0
            &&
            signTab[vRefl12] > 0 &&
            signTab[vRefl13] > 0 &&
            signTab[vRefl23] > 0 &&
            signTab[vRefl123] > 0 )
        {
            MarchingCubesCase& cubeCase = triangleTable[code];
            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl12);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
            cubeCase.numTri++;

            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl3, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
            cubeCase.numTri++;

            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl3, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl12);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl23);
            cubeCase.numTri++;

            cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( v );
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl3, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl23);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl3, vRefl23);
            cubeCase.numTri++;
            counter+=4;
        }
    }
    return counter;
}

int MarchingCubes::_findSnake( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int v = 0; v < 8; v++ ) {
        for( int ax1 = 0; ax1 < 3; ax1++ ) {
            for( int ax2_ = ax1+1; ax2_ < ax1+3; ax2_++ ) {
                int ax2 = ax2_ % 3;
                int ax3 = 3 - (ax1+ax2);

                int vRefl_1 = _getVertexBySymmetry( v, ax1 );
                int vRefl_12 = _getVertexBySymmetry( vRefl_1, ax2 );
                int vRefl_123 = _getVertexBySymmetry( vRefl_12, ax3 );

                std::set<int> vertPath;
                vertPath.insert( v );
                vertPath.insert( vRefl_1 );
                vertPath.insert( vRefl_12 );
                vertPath.insert( vRefl_123 );

                bool fail = false;
                for( int i = 0; i < 8; i++ ) {
                    // not found
                    if( vertPath.find(i) == vertPath.end() &&
                        signTab[i] < 0 ) {
                        fail = true;
                        break;
                    }
                    // found
                    if( vertPath.find(i) != vertPath.end() &&
                        signTab[i] > 0 ) {
                        fail = true;
                        break;
                    }
                }
                if( !fail ) {
                    int vRefl_2 = _getVertexBySymmetry( v, ax2 );
                    int vRefl_3 = _getVertexBySymmetry( v, ax3 );
                    int vRefl_13 = _getVertexBySymmetry( vRefl_1, ax3 );
                    int vRefl_23 = _getVertexBySymmetry( vRefl_2, ax3 );

                    MarchingCubesCase& cubeCase = triangleTable[code];

                    cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( vRefl_1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(v, vRefl_3);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl_1, vRefl_13);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl_123, vRefl_23);
                    cubeCase.numTri++;

                    cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( vRefl_1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(v, vRefl_2);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(v, vRefl_3);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl_123, vRefl_23);
                    cubeCase.numTri++;

                    cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( vRefl_1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl_2, vRefl_12);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(v, vRefl_2);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl_123, vRefl_23);
                    cubeCase.numTri++;

                    cubeCase.normal[cubeCase.numTri] = _getNormalFromBits( vRefl_1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl_1, vRefl_13);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl_13, vRefl_123);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl_123, vRefl_23);
                    cubeCase.numTri++;

                    counter += 4;
                    return counter;
                }
            }
        }
    }
    return counter;
}


int MarchingCubes::_selectCapPlanes( int code )
{
    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

	for( int axis = 0; axis < 3; axis++ )
	{
		for( int sign = 0; sign < 2; sign++ )
		{
			int plane = axis * 2 + sign;
			triangleTable[code].capPlanesTab[plane] = 0;

			int* planeEdges = planeToEdge[plane];

bool side = false;
			int edgesOnPlane = 0;
			MarchingCubesCase& cubeCase = triangleTable[code];
			// for each triangle
			for( int tri = 0; tri < cubeCase.numTri; tri++ ) {
				// check each combination: tri corner and edge on this plane
				for( int i = 0; i < 4; i++ )
				for( int j = 0; j < 3; j++ )
					if( cubeCase.tris[tri].i[j] == planeEdges[i] )
						edgesOnPlane++;

{
int c = 0;
for( int i = 0; i < 3; i++ )
	if( cubeCase.tris[tri].i[i] == planeEdges[0] ||
		cubeCase.tris[tri].i[i] == planeEdges[2] )
						c++;
			//}
if( c > 1 )
	side = true;
}
//			if( edgesOnPlane )
//				printf( "cap plane -> case:%d plane:%d edgesOnPlane:%d\n", code, plane, edgesOnPlane );

			if( edgesOnPlane == 4 ) {
				if( side )
					triangleTable[code].capPlanesTab[plane] = 2;
				else
					triangleTable[code].capPlanesTab[plane] = 1;

				//triangleTable[code].capPlanesTab[plane] = 2;
				printf( "cap plane (4) -> case:%d plane:%d\n", code, plane );
			}
		}
	}
	}
	for( int i = 0; i < 6; i++ ) {
		if( triangleTable[code].capPlanesTab[i] > 0 ) {
			triangleTable[code].capPlanes = true;
			break;
		}
	}
}

int MarchingCubes::_fixTrianglesNormals( int code )
{
    MarchingCubesCase& cubeCase = triangleTable[code];
    int counter = 0;
    for( int t = 0; t < cubeCase.numTri; t++ ) {
        int e1 = cubeCase.tris[t][0];
        int e2 = cubeCase.tris[t][1];
        int e3 = cubeCase.tris[t][2];

        Vector3F v1 = getHalfEdge(e1);
        Vector3F v2 = getHalfEdge(e2);
        Vector3F v3 = getHalfEdge(e3);

        Vector3F delta1 = v2 - v1;
        Vector3F delta2 = v3 - v1;
        Vector3F normal;
        getCrossProduct( delta1.f, delta2.f, normal.f );

        if( dotProduct(normal, cubeCase.normal[t]) < 0 ) {
            int tmp = cubeCase.tris[t][1];
            cubeCase.tris[t][1] = cubeCase.tris[t][2];
            cubeCase.tris[t][2] = tmp;
            counter++;
        }
        else {     int x = 5;   }
    }
    return counter;
}


int MarchingCubes::_capSingleVertexPlane( MarchingCubesCase& cubeCase, int v1, int v2 )
{
    int edges[4] = {-1};
    _getPlaneEdges( v1, v2, edges );

    int counter = 0;
    Vector3F normal = _getNormalFromBits( v1 );

    cubeCase.normal[cubeCase.numTri] = normal;
    cubeCase.tris[ cubeCase.numTri ][0] = edges[0];
    cubeCase.tris[ cubeCase.numTri ][1] = edges[1];
    cubeCase.tris[ cubeCase.numTri ][2] = edges[2];
    cubeCase.numTri++;

    cubeCase.normal[cubeCase.numTri] = normal;
    cubeCase.tris[ cubeCase.numTri ][0] = edges[3];
    cubeCase.tris[ cubeCase.numTri ][1] = edges[1];
    cubeCase.tris[ cubeCase.numTri ][2] = edges[2];
    cubeCase.numTri++;

    counter += 2;

    return counter;
}

