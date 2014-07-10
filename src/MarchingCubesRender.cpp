/*
    MarchingCubes - main class for all Marching Cubes computations
    Author: Karol Herda
    Web:    http://kolenda.me/algorithmic-marching-cubes/
    Date:   12-03-2013
*/

#include <set>
#include <map>
#include <stdio.h>
#include <string.h>
#include "MarchingCubes.h"


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
        Cube2 cube = field.getCube( x, y, z );
		cube.setGridSize( field.getSizeX(), field.getSizeY(), field.getSizeZ() );

		if( currTriangle < maxTris - 10 ) {
			TriangleF     tmpTris[10];
			setValues( cube );

			MarchingCubesCase &cubeCase = getCaseFromValues();
					usageStats[cubeCase.index]++;

			int triNum = 0;
			// for each triangle
			for( ; triNum < cubeCase.numTri; triNum++ )
			{
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

//				Vector3F  delta1 = vec2 - vec1;
//				Vector3F  delta2 = vec3 - vec1;

//				Vector3F  normal;
//				getCrossProduct( delta1.f, delta2.f, normal.f );

				//	compute face normal
				Vector3F  normal = getTriangleNormal( vec1, vec2, vec3 );

				// Check for 0 or we get NaN errors
				if( normal.isNotZero() ) {
					normal.normalise();

					// add normal to the cache
					vert[index1].norm += normal;
					vert[index2].norm += normal;
					vert[index3].norm += normal;
				}

				tris[currTriangle].i[0] = index1;
				tris[currTriangle].i[1] = index2;
				tris[currTriangle].i[2] = index3;
				currTriangle++;
			}

			//*
			if( cubeCase.capPlanes )
			{
				for( int plane = 0; plane < 6; plane++ )
				{
					int p = cubeCase.capPlanesTab[plane];
					if( p != 0 )
					{
						int offset = _cacheOffsetFromPlane( x, y, z, plane );

						std::map<int,int>::iterator cacheIter = capPlaneCache.find(offset);
						if( cacheIter != capPlaneCache.end() )
						{
							int p2 = cacheIter->second;
							capPlaneCache.erase( offset );

							if( p2 == p )
								_capPlane( vert, tris, x,y,z, plane, p );
						}
						else {
							capPlaneCache.insert( std::pair<int,int>(offset, p) );
						}
					}
				}
			}//*/
		}	// cur tri
    }	//	for

//	int	lenVector[10] = {0};

    for( int v = 0; v < currVert; v++ )
	{
//		if( vert[v].norm.length() < 0.5 ) {
//			int x = 5;
//		}
//		if( vert[v].used < 10 ) {
//			lenVector[ vert[v].used ] ++;
//		}
        vert[v].norm.normalise();
    }

    vertexNum = currVert;
    triNum = currTriangle;

    return currTriangle;
}


