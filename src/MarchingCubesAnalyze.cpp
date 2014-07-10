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

//
//	Bit tools
//

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

bool MarchingCubes::_vertexIsNegByAxis( int v, int axis )
{
    if( v & (1<<axis) )
        return false;
    return true;
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


//
//	Fill basic geometry data
//

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

MarchingCubes::Vector3F MarchingCubes::getTriangleNormal( const Vector3F& v0, const Vector3F& v1, const Vector3F& v2 )
{
	Vector3F  delta1 = v1 - v0;
	Vector3F  delta2 = v2 - v0;

	Vector3F  normal;
	getCrossProduct( delta1.f, delta2.f, normal.f );
	return normal;
}

int MarchingCubes::_fixPlaneEdgesNormal( int plane, int planeEdges[4] )
{
	Vector3F vec[3];
	for( int i = 0; i < 3; i++ )
		vec[i] = getHalfEdge( planeEdges[i] );

//	Vector3F  delta1 = vec[1] - vec[0];
//	Vector3F  delta2 = vec[2] - vec[0];
//	Vector3F  normal;
//	getCrossProduct( delta1.f, delta2.f, normal.f );

	Vector3F  normal = getTriangleNormal( vec[0], vec[1], vec[2] );

	int axis = _planeToAxis( plane );
	int sign = _planeToSign( plane );

	Vector3F normal2(0.0f,0.0f,0.0f);
	if( sign == 1 )
		normal2.f[axis] = 1.0f;
	else if( sign == 0 )
		normal2.f[axis] = -1.0f;
	else
		throw "_fixPlaneEdgesNormal: wrong sign error!";

	float dot = dotProduct( normal, normal2 );
	if( dot < 0.0f ) {
		int tmp = planeEdges[1];
		planeEdges[1] = planeEdges[2];
		planeEdges[2] = tmp;
		return 1;
	}
	else if( dot == 0.0f )
		throw "_fixPlaneEdgesNormal: zero dot product error!";

	return 0;
}

void MarchingCubes::_fillPlanes()
{
	for( int axis = 0; axis < 3; axis++ )
	{
		for( int sign = 0; sign < 2; sign++ )
		{
			int plane = _planeFromAxisSign( axis, sign );

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

			_fixPlaneEdgesNormal( plane, planeToEdge[plane] );

			for( int vert = 0; vert < 8; vert++ ) {
				if( _vertexIsAtAxisSide( vert, axis, sign ) )
					planeToVertex[plane][vertexCounter++] = vert;
			}
			assert( vertexCounter == 4 );
		}
	}
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


void MarchingCubes::_getEdgesAlongAxis( int axis, int edges[4] )
{
    int counter = 0;
    for( int edge = 0; edge < 12; edge++ )
		{
/*					bool b10 = _vertexIsAtAxisSide( edgeToVertex[edge][0], axis, false );
					bool b11 = _vertexIsAtAxisSide( edgeToVertex[edge][0], axis, true );
					bool b2 = _vertexIsNegByAxis( edgeToVertex[edge][0], axis );
					if( b10 != b2 ) {
						int x = 5;
					}*/
        if( _vertexIsNegByAxis(edgeToVertex[edge][0],axis) !=
            _vertexIsNegByAxis(edgeToVertex[edge][1],axis) )
            edges[counter++] = edge;
	}
	assert( counter == 4 );
}

int MarchingCubes::_getVertexBySymmetry( int vertex, int axis )
{
    int res = vertex ^ (1<<axis);		// (^) XOR operation
    return res;
}

int MarchingCubes::_getEdgeBySymmetry( int edge, int axis )
{
    int v1 = edgeToVertex[edge][0];
    int v2 = edgeToVertex[edge][1];
    int v1Reflected = _getVertexBySymmetry( v1, axis );
    int v2Reflected = _getVertexBySymmetry( v2, axis );

	int res = _findEdge( v1Reflected, v2Reflected );
	return res;
}

void MarchingCubes::_getPlaneEdges( int v1, int v2, int edges[4] )
{
    int eCounter = 0;

    for( int i = 0; i < 8; i++ )
        if( _oneBitDiff(i,v1) && _oneBitDiff(i,v2) )
        {
            int e1 = _findEdge(i,v1);
            int e2 = _findEdge(i,v2);
            edges[eCounter++] = e1;
            edges[eCounter++] = e2;
        }

    assert( eCounter == 4 );
}
/*void MarchingCubes::_getPlaneEdges( int plane, int edges[4] )
{
    int eCounter = 0;

    for( int e = 0; e < 12; e++ ) {

		int v1 = edgeToVertex[e][0];
		int v2 = edgeToVertex[e][1];

//		if( _vertexIsAtAxisSide(v,))
    }
        if( _oneBitDiff(i,v1) && _oneBitDiff(i,v2) )
        {
            int e1 = _findEdge(i,v1);
            int e2 = _findEdge(i,v2);
            edges[eCounter++] = e1;
            edges[eCounter++] = e2;
        }

    assert( eCounter == 4 );
}*/

MarchingCubes::Vector3F MarchingCubes::_getNormalFromBits( int bits )
{
    MarchingCubes::Vector3F res;

    for( int i = 0; i < 3; i++ )
    {
        if( bits&(1<<i) )
            res.f[i] = 1.0f;
        else
            res.f[i] = -1.0f;
    }
    return res;
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
if( i == 49 ) {
	int x = 5;
}
            int tris = 0;
            tris += _findSingleVertexTriangles( i );
            tris += _findEdgeTriangles( i );
            tris += _findHalfSplit( i );
            tris += _findTripleVertex( i );
            tris += _findFourVertex( i );
            tris += _findSnake( i );

            int fixed = _fixTrianglesNormals( i );

			_selectCapPlanes( i );
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
			}
		}//*/
    }
    return counter;
}

