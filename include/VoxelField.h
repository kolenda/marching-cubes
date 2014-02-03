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

public:
    VoxelField() {
        field = NULL;
        planeSize = 0.0f;
        setExtent( 10, 10, 10 );
    };
    VoxelField( int x, int y, int z ) {
        VoxelField();
        setSize( x, y, z );
    };
    ~VoxelField() {
        if( field )
            delete[] field;
    };

    void setSize( int x, int y, int z ) {
        if( field ) {
            delete[] field;
            field = 0;
        }
        if( x < 1 || y < 1 || z < 1 )
            return;
        sizeX = x;
        sizeY = y;
        sizeZ = z;
        planeSize = x*y;
        field = new float[sizeX * sizeY * sizeZ];
    };
    void setExtent( float x, float y, float z ) {
        if( x < 1 || y < 1 || z < 1 )
            return;
        extentX = x;
        extentY = y;
        extentZ = z;
    };

    bool _outsideOf( int val, int min, int max ) {
        if( val < min || val >= max )
            return true;
        return false;
    }
    bool setVal( int x, int y, int z, float val ) {
//        if( !field )
//            return false;

//        if( _outsideOf(x,0,sizeX) ||
//            _outsideOf(y,0,sizeY) ||
//            _outsideOf(z,0,sizeZ) )
//            return false;

        field[ planeSize*z + sizeX*y + x ] = val;

        return true;
    }
    void	//bool
		getVal( int x, int y, int z, float* val ) {

//        assert( field );
//        if( !field )
//            return false;

//        assert( !_outsideOf(x,0,sizeX) &&
//            !_outsideOf(y,0,sizeY) &&
//            !_outsideOf(z,0,sizeZ) );

//        if( _outsideOf(x,0,sizeX) ||
  //          _outsideOf(y,0,sizeY) ||
    //        _outsideOf(z,0,sizeZ) )
      //      return false;

        *val = field[ planeSize*z + sizeX*y + x ];
//        return true;
    }

    void    setAllValues( float val ) {
        if( field )
            for( int i = 0; i < sizeX*sizeY*sizeZ; i++ )
                field[i] = val;
    }

    // this method gets proper values forming a cube and returns it in a helper class
    Cube2    getCube( int x, int y, int z ) {
        float   vTab[8];

//        if( x < sizeX-1 && y < sizeY-1 && z < sizeZ-1 ) {
//            getVal( x, y, z, vTab );
//            getVal( x+1, y, z, vTab+1 );
//            getVal( x, y+1, z, vTab+2 );
//            getVal( x+1, y+1, z, vTab+3 );
//            getVal( x, y, z+1, vTab+4 );
//            getVal( x+1, y, z+1, vTab+5 );
//            getVal( x, y+1, z+1, vTab+6 );
//            getVal( x+1, y+1, z+1, vTab+7 );
//        }

        Cube2 c( field + planeSize*z + sizeX*y + x );
        //vTab);
        return c;
    };

    // Function to create object representing a sphere
    //      it's meant to create spherical objects in voxel space,
    //      but mathematically it's just a linear function of distance from center
    void addSphere( float fx, float fy, float fz, float frad ) {
        int startX = //0; //
						max( 0.0f, fx-frad );
        int startY = //0; //
						max( 0.0f, fy-frad );
        int startZ = //0; //
						max( 0.0f, fz-frad );
        int stopX  = //sizeX; //
						min( (float)sizeX, fx+frad );
        int stopY  = //sizeY; //
						min( (float)sizeY, fy+frad );
        int stopZ  = //sizeZ; //
						min( (float)sizeZ, fz+frad );

        //we iterate over all points in voxel space
        for( int xx = startX; xx < stopX; xx++ ) {
            for( int yy = startY; yy < stopY; yy++ ) {
                for( int zz = startZ; zz < stopZ; zz++ ) {
                    //compute distance of point from 'sphere' position
                    float dist = fabs((double)xx-fx)*fabs((double)xx-fx) +
                                fabs((double)yy-fy)*fabs((double)yy-fy) +
                                fabs((double)zz-fz)*fabs((double)zz-fz);
                    dist = sqrt(dist);

                    //if distance is shorter than sphere radius, then add some value to this point
                    if( dist < frad ) {
                        //get value from the field
                        float val = 0;
                        getVal( xx, yy, zz, &val );

                        float diff;
                        if( dist > 0.1f )
                            // function that scales nicely to 0 at the sphere radius
                            diff = max( 0.0f, (frad-dist)/dist );
                        else    // this case is added to prevent x/0 type of errors and to cap max value
                            diff = frad / 0.1f;

                        val += diff;
                        //put the value back
                        setVal( xx, yy, zz, val );
                    }
                }
            }
        }
    }

    int getSizeX() { return sizeX; }
    int getSizeY() { return sizeY; }
    int getSizeZ() { return sizeZ; }

    float getExtentX() { return extentX; }
    float getExtentY() { return extentY; }
    float getExtentZ() { return extentZ; }
};
#endif // VOXELFIELD_H_INCLUDED
