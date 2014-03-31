/*
    MarchingCubes - main class for all Marching Cubes computations
    Author: Karol Herda
    Web:    http://kolenda.vipserv.org/algorithmic-marching-cubes/
    Date:   12-03-2013

    Defines basic data structures and algorithms used to compute geometry

    During initialization it analyzes all 256 possible cases and stores the result in a table:

        MarchingCubes mc;
        mc.init();

    Then you need to set corner values of the cube you want to generate:

        float values[8];
        ... set values ...
        mc.setValues( values );

    and ask for triangles for this case:

        MarchingCubes::TriangleF     tris[4];
        int triNum = march.fillInTriangles( tris );

    Then you need to draw tris[0] to tris[triNum-1] - look at the TriangleF definition, it's really simple
    But remember - object 'mc' doesn't have any data about position, where you want to render it
                    so it returns triangles in (0,0,0)->(1,1,1) space, you need to translate it on your own
*/

#ifndef MARCHINGCUBES_H
#define MARCHINGCUBES_H

#include "VoxelField.h"
#include <math.h>

#define CAP_TRI_OFFSET 16

class MarchingCubes
{
	int sizeX;
	int sizeY;
	int sizeZ;

	int sizePlane;

public:
    // 3D vector class storing float position
    struct  Vector3F {
        float f[3];
        Vector3F& operator+ (Vector3F& v1) {
            Vector3F res;
            res.f[0] = f[0] + v1.f[0];
            res.f[1] = f[1] + v1.f[1];
            res.f[2] = f[2] + v1.f[2];
            return res;
        };
        Vector3F& operator+= (const Vector3F& v1)
        {
            f[0] += v1.f[0];
            f[1] += v1.f[1];
            f[2] += v1.f[2];

            return *this;
        }
        Vector3F operator- (Vector3F& v1) {
            Vector3F res;
            res.f[0] = f[0] - v1.f[0];
            res.f[1] = f[1] - v1.f[1];
            res.f[2] = f[2] - v1.f[2];
            return res;
        };
        void setValue( float x, float y, float z ) {
			f[0] = x;
			f[1] = y;
			f[2] = z;
		}
        void normalise() {
            float len = f[0]*f[0] + f[1]*f[1] + f[2]*f[2];
            float lenSqrRoot = sqrtf(len);
            float mulFactor = 1 / lenSqrRoot;
            f[0] *= mulFactor;
            f[1] *= mulFactor;
            f[2] *= mulFactor;
        }
        float length() {
            float len = f[0]*f[0] + f[1]*f[1] + f[2]*f[2];
            float lenSqrRoot = sqrtf(len);
            return lenSqrRoot;
        }
        bool isNotZero() {
			return f[0] != 0.0f || f[1] != 0.0f || f[2] != 0.0f;
        }
    };
    struct Vertex {
        Vector3F    pos;
        Vector3F    norm;
        int		used;
    };
    // Triangle described as 3 vector positions
    struct  TriangleF {
        Vertex  v[3];
    };
    // Triangle represented by 3 vector indices - will be needed for vertex buffers (some day;))
    struct  TriangleI {
        int i[3];
        int& operator[] (int index) {
            return i[index];
        };
    };
    // Description of one combination of corners
    //  stores its own index, number of triangles with triangle table, and normal used during data generation
    struct  MarchingCubesCase {
		int				index;
		int				numTri;
		TriangleI		tris[25];
        Vector3F		normal[25];
//      TriangleI tris[10];
  //      Vector3F normal;
		unsigned char	capPlanes;
		unsigned char	capPlanesTab[6];
    };

	void	setOffsets( float sizex, float sizey, float sizez );

private:
    // main table storing all geometry data, it's generated during initialization and used for rendering
    MarchingCubesCase    triangleTable[256];
    // stores statistics for each case telling how many times it was used
    int                 usageStats[256];
    // temporary table of corner values
    float               vertex[8];

    // positions of all corners
    Vector3F            vertexOffset[8];
    // definition of edges, we have 12 of them, each is stored as 2 vertex indices
    int                 edgeToVertex[12][2];
	int					planeToVertex[6][4];
	int					planeToEdge[6][4];

int currTriangle;
int currVert;

    // returns index of axis parallel to the edge
    int         _getEdgeAxis( int edge );
    // returns corner reflected by the given axis
    int         _getVertexBySymmetry( int vertex, int axis );
    // returns edge reflected by the given axis
    int         _getEdgeBySymmetry( int edge, int axis );
    // check if params differ by 1 and only 1 bit (checks only 3 first bits)
    bool        _oneBitDiff( int v1, int v2 );
    // check if params differ by 2 and only 2 bits (checks only 3 first bits)
    bool        _twoBitsDiff( int v1, int v2 );
    // gets 4 edges parallel to the axis
    void        _getEdgesAlongAxis( int axis, int edges[4] );