int MarchingCubes::_findEdgeTriangles( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int e = 0; e < 12; e++ )
	{
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
                cubeCase.normal[cubeCase.numTri] = _getNormalFromBits(v1);
            else
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

    for( int axis = 0; axis < 3; axis++ )
	{
        bool casePosFail = false;
        bool caseNegFail = false;

        for( int v = 0; v < 8; v++ )
		{
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

float getComponentByIndex( MarchingCubes::Vector3F vec, int index )
{
	return vec.f[index];
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
			int plane = _planeFromAxisSign( axis, sign );

			triangleTable[code].capPlanesTab[plane] = 0;

			int* planeEdges = planeToEdge[plane];

			bool side = false;
			std::set<int> edgesSet;
			MarchingCubesCase& cubeCase = triangleTable[code];

			int triangleOnPlaneCount = 0;
			// for each triangle
			for( int tri = 0; tri < cubeCase.numTri; tri++ )
			{
				TriangleI& triI = cubeCase.tris[tri];

				int planeVertCount = 0;
				for( int triVert = 0; triVert < 3; triVert++ ) {
					for( int edgeIdx = 0; edgeIdx < 4; edgeIdx++ ) {
						if( triI.i[triVert] == planeEdges[edgeIdx] )
							planeVertCount++;
					}
				}

				// THIS TRIANGLE
				if( planeVertCount == 2 ) {
					triangleOnPlaneCount++;
				}
				if( triangleOnPlaneCount == 2 ) {
if( code == 150 && plane == 1 ) {
	int x =5;
}

					Vector3F verts[3];
					for( int v = 0; v < 3; v++ )
						verts[v] = getHalfEdge( triI.i[v] );

//					Vector3F  delta1 = verts[1] - verts[0];
//					Vector3F  delta2 = verts[2] - verts[0];
//					Vector3F  normal;
//					getCrossProduct( delta1.f, delta2.f, normal.f );
					Vector3F  normal = getTriangleNormal( verts[0], verts[1], verts[2] );

					float val = getComponentByIndex( normal, axis );

					int planeSign = 0;
					if( (val * ((float)sign - 0.5f)) > 0.0f )
						planeSign = 1;
					else if( (val * ((float)sign - 0.5f)) < 0.0f )
						planeSign = -1;
					else {
						Vector3F cubeCenter(0.5f,0.5f,0.5f);
						Vector3F triCenter = verts[0] + verts[1] + verts[2];
						triCenter.f[0] /= 3.0f;
						triCenter.f[1] /= 3.0f;
						triCenter.f[2] /= 3.0f;

						Vector3F triToCenter = cubeCenter - triCenter;

						float dot = dotProduct( normal, triToCenter );
						if( dot > 0 )
							planeSign = 1;
						else if( dot < 0 )
							planeSign = -1;
						else
							throw "sdfsd";
					}

					if( sign == 0 )
						planeSign = -planeSign;

					triangleTable[code].capPlanesTab[plane] = planeSign;

					break;
				}
			}
		}
	}

	// if any of the planes can be capped - set the main cap flag
	// it will be easier during rendering to check one flag instead of 6
	for( int i = 0; i < 6; i++ ) {
		if( triangleTable[code].capPlanesTab[i] != 0 ) {
			triangleTable[code].capPlanes = true;
			break;
		}
	}
}

int MarchingCubes::_fixTrianglesNormals( int code )
{
    MarchingCubesCase& cubeCase = triangleTable[code];
    int counter = 0;
    for( int t = 0; t < cubeCase.numTri; t++ )
	{
        int e1 = cubeCase.tris[t][0];
        int e2 = cubeCase.tris[t][1];
        int e3 = cubeCase.tris[t][2];

        Vector3F v1 = getHalfEdge(e1);
        Vector3F v2 = getHalfEdge(e2);
        Vector3F v3 = getHalfEdge(e3);

//        Vector3F delta1 = v2 - v1;
//        Vector3F delta2 = v3 - v1;
//        Vector3F normal;
//        getCrossProduct( delta1.f, delta2.f, normal.f );
		Vector3F  normal = getTriangleNormal( v1, v2, v3 );

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
