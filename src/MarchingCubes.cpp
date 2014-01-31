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
//include/

inline bool _differentSign( int a, int b )
{
    if( a >= 0 && b >= 0 )
        return false;
    if( a < 0 && b < 0 )
        return false;
    return true;
}
inline bool _sameSign( int a, int b )
{
    return !_differentSign( a, b );
}


MarchingCubes::MarchingCubes()
{
    memset( usageStats, 0, sizeof(usageStats) );
}

void MarchingCubes::init()
{
    _fillVertices();
    _fillEdges();
    generateTriangles();

    printTable();
}

int MarchingCubes::generateTriangles()
{
    triangleTable[0].index = triangleTable[0].numTri = 0;
    triangleTable[255].index = 255;
    triangleTable[255].numTri = 0;

    for( int i = 1; i < 255; i++ ) {
        if( triangleTable[i].index < 1 ) {

            int tris = _findSingleVertexTriangles(i);
            tris = _findEdgeTriangles( i );
            tris = _findHalfSplit( i );
            tris = _findTripleVertex( i );
            tris = _findFourVertex( i );
            tris = _findSnake( i );

            _fixTriangles( i );
            triangleTable[i].index = i;
        }
    }
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
    for( int i = 0; i < 256; i++ ) {
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
        printf("\n");
    }
    printf( "case count:%d\n", rowCount );
}

void MarchingCubes::_getEdgesAlongAxis( int axis, int edges[4] )
{
    int counter = 0;
    for( int edge = 0; edge < 12; edge++ ) {
        if( _vertexIsNegByAxis(edgeToVertex[edge][0],axis) !=
            _vertexIsNegByAxis(edgeToVertex[edge][1],axis) )
            edges[counter++] = edge;
    }
}

int MarchingCubes::_getVertexBySymmetry( int vertex, int axis )
{
    int res = vertex ^ (1<<axis);
    return res;
}

int MarchingCubes::_getEdgeBySymmetry( int edge, int axis )
{
    int v1 = edgeToVertex[edge][0];
    int v2 = edgeToVertex[edge][1];
    int v1Reflected = _getVertexBySymmetry( v1, axis );
    int v2Reflected = _getVertexBySymmetry( v2, axis );
    for( int e = 0; e < 12; e++ ) {
        if( edgeToVertex[e][0] == v1Reflected &&
            edgeToVertex[e][1] == v2Reflected )
            return e;
        if( edgeToVertex[e][1] == v1Reflected &&
            edgeToVertex[e][0] == v2Reflected )
            return e;
    }
    return -1;
}

