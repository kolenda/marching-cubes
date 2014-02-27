
#include "VoxelField.h"

VoxelField::VoxelField() {
	field = NULL;
	planeSize = 0.0f;
	setExtent( 10, 10, 10 );
};
VoxelField::VoxelField( int x, int y, int z ) {
	VoxelField();
	setSize( x, y, z );
};
VoxelField::~VoxelField() {
	if( field )
		delete[] field;
};

void VoxelField::setSize( int x, int y, int z ) {
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
void VoxelField::setExtent( float x, float y, float z ) {
	if( x < 1 || y < 1 || z < 1 )
		return;
	extentX = x;
	extentY = y;
	extentZ = z;
};

bool VoxelField::_outsideOf( int val, int min, int max ) {
	if( val < min || val >= max )
		return true;
	return false;
}
bool VoxelField::setVal( int x, int y, int z, float val ) {
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
	VoxelField::getVal( int x, int y, int z, float* val ) {

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

void VoxelField::setAllValues( float val ) {
	if( field )
		for( int i = 0; i < sizeX*sizeY*sizeZ; i++ )
			field[i] = val;
}

// this method gets proper values forming a cube and returns it in a helper class
Cube2 VoxelField::getCube( int x, int y, int z ) {
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
void VoxelField::addSphere( float fx, float fy, float fz, float frad ) {
	int startX = 0; //						max( 0.0f, fx-frad );
	int startY = 0; //						max( 0.0f, fy-frad );
	int startZ = 0; //						max( 0.0f, fz-frad );
	int stopX  = sizeX; //						min( (float)sizeX, fx+frad );
	int stopY  = sizeY; //						min( (float)sizeY, fy+frad );
	int stopZ  = sizeZ; //						min( (float)sizeZ, fz+frad );

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

void VoxelField::setAmbiguousCase( int num )
{
	if( num == 0 ) {
		setSize( 3, 2, 2 );
		setVal( 0,0,0, -5 );
		setVal( 0,0,1, 10 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, -5 );

		setVal( 1,0,0, 10 );
		setVal( 1,0,1, -5 );
		setVal( 1,1,0, -5 );
		setVal( 1,1,1, 10 );

	/*
		cf.setVal( 2,0,0, -5 );
		cf.setVal( 2,0,1, 10 );
		cf.setVal( 2,1,0, 10 );
		cf.setVal( 2,1,1, -5 );
	/*/
		setVal( 2,0,0, 10 );
		setVal( 2,0,1, 10 );
		setVal( 2,1,0, 10 );
		setVal( 2,1,1, 10 );
	}
	else if( num == 1 ) {
		setSize( 3, 2, 2 );
		setVal( 0,0,0, 10 );
		setVal( 0,0,1, 10 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, 10 );

		setVal( 1,0,0, 10 );
		setVal( 1,0,1, -5 );
		setVal( 1,1,0, -5 );
		setVal( 1,1,1, 10 );

		setVal( 2,0,0, -5 );
		setVal( 2,0,1, 10 );
		setVal( 2,1,0, 10 );
		setVal( 2,1,1, -5 );
	}

	else if( num == 2 ) {
		setSize( 2, 3, 2 );
		setVal( 0,0,0, -5 );
		setVal( 0,0,1, 10 );
		setVal( 1,0,0, 10 );
		setVal( 1,0,1, -5 );

		setVal( 0,1,0, 10 );
		setVal( 0,1,1, -5 );
		setVal( 1,1,0, -5 );
		setVal( 1,1,1, 10 );

		setVal( 0,2,0, 10 );
		setVal( 0,2,1, 10 );
		setVal( 1,2,0, 10 );
		setVal( 1,2,1, 10 );
	}
	else if( num == 3 ) {
		setSize( 2, 3, 2 );
		setVal( 0,0,0, 10 );
		setVal( 0,0,1, 10 );
		setVal( 1,0,0, 10 );
		setVal( 1,0,1, 10 );

		setVal( 0,1,0, 10 );
		setVal( 0,1,1, -5 );
		setVal( 1,1,0, -5 );
		setVal( 1,1,1, 10 );

		setVal( 0,2,0, -5 );
		setVal( 0,2,1, 10 );
		setVal( 1,2,0, 10 );
		setVal( 1,2,1, -5 );
	}



	else if( num == 4 ) {
		setSize( 2, 2, 3 );
		setVal( 0,0,0, -5 );
		setVal( 1,0,0, 10 );
		setVal( 0,1,0, 10 );
		setVal( 1,1,0, -5 );

		setVal( 0,0,1, 10 );
		setVal( 1,0,1, -5 );
		setVal( 0,1,1, -5 );
		setVal( 1,1,1, 10 );

		setVal( 0,0,2, 10 );
		setVal( 1,0,2, 10 );
		setVal( 0,1,2, 10 );
		setVal( 1,1,2, 10 );
	}
	else if( num == 5 ) {
		setSize( 2, 2, 3 );
		setVal( 0,0,0, 10 );
		setVal( 1,0,0, 10 );
		setVal( 0,1,0, 10 );
		setVal( 1,1,0, 10 );

		setVal( 0,0,1, 10 );
		setVal( 1,0,1, -5 );
		setVal( 0,1,1, -5 );
		setVal( 1,1,1, 10 );

		setVal( 0,0,2, -5 );
		setVal( 1,0,2, 10 );
		setVal( 0,1,2, 10 );
		setVal( 1,1,2, -5 );
	}

}
