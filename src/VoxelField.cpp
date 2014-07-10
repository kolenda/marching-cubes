#include "VoxelField.h"
#include "simplexnoise1234.h"

VoxelField::VoxelField()
{
	field = NULL;
	planeSize = 0.0f;
	setExtent( 10, 10, 10 );
}

VoxelField::VoxelField( int x, int y, int z )
{
	VoxelField();
	setSize( x, y, z );
}

VoxelField::~VoxelField()
{
	if( field )
		delete[] field;
}

void VoxelField::setSize( int x, int y, int z )
{
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
}

void VoxelField::setExtent( float x, float y, float z )
{
	if( x < 1 || y < 1 || z < 1 )
		return;

	extentX = x;
	extentY = y;
	extentZ = z;
}

bool VoxelField::_outsideOf( int val, int min, int max )
{
	if( val < min || val >= max )
		return true;
	return false;
}

bool VoxelField::setVal( int x, int y, int z, float val )
{
//	parameter checks, disabled for performance

//	if( !field )
//		return false;

//	if( _outsideOf(x,0,sizeX) ||
//		_outsideOf(y,0,sizeY) ||
//		_outsideOf(z,0,sizeZ) )
//		return false;

	field[ planeSize*z + sizeX*y + x ] = val;

	return true;
}

void VoxelField::getVal( int x, int y, int z, float* val )
{
//	parameter checks, disabled for performance
//	assert( field );
//	if( !field )
//		return false;

//	if( _outsideOf(x,0,sizeX) ||
//		_outsideOf(y,0,sizeY) ||
//		_outsideOf(z,0,sizeZ) )
//		return false;

	*val = field[ planeSize*z + sizeX*y + x ];
}

void VoxelField::setAllValues( float val )
{
	if( field )
		for( int i = 0; i < sizeX*sizeY*sizeZ; i++ )
			field[i] = val;
}

// this method gets proper values forming a cube and returns it in a helper class
Cube2 VoxelField::getCube( int x, int y, int z )
{
	Cube2 c( field + planeSize*z + sizeX*y + x );
	return c;
}

// Function to create object representing a sphere
//      it's meant to create spherical objects in voxel space,
//      but mathematically it's just a linear function of distance from center
void VoxelField::addSphere( float fx, float fy, float fz, float frad )
{
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

void VoxelField::setSnake( int num )
{
	setSize( 2, 2, 2 );
	if( num == 0 ) {
		setVal( 0,0,0, -5 );
		setVal( 0,0,1, -5 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, -5 );

		setVal( 1,0,0, 10 );
		setVal( 1,0,1, 10 );
		setVal( 1,1,0, 10 );
		setVal( 1,1,1, -5 );
	}
	else if( num == 1 ) {
		setVal( 1,0,0, -5 );
		setVal( 1,0,1, -5 );
		setVal( 1,1,0, 10 );
		setVal( 1,1,1, -5 );

		setVal( 0,0,0, 10 );
		setVal( 0,0,1, 10 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, -5 );
	}
	else if( num == 2 ) {
		setVal( 0,1,0, -5 );
		setVal( 0,1,1, -5 );
		setVal( 0,0,0, 10 );
		setVal( 0,0,1, -5 );

		setVal( 1,1,0, 10 );
		setVal( 1,1,1, 10 );
		setVal( 1,0,0, 10 );
		setVal( 1,0,1, -5 );
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

		setVal( 2,0,0, 10 );
		setVal( 2,0,1, 10 );
		setVal( 2,1,0, 10 );
		setVal( 2,1,1, 10 );
	}
	else if( num == 1 ) {
		setSize( 3, 2, 2 );
		setVal( 2,0,0, -5 );
		setVal( 2,0,1, 10 );
		setVal( 2,1,0, 10 );
		setVal( 2,1,1, -5 );

		setVal( 1,0,0, 10 );
		setVal( 1,0,1, -5 );
		setVal( 1,1,0, -5 );
		setVal( 1,1,1, 10 );

		setVal( 0,0,0, 10 );
		setVal( 0,0,1, 10 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, 10 );
	}
	else if( num == 2 ) {
		setSize( 3, 2, 2 );
		setVal( 0,0,0, 5 );
		setVal( 0,0,1, -10 );
		setVal( 0,1,0, -10 );
		setVal( 0,1,1, 5 );

		setVal( 1,0,0, -10 );
		setVal( 1,0,1, 5 );
		setVal( 1,1,0, 5 );
		setVal( 1,1,1, -10 );

		setVal( 2,0,0, 10 );
		setVal( 2,0,1, 10 );
		setVal( 2,1,0, 10 );
		setVal( 2,1,1, 10 );
	}
	else if( num == 3 ) {
		setSize( 3, 2, 2 );
		setVal( 2,0,0, 5 );
		setVal( 2,0,1, -10 );
		setVal( 2,1,0, -10 );
		setVal( 2,1,1, 5 );

		setVal( 1,0,0, -10 );
		setVal( 1,0,1, 5 );
		setVal( 1,1,0, 5 );
		setVal( 1,1,1, -10 );

		setVal( 0,0,0, 10 );
		setVal( 0,0,1, 10 );
		setVal( 0,1,0, 10 );
		setVal( 0,1,1, 10 );
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

void VoxelField::setSpheres( float phase )
{
	setAllValues( -0.5f );
	float x = 0.3 * getSizeX() * sin(phase*0.1) + 0.5 * getSizeX();
	float y = 0.3 * getSizeY() * cos(phase*0.2) + 0.5 * getSizeY();
	float z = 0.3 * getSizeZ() * sin(1+phase*0.15) + 0.5 * getSizeZ();
	float rad = getSizeX() / 3;
	addSphere( x, y, z, rad );
	addSphere( y, z, x, rad );
	addSphere( z, x, y, rad );

	addSphere( z, y, x, rad );
	addSphere( y, x, z, rad );
	addSphere( x, z, y, rad );
}

void VoxelField::setPerlinNoise( int num )
{
	float scale = 0.1f;
	for( int xx = 0; xx < sizeX; xx++ ) {
		for( int yy = 0; yy < sizeY; yy++ ) {
			for( int zz = 0; zz < sizeZ; zz++ ) {
				float val = snoise3( (float)xx*scale, (float)yy*scale, (float)zz*scale );
//				val += (1.0f/127.8f);
				setVal( xx, yy, zz, val );
			}
		}
	}
}

void VoxelField::setZeroSlice()
{
	setSize( 3, 3, 3 );

	for( int x = 0; x < 3; x++ )
	for( int y = 0; y < 3; y++ )
	for( int z = 0; z < 3; z++ )
	{
		int sum = x+y+z;
		if( sum < 3 )
			setVal( x,y,z, -1.0f );
		else if( sum > 3 )
			setVal( x,y,z, 1.0f );
		else
			setVal( x,y,z, 0.0f );
	}
}