int MarchingCubes::_capPlane( MarchingCubes::Vertex* vert, MarchingCubes::TriangleI* tris,
								int x, int y, int z, int plane, int side )
{
	int* edges = planeToEdge[plane];

	int index[4];
	for( int i = 0; i < 4; i++ )
		index[i] = _cacheVertex( vert, x,y,z, edges[i] );

	Vector3F vec1 = vert[ index[0] ].pos;
	Vector3F vec2 = vert[ index[1] ].pos;
	Vector3F vec3 = vert[ index[2] ].pos;
	Vector3F vec4 = vert[ index[3] ].pos;

	Vector3F  normal21;
	Vector3F  normal22;

	if( side == -1 ) {
		normal21 = getTriangleNormal( vec1, vec2, vec3 );
		normal22 = getTriangleNormal( vec4, vec3, vec2 );
	}
	else if( side == 1 ) {
		normal21 = getTriangleNormal( vec2, vec1, vec3 );
		normal22 = getTriangleNormal( vec3, vec4, vec2 );
	}
	else
		throw "side";

	normal21.normalise();
	normal22.normalise();



	Vector3F  delta1 = vec2 - vec1;
	Vector3F  delta2 = vec3 - vec1;

	Vector3F  delta21 = vec4 - vec2;
	Vector3F  delta22 = vec4 - vec3;

	//	compute face normal
	Vector3F  normal, normal2;

	if( side == -1 ) {
		getCrossProduct( delta1.f, delta2.f, normal.f );
		getCrossProduct( delta21.f, delta22.f, normal2.f );
	}
	else if( side == 1 ) {
		getCrossProduct( delta2.f, delta1.f, normal.f );
		getCrossProduct( delta22.f, delta21.f, normal2.f );
	}
	normal.normalise();
	normal2.normalise();

//TODO: fix those normals
	Vector3F d1 = normal - normal21;
	Vector3F d2 = normal2 - normal22;
	if( d1.isNotZero() ||
		d2.isNotZero() ) {
		int x = 5;
		//throw "normal";
	}

	//*	// add normal to the cache

	vert[ index[0] ].norm += normal;
	vert[ index[1] ].norm += normal;
	vert[ index[2] ].norm += normal;

	vert[ index[1] ].norm += normal2;
	vert[ index[2] ].norm += normal2;
	vert[ index[3] ].norm += normal2;

	if( side == -1 ) {
		tris[currTriangle].i[0] = index[0];
		tris[currTriangle].i[1] = index[1];
		tris[currTriangle].i[2] = index[2];
		currTriangle++;

		tris[currTriangle].i[0] = index[1];
		tris[currTriangle].i[1] = index[3];
		tris[currTriangle].i[2] = index[2];
		currTriangle++;
	}
	else if( side == 1 ) {
		tris[currTriangle].i[0] = index[0];
		tris[currTriangle].i[1] = index[2];
		tris[currTriangle].i[2] = index[1];
		currTriangle++;

		tris[currTriangle].i[0] = index[1];
		tris[currTriangle].i[1] = index[2];
		tris[currTriangle].i[2] = index[3];
		currTriangle++;
	}
}