    void        _getPlaneEdges( int v1, int v2, int edges[4] );
//	void		_getPlaneEdges( int plane, int edges[4] );

    // TODO delete?
    int         _capSingleVertexPlane( MarchingCubesCase& cubeCase, int v1, int v2 );
	int			_capPlane( MarchingCubes::Vertex* vert, MarchingCubes::TriangleI* tris, int x, int y, int z, int plane, int side );


//  ++startup data++
    void        _fillVertices();
    void        _fillEdges();
    void		_fillPlanes();

    int         _findEdge( int v1, int v2 );
//  --startup data--

//  ++triangle table generation++
    // main generate method
    int         generateTriangles();

    int         _findSingleVertexTriangles( int code );
    int         _findEdgeTriangles( int code );
    int         _findHalfSplit( int code );
    int         _findTripleVertex( int code );
    int         _findFourVertex( int code );
    int         _findSnake( int code );
	int			_selectCapPlanes( int code );
    // fixes triangles direction between CW and CCW
    int         _fixTrianglesNormals( int code );
//  --triangle table generation--


bool _vertexIsAtAxisSide( int v, int axis, int sign );


    int         _bitsToCode( float verts[8] );

    // gets an interpolated vector of given edge, based on its corner values
    Vector3F    getVertexFromEdge( int edgeNum );
    // gets a center of given edge
    Vector3F    getHalfEdge( int edgeNum );

    void        _codeToSignTable( int code, int* tab );
    bool        _vertexIsNegByAxis( int v, int axis );
    Vector3F   _getNormalFromBits( int bits );

    // prints all values from triangles table - for debug purposes
    void        printTable();

    VoxelField& field;


	//	tools
	inline bool _differentSign( int a, int b );
	//inline
	bool _sameSign( int a, int b );
	int _getBitNum( unsigned short number );

//  CACHE
//	cache stores int value for every edge in the vertex field
    // alloc cache x*y*z
    int*    _cacheAlloc( int fieldX, int fieldY, int fieldZ );
    // free cache
    void    _cacheFree();
    // set cache to -1
    void    _cacheClear();

    int     _cacheVertex( MarchingCubes::Vertex* vert, int x, int y, int z, int e );
    // params: cube position (x,y,z), edge index
    // returns: cache index for given edge
    int     _cacheOffsetFromCubeEdge( int x, int y, int z, int e );
	int		_cacheOffsetFromPlane( int x, int y, int z, int plane );

	// cache field int[]
    int*    cacheField;
    // cache size x, y, z
    int     cacheSizeX;
    int     cacheSizeY;
    int     cacheSizeZ;
    // cache size x*y*z * 4[int]
    int     cacheSize;

    int     vertexNum;

public:
    MarchingCubes( VoxelField& f );

    // inits entire geometry
    void init();

    // sets corner values
    void setValues( Cube2& cube );	//float vert[8] );

    MarchingCubesCase&    getCase( int code ){
        return triangleTable[code];
    }
    MarchingCubesCase&    getCaseFromValues(){
        int code = _bitsToCode( vertex );
        return getCase( code );
    }

    // copy triangles for current case
    int     fillInTriangles( MarchingCubes::TriangleF tris[8] );
//    int     fillInAllTriangles( MarchingCubes::TriangleF* tris, int maxTris );
    int     fillInTrianglesIndexed( MarchingCubes::Vertex* vert, int maxVert, MarchingCubes::TriangleI* tris, int maxTris, int& vertexNum, int& triNum );

    int     getUsageStats( int i ) { return usageStats[i]; }

    // just some math helpers to avoid any deps
    static void getCrossProduct( float v1[3], float v2[3], float cross[3] ) {
        cross[0] = (v1[1]*v2[2]) - (v2[1]*v1[2]);
        cross[1] = (v1[2]*v2[0]) - (v2[2]*v1[0]);
        cross[2] = (v1[0]*v2[1]) - (v2[0]*v1[1]);
    }
    static float dotProduct( MarchingCubes::Vector3F v1, MarchingCubes::Vector3F v2 ) {
    return v1.f[0]*v2.f[0] +
            v1.f[1]*v2.f[1] +
            v1.f[2]*v2.f[2];
    }
};
#endif // MARCHINGCUBES_H
