/*	This file is part of dump-tools.

	dump-tools is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dump-tools is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dump-tools.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AraImage.hpp"

#include "transform.hpp"

using namespace std;


QImage AraPlane::asImage( unsigned depth ) const{
	QImage img( width, height, QImage::Format_RGB32 );
	img.fill( 0 );
	
	for( unsigned iy=0; iy<height; iy++ )
		for( unsigned ix=0; ix<width; ix++ ){
			auto val = int32_t( value( ix, iy ) >> (depth - 8 ) ) % 256;
			img.setPixel( ix, iy, qRgb( val, val, val ) );
		}
	
	return img;
}


int AraImage::normal_filter( int plane, unsigned x, unsigned y ) const{
	return planes[plane].value( x, y );
}

int AraImage::sub_filter( int plane, unsigned x, unsigned y ) const{
	if( x == 0 )
		return normal_filter( plane, x ,y );
	
	return planes[plane].value( x-1, y ) - planes[plane].value( x, y );
}

int AraImage::up_filter( int plane, unsigned x, unsigned y ) const{
	if( x == 0 )
		return normal_filter( plane, x ,y );
	
	return planes[plane].value( x, y-1 ) - planes[plane].value( x, y );
}

int AraImage::avg_filter( int plane, unsigned x, unsigned y ) const{
	if( x == 0 )
		return up_filter( plane, x, y );
	
	int val = ( planes[plane].value( x, y-1 ) + planes[plane].value( x-1, y ) ) / 2;
	return val - planes[plane].value( x, y );
}

int AraImage::diff_filter( int plane, unsigned x, unsigned y, int dx, int dy ) const{
	return planes[plane].value( x, y ) - planes[plane].value( x + dx, y + dy );
}



static uint8_t read8( QIODevice &dev ){
	char byte1 = 0;
	dev.getChar( &byte1 );
	return byte1;
}

static uint16_t read16( QIODevice &dev ){
	char byte1, byte2;
	if( !dev.getChar( &byte1 ) )
		return 0;
	if( !dev.getChar( &byte2 ) )
		return 0;
	return ((uint16_t)byte2 << 8) + (uint8_t)byte1;
}

static uint32_t read32( QIODevice &dev ){
	uint16_t byte1 = read16( dev );
	uint32_t byte2 = read16( dev );
	return (byte2 << 16) + byte1;
}
static float readFloat( QIODevice &dev ){
	float val;
	dev.read( (char*)&val, 4 );
	return val;
}
static void write8( QIODevice &dev, uint8_t val ){
	dev.write( (char*)&val, 1 );
}
static void write16( QIODevice &dev, uint16_t val ){
	dev.write( (char*)&val, 2 );
}
static void write32( QIODevice &dev, uint32_t val ){
	dev.write( (char*)&val, 4 );
}
static void writeFloat( QIODevice &dev, float val ){
	dev.write( (char*)&val, 4 );
}

void AraImage::initFromQImage( QImage img ){
	planes.clear();
	width = img.width();
	height = img.height();
	depth = 8;
	
	if( img.allGray() ){
		AraPlane plane( width, height );
		
		for( unsigned iy=0; iy<height; iy++ )
			for( unsigned ix=0; ix<width; ix++ )
				plane.setValue( ix, iy, qGray( img.pixel( ix, iy ) ) );
		
		planes.emplace_back( plane );
	}
	else{
		AraPlane r( width, height );
		AraPlane g( width, height );
		AraPlane b( width, height );
		
		for( unsigned iy=0; iy<height; iy++ )
			for( unsigned ix=0; ix<width; ix++ ){
				auto rgb = img.pixel( ix, iy );
				r.setValue( ix, iy, qRed( rgb ) );
				g.setValue( ix, iy, qGreen( rgb ) );
				b.setValue( ix, iy, qBlue( rgb ) );
			}
		
		planes.emplace_back( r );
		planes.emplace_back( g );
		planes.emplace_back( b );
	}
	
	//TODO: alpha
}

void AraImage::initFromDump( std::vector<DumpPlane> dumps ){
	planes.clear();
	
	for( auto dump : dumps ){
		width = max( width, dump.getWidth() );
		height = max( height, dump.getHeight() );
		depth = max( depth, dump.getDepth() );
		
		AraPlane p( dump.getWidth(), dump.getHeight() );
		for( unsigned iy=0; iy<p.height; iy++ )
			for( unsigned ix=0; ix<p.width; ix++ )
				p.setValue( ix, iy, dump.get_value( iy, ix ) );
		
		planes.emplace_back( p );
	}
	
	//TODO: figure out chroma-subsambling
}

bool AraImage::read( QIODevice &dev ){
	//TODO: read magic
	auto version = read8( dev );
	if( version > 0 )
		return false;
	
	auto header_lenght = read16( dev );
	
	channels = read8( dev );
	depth = read8( dev );
	sub_sampling = read8( dev );
	color_space = read8( dev );
	compression = (Compression)read8( dev );
	compression_level = read8( dev );
	width = read32( dev );
	height = read32( dev );
	pixel_ratio = readFloat( dev );
	
	//TODO: skip if header is longer than read

	data_length = read32( dev );
	
	//TODO: read data
	
	return false;
}

AraPlane center( AraPlane p ){
	auto stat = statistics( p.data );
	stat.debug();
	p.data = offsetData( p.data, -stat.min );
	return p;
}

bool AraImage::write( QIODevice &dev, Compression level ){
	dev.write( "ara", 3 );
	write8( dev, 0 );
	
	write16( dev, 18 );
	
	write8( dev, channels );
	write8( dev, depth );
	write8( dev, sub_sampling );
	write8( dev, color_space );
	write8( dev, compression=level );
	write8( dev, compression_level=0 );
	write32( dev, width );
	write32( dev, height );
	writeFloat( dev, pixel_ratio );
	
	//Test stuff
	auto copy = planes;
	
//	planes[0].asImage( depth ).save( "l-normal.png" );
	
//	center( planes[1] ).asImage( depth ).save( "u-normal.png" );
//	center( planes[2] ).asImage( depth ).save( "v-normal.png" );
	
//	planes[0].data = offsetData( copy[0].data, invertData( copy[1].data ) );
//	planes[2].data = offsetData( copy[2].data, invertData( copy[1].data ) );
	
//	planes[0] = center( planes[0] );
//	planes[1] = center( planes[1] );
	
//	outputPlanes().save( "test.png" );
	
	
// Works well with RGB
//	planes[1].data = offsetData( copy[1].data, invertData( copy[2].data ) );
//	planes[2].data = offsetData( copy[2].data, invertData( copy[1].data ) );
//	center( planes[1] ).asImage( depth ).save( "u-invert-diff-nooffset.png" );
//	center( planes[2] ).asImage( depth ).save( "v-invert-diff-nooffset.png" );
	
	//Generate output data
	vector<uint8_t> data;
	switch( compression ){
		case NONE:   data = compress_none(); break;
		case LINES:  data = compress_lines(); break;
		case BLOCKS: data = compress_blocks(); break;
	};
	
	//Write compressed data
	auto out = lzmaCompress( data );
	write32( dev, data_length = out.size() );
	dev.write( (char*)out.data(), data_length );
	

	return false;
}

QImage AraImage::outputPlanes() const{
	unsigned out_height = 0;
	for( auto p : planes )
		out_height += p.height;
	
	QImage img( width, out_height, QImage::Format_RGB32 );
	img.fill( 0 );
	
	unsigned offset = 0;
	for( auto p : planes ){
		for( unsigned iy=0; iy<p.height; iy++ )
			for( unsigned ix=0; ix<p.width; ix++ ){
				auto val = p.value( ix, iy ) >> (depth - 8 );
				img.setPixel( ix, iy+offset, qRgb( val, val, val ) );
			}
		
		offset += p.height;
	}
	
	return img;
}


vector<uint8_t> AraImage::compress_none() const{
	vector<int> data;
	for( auto p : planes )
		for( auto val : p.data )
			data.push_back( val );
	
	if( depth > 8 )
		return packTo16bit( data );
	else if( depth == 1 )
		return packTo1bit( data );
	else
		return packTo8bit( data );
}


vector<uint8_t> AraImage::compress_lines() const{
	vector<int> out;
	vector<uint8_t> types;
	
	for( unsigned p=0; p<planes.size(); p++ )
		for( unsigned iy=0; iy<planes[p].height; iy++ ){
			//Try all the possibilities
			AraLine best( NORMAL, iy, *this, p, &AraImage::normal_filter );
			AraLine sub( SUB, iy, *this, p, &AraImage::sub_filter );
			
			//Pick the one which has the smallest sum
			if( sub.count < best.count )
				best = sub;
			
			if( iy > 0 ){
				AraLine up( UP, iy, *this, p, &AraImage::up_filter );
				AraLine avg( AVG, iy, *this, p, &AraImage::avg_filter );
				
				if( up.count < best.count )
					best = up;
				if( avg.count < best.count )
					best = avg;
			}
			
			//Write it out
			types.push_back( best.type ); //TODO: temp
			for( auto val : best.data )
				out.push_back( val );
		}
	
	auto data = depth > 8 ? packTo16bit( out ) : packTo8bit( out );
	cout << "Type amount: " << types.size() << endl;
	for( auto type : types )
		data.push_back( type );
	
	return data;
}

vector<uint8_t> AraImage::compress_blocks() const{
	
}