void MarchingCubes::setValues( float vert[8] )
{
    for( int v = 0; v < 8; v++ )
        vertex[v] = vert[v];
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

MarchingCubes::Vector3F MarchingCubes::getVertexFromEdge( int edgeNum )
{
    int v1 = edgeToVertex[edgeNum][0];
    int v2 = edgeToVertex[edgeNum][1];

    float vf1 = vertex[v1];
    float vf2 = vertex[v2];

    Vector3F result;

    float v1x = vertexOffset[v1].f[0];
    float v1y = vertexOffset[v1].f[1];
    float v1z = vertexOffset[v1].f[2];
    float v2x = vertexOffset[v2].f[0];
    float v2y = vertexOffset[v2].f[1];
    float v2z = vertexOffset[v2].f[2];

    float perc = vf1/(vf1-vf2);
    result.f[0] = v1x + (v2x-v1x) * perc;
    result.f[1] = v1y + (v2y-v1y) * perc;
    result.f[2] = v1z + (v2z-v1z) * perc;

    return result;
}

bool MarchingCubes::_oneBitDiff( int v1, int v2 )
{
    int diff = v1 ^ v2;    //  XOR
    if( diff == 1 || diff == 2 || diff == 4 )
        return true;
    return false;
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


int MarchingCubes::_findSingleVertexTriangles( int code )
{
    int counter = 0;

    // signTab - signs for all corners
    int signTab[8];
    _codeToSignTable( code, signTab );

    for( int v = 0; v < 8; v++ ) {
        int vRefl[3];
        for( int ax = 0; ax < 3; ax++ )
            vRefl[ax] = _getVertexBySymmetry( v, ax );

        if( signTab[v] >= 0 &&
            signTab[vRefl[0]] < 0 &&
            signTab[vRefl[1]] < 0 &&
            signTab[vRefl[2]] < 0 )
            {
                MarchingCubesCase& cubeCase = triangleTable[code];
                cubeCase.normal = _getNormalFromBits( 7-v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v, vRefl[0] );
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v, vRefl[1] );
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v, vRefl[2] );

                cubeCase.numTri++;
                counter++;
            }
        else if( signTab[v] < 0 &&
            signTab[vRefl[0]] >= 0 &&
            signTab[vRefl[1]] >= 0 &&
            signTab[vRefl[2]] >= 0 )
            {
                MarchingCubesCase& cubeCase = triangleTable[code];
                cubeCase.normal = _getNormalFromBits( //7-
                                                        v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v, vRefl[0] );
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v, vRefl[1] );
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v, vRefl[2] );

                cubeCase.numTri++;
                counter++;
            }
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
                cubeCase.normal = _getNormalFromBits(v1);
            else
                cubeCase.normal = _getNormalFromBits(7-v1);
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge( v1, _getVertexBySymmetry(v1,(edgeAxis+1)%3) );
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge( v1, _getVertexBySymmetry(v1,(edgeAxis+2)%3) );
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge( v2, _getVertexBySymmetry(v2,(edgeAxis+2)%3) );

            cubeCase.numTri++;
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
            cubeCase.normal = _getNormalFromBits( 0 );
            cubeCase.tris[ cubeCase.numTri ][0] = startEdge;
            cubeCase.tris[ cubeCase.numTri ][1] = edge1;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;
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
            cubeCase.normal = _getNormalFromBits( 7 );
            cubeCase.tris[ cubeCase.numTri ][0] = startEdge;
            cubeCase.tris[ cubeCase.numTri ][1] = edge1;
            cubeCase.tris[ cubeCase.numTri ][2] = edge2;
            cubeCase.numTri++;
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
                cubeCase.normal = _getNormalFromBits( v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl1A);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl1, vRefl12);
                cubeCase.numTri++;

                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl2A);
                cubeCase.numTri++;

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
                cubeCase.normal = _getNormalFromBits( 7-v );
                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl1, vRefl1A);
                cubeCase.numTri++;

                cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl12);
                cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl2A);
                cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
                cubeCase.numTri++;

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
            cubeCase.normal = _getNormalFromBits( v );
            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl1, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl12);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
            cubeCase.numTri++;

            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl3, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl12);
            cubeCase.numTri++;

            cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl3, vRefl13);
            cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl2, vRefl12);
            cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl2, vRefl23);
            cubeCase.numTri++;

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

                int vRefl1 = _getVertexBySymmetry( v, ax1 );
                int vRefl12 = _getVertexBySymmetry( vRefl1, ax2 );
                int vRefl123 = _getVertexBySymmetry( vRefl12, ax3 );

                std::set<int> vertPath;
                vertPath.insert( v );
                vertPath.insert( vRefl1 );
                vertPath.insert( vRefl12 );
                vertPath.insert( vRefl123 );

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
                    int vRefl2 = _getVertexBySymmetry( v, ax2 );
                    int vRefl3 = _getVertexBySymmetry( v, ax3 );
                    int vRefl13 = _getVertexBySymmetry( vRefl1, ax3 );
                    int vRefl23 = _getVertexBySymmetry( vRefl2, ax3 );

                    MarchingCubesCase& cubeCase = triangleTable[code];
                    cubeCase.normal = _getNormalFromBits( vRefl1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(v, vRefl3);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(vRefl1, vRefl13);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl123, vRefl23);
                    cubeCase.numTri++;

                    cubeCase.normal = _getNormalFromBits( vRefl1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(v, vRefl2);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(v, vRefl3);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl123, vRefl23);
                    cubeCase.numTri++;

                    cubeCase.normal = _getNormalFromBits( vRefl1 );
                    cubeCase.tris[ cubeCase.numTri ][0] = _findEdge(vRefl2, vRefl12);
                    cubeCase.tris[ cubeCase.numTri ][1] = _findEdge(v, vRefl2);
                    cubeCase.tris[ cubeCase.numTri ][2] = _findEdge(vRefl123, vRefl23);
                    cubeCase.numTri++;

                    counter+=3;
                    return counter;
                }
            }
        }
    }
    return counter;
}

int MarchingCubes::_fixTriangles( int code )
{
    int counter = 0;
    for( int t = 0; t < triangleTable[code].numTri; t++ ) {
        int e1 = triangleTable[code].tris[t][0];
        int e2 = triangleTable[code].tris[t][1];
        int e3 = triangleTable[code].tris[t][2];

        Vector3F v1 = getHalfEdge(e1);
        Vector3F v2 = getHalfEdge(e2);
        Vector3F v3 = getHalfEdge(e3);

        Vector3F delta1 = v2 - v1;
        Vector3F delta2 = v3 - v1;
        Vector3F normal;
        getCrossProduct( delta1.f, delta2.f, normal.f );

        if( dotProduct(normal, triangleTable[code].normal) < 0 ) {
            int tmp = triangleTable[code].tris[t][1];
            triangleTable[code].tris[t][1] = triangleTable[code].tris[t][2];
            triangleTable[code].tris[t][2] = tmp;
            counter++;
        }
    }
    return counter;
}

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

        tris[triNum].v[0].f[0] = vec1.f[0];
        tris[triNum].v[0].f[1] = vec1.f[1];
        tris[triNum].v[0].f[2] = vec1.f[2];

        tris[triNum].v[1].f[0] = vec2.f[0];
        tris[triNum].v[1].f[1] = vec2.f[1];
        tris[triNum].v[1].f[2] = vec2.f[2];

        tris[triNum].v[2].f[0] = vec3.f[0];
        tris[triNum].v[2].f[1] = vec3.f[1];
        tris[triNum].v[2].f[2] = vec3.f[2];
    }
    return triNum;
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

bool MarchingCubes::_vertexIsNegByAxis( int code, int axis )
{
    if( code & (1<<axis) )
        return false;
    return true;
}

MarchingCubes::Vector3F& MarchingCubes::_getNormalFromBits( int bits )
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
