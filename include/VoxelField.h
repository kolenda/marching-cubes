/*
    VoxelField - class implementing simple 3D voxel field
    Author: Karol Herda
    Web:    http://kolenda.vipserv.org/algorithmic-marching-cubes/
    Date:   12-03-2013

    Contains 3D field of float values and manages memory allocation, setting grid size, etc.
*/
#ifndef VOXELFIELD_H_INCLUDED
#define VOXELFIELD_H_INCLUDED

#include <vector>
#include <math.h>
#include <assert.h>
#include <string.h>

using namespace std;

/*
    Cube - helper class that stores 8 corners as float values
*/
class Cube {
public:
    float   vec[8];
public:
    Cube( float val[8] ) {
        memcpy( vec, val, 8*sizeof(float) );
    };

    // 'empty' means that entire cube is on the one side of the surface
    // so the cube doesn't have any geometry
    bool notEmpty() {

    	for( int i = 1; i < 8; i++ ) {
			if( vec[0] * vec[i] < 0.0f )
				return true;
    	}
        return false;
    }
};


class Cube2 {
private:
    float*   vec;
    int sizeX, sizeY, sizeZ;

public:
    Cube2( float* val ) {
        vec = val;
    };

	void setGridSize( int x, int y, int z ) {
		sizeX = x;
		sizeY = y;
		sizeZ = z;
	}

    float getVec( int i )
    {
		int offset = 0;
		if( i & 0x01 )
			offset ++;	//= sizeX;
		if( i & 0x02 )
			offset += sizeX;
		if( i & 0x04 )
			offset += sizeX * sizeY;

		return vec[offset];
    }

    // 'empty' means that entire cube is on the one side of the surface
    // so the cube doesn't have any geometry
    bool notEmpty()
    {
    	float v0 = vec[0];
    	float v1 = vec[1];
		if( v0 * v1 < 0.0f )
			return true;

    	float v10 = vec[sizeX];
		if( v0 * v10 < 0.0f )
			return true;

    	float v11 = vec[sizeX+1];
		if( v0 * v11 < 0.0f )
			return true;

    	float v100 = vec[sizeX*sizeY];
		if( v0 * v100 < 0.0f )
			return true;

    	float v101 = vec[sizeX*sizeY+1];
		if( v0 * v101 < 0.0f )
			return true;

    	float v110 = vec[sizeX*sizeY+sizeX];
		if( v0 * v110 < 0.0f )
			return true;

    	float v111 = vec[sizeX*sizeY+sizeX+1];
		if( v0 * v111 < 0.0f )
			return true;


/*    	for( int i = 1; i < 8; i++ ) {
    		int offset = 0;
    		if( i & 0x01 )
				offset ++;	//= sizeX;
    		if( i & 0x02 )
				offset += sizeX;
    		if( i & 0x04 )
				offset += sizeX*sizeY;

			if( vec[0] * vec[offset] < 0.0f )
				return true;
    	}*/
        return false;
    }
};

class VoxelField   {
    // pointer to values data
    float*      field;

    // number of elements along each axis
    int         sizeX, sizeY, sizeZ;

    // sizeX*sizeY - to save some computations
    int         planeSize;

    // it's not really used ATM
    float       extentX, extentY, extentZ;

	inline double	findnoise2(double x,double y)
	{
		int n=(int)x+(int)y*57;
		n=(n<<13)^n;
		int nn=(n*(n*n*60493+19990303)+1376312589)&0x7fffffff;
		return 1.0-((double)nn/1073741824.0);
	}
	inline double	interpolate(double a,double b,double x)
	{
		double ft=x * 3.1415927;
		double f=(1.0-cos(ft))* 0.5;
		return a*(1.0-f)+b*f;
	}


	double	noise( double x, double y );

public:
    VoxelField();
    VoxelField( int x, int y, int z );
    ~VoxelField();

    void setSize( int x, int y, int z );
    void setExtent( float x, float y, float z );

    bool _outsideOf( int val, int min, int max );
    bool setVal( int x, int y, int z, float val );
    void	//bool
		getVal( int x, int y, int z, float* val );

    void    setAllValues( float val );

    // this method gets proper values forming a cube and returns it in a helper class
    Cube2    getCube( int x, int y, int z );

    // Function to create object representing a sphere
    //      it's meant to create spherical objects in voxel space,
    //      but mathematically it's just a linear function of distance from center
    void addSphere( float fx, float fy, float fz, float frad );

	void	setAmbiguousCase( int num );
	void	setSnake( int num );
	void	setSpheres( float phase );
	void	setPerlinNoise( int num );
	void	setZeroSlice();

    int getSizeX() { return sizeX; }
    int getSizeY() { return sizeY; }
    int getSizeZ() { return sizeZ; }

    float getExtentX() { return extentX; }
    float getExtentY() { return extentY; }
    float getExtentZ() { return extentZ; }
};
#endif // VOXELFIELD_H_INCLUDED