// param: edge index
// returns: vertex along edge, computed with current voxel field state
MarchingCubes::Vector3F MarchingCubes::getVertexFromEdge( int edgeNum )
{
	// get two vertex indices
    int v1 = edgeToVertex[edgeNum][0];
    int v2 = edgeToVertex[edgeNum][1];

	// get current values for two vertices
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

/*
struct AxisVert {
    GLfloat pos[3];
    GLubyte color[3];
};
void generateTrianglesVBO() {
//    glGenBuffersARB( 1, &trianglesVertexBuffer );
    glGenBuffers( 1, &trianglesVertexBuffer );
}
void deleteTrianglesVBO() {
    glDeleteBuffers(1, &trianglesVertexBuffer );
}
void generateTrianglesIndexedVBO() {
    glGenBuffers( 1, &trianglesIndexedVertexBuffer );
    glGenBuffers( 1, &trianglesIndexedIndexBuffer );
}
void deleteTrianglesIndexedVBO() {
    glDeleteBuffers(1, &trianglesIndexedVertexBuffer );
    glDeleteBuffers(1, &trianglesIndexedIndexBuffer );
}


int currentTriangleNum = 0;
int currentTriangleIndexedNum  = 0;

void fillTrianglesVBO( MarchingCubes::TriangleF* tris, int triNum ) {
    glBindBuffer( GL_ARRAY_BUFFER, trianglesVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, triNum*sizeof(MarchingCubes::TriangleF), tris, GL_DYNAMIC_DRAW );  //
                                                                                  //  GL_STATIC_DRAW );
    currentTriangleNum = triNum;
}

void fillTrianglesIndexedVBO( MarchingCubes::Vertex* vert, int vertexNumber, MarchingCubes::TriangleI* tris, int triangleNumber ) {

    glBindBuffer( GL_ARRAY_BUFFER, trianglesIndexedVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, vertexNumber*sizeof(MarchingCubes::Vertex), vert, GL_DYNAMIC_DRAW );  //
                                                                                  //  GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, trianglesIndexedIndexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, triangleNumber*sizeof(MarchingCubes::TriangleI), tris, GL_DYNAMIC_DRAW );

    currentTriangleIndexedNum = triangleNumber;
}


void drawTrianglesVBO() {
    glBindBuffer( GL_ARRAY_BUFFER, trianglesVertexBuffer );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    glVertexPointer( 3, GL_FLOAT, sizeof(MarchingCubes::Vertex), NULL ); //(GLvoid*)(verts[0].pos));
    glNormalPointer( GL_FLOAT, sizeof(MarchingCubes::Vertex), (GLvoid*)(sizeof(GLfloat)*3) );

    if( wireframe )     glDrawArrays( GL_LINES, 0, currentTriangleNum*3 );
    else                glDrawArrays( GL_TRIANGLES, 0, currentTriangleNum*3 );

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}

void drawTrianglesIndexedVBO()
{
    glBindBuffer( GL_ARRAY_BUFFER, trianglesIndexedVertexBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, trianglesIndexedIndexBuffer );

    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_NORMAL_ARRAY );

    glVertexPointer( 3, GL_FLOAT, sizeof(MarchingCubes::Vertex), NULL ); //(GLvoid*)(verts[0].pos));
    glNormalPointer( GL_FLOAT, sizeof(MarchingCubes::Vertex), (GLvoid*)(sizeof(float    //GLfloat
                                                                                    )*3) );

//    glVertexPointer( 3, GL_FLOAT, sizeof(AxisVert), NULL ); //(GLvoid*)(verts[0].pos));
  //  glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof(AxisVert), (GLvoid*)(sizeof(GLfloat)*3));

    if( wireframe ){
//        for( int i = 0; i < currentTriangleIndexedNum; i++ )
  //      glDrawElements( GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)0);
                        glDrawElements( GL_LINES, currentTriangleIndexedNum*3, GL_UNSIGNED_INT, (void*)0);
    }
    else                glDrawElements( GL_TRIANGLES, currentTriangleIndexedNum*3, GL_UNSIGNED_INT, (void*)0);

    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );
}*/
/*void generateAxesVBO() {
    const GLfloat axExt = 40.0f;

    AxisVert    verts[6] = {
    { 0.0f, 0.0f, 0.0f, 255, 255, 255 },
    { axExt, 0.0f, 0.0f, 255, 0, 0 },
    { 0.0f, 0.0f, 0.0f, 255, 255, 255 },
    { 0.0f, axExt, 0.0f, 0, 255, 0 },
    { 0.0f, 0.0f, 0.0f, 255, 255, 255 },
    { 0.0f, 0.0f, axExt, 0, 0, 255 },
    };

    glGenBuffersARB( 1, &axesVertexBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, axesVertexBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);  //STATIC_DRAW);
}
void deleteAxesVBO() {
    glDeleteBuffers(1, &axesVertexBuffer);
    glDeleteBuffers(1, &axesIndexBuffer);
}*/

/*
void drawAxesVBO()
{
    glBindBuffer( GL_ARRAY_BUFFER, axesVertexBuffer );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer( 3, GL_FLOAT, sizeof(AxisVert), NULL ); //(GLvoid*)(verts[0].pos));
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof(AxisVert), (GLvoid*)(sizeof(GLfloat)*3));

    glDrawArrays( GL_LINES, 0, 6 );

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glBindBuffer( GL_ARRAY_BUFFER, axesVertexBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, axesIndexBuffer );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer( 3, GL_FLOAT, sizeof(AxisVert), NULL ); //(GLvoid*)(verts[0].pos));
    glColorPointer( 3, GL_UNSIGNED_BYTE, sizeof(AxisVert), (GLvoid*)(sizeof(GLfloat)*3));

    glDrawElements( GL_LINES, 6, GL_UNSIGNED_INT, (void*)0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}*/
/*void drawVBO()
{
    setupLight( lighting );
    setView();

//    drawTrianglesVBO();
    drawTrianglesIndexedVBO();

    print = false;

    setupLight( false );
    drawAxesVBO();

    glLoadIdentity ();
    glColor3f( 1, 1, 1 );
}*/
