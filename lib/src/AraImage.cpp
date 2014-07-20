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


int AraImage::normal_predict( int, unsigned, unsigned ) const{
	return pow( 2, depth-1 );
}

int AraImage::sub_predict( int plane, unsigned x, unsigned y ) const{
	if( x == 0 )
		return normal_predict( plane, x ,y );
	return planes[plane].value( x-1, y );
}

int AraImage::up_predict( int plane, unsigned x, unsigned y ) const{
	if( y == 0 )
		return normal_predict( plane, x,y );
	return planes[plane].value( x, y-1 );
}

int AraImage::avg_predict( int plane, unsigned x, unsigned y ) const{
	if( x == 0 || y == 0 )
		return up_predict( plane, x, y );
	return ( planes[plane].value( x, y-1 ) + planes[plane].value( x-1, y ) ) / 2;
}

int AraImage::avg_right_predict( int plane, unsigned x, unsigned y ) const{
	if( x == planes[plane].width-1 || y == 0 )
		return up_predict( plane, x, y );
	return ( planes[plane].value( x, y-1 ) + planes[plane].value( x+1, y ) ) / 2;
}

int AraImage::up_right_predict( int plane, unsigned x, unsigned y ) const{
	if( x == planes[plane].width-1 || y == 0 )
		return up_predict( plane, x, y );
	return planes[plane].value( x+1, y-1 );
}

int AraImage::up_left_predict( int plane, unsigned x, unsigned y ) const{
	if( x == 0 || y == 0 )
		return up_predict( plane, x, y );
	return planes[plane].value( x-1, y-1 );
}

int AraImage::paeth_predict( int plane, unsigned x, unsigned y ) const{
	if( x == 0 || y == 0 )
		return up_predict( plane, x, y );
	
	auto a = planes[plane].value( x-1, y );
	auto b = planes[plane].value( x, y-1 );
	auto c = planes[plane].value( x-1, y-1 );
	
	auto p = a + b - c;
	auto pa = abs( p - a );
	auto pb = abs( p - b );
	auto pc = abs( p - c );
	
	if( pa <= pb && pa <= pb )
		return a;
	else if( pb <= pc )
		return b;
	else
		return c;
}

int AraImage::paeth_right_predict( int plane, unsigned x, unsigned y ) const{
	if( x == planes[plane].width-1 || y == 0 )
		return up_predict( plane, x, y );
	
	auto a = planes[plane].value( x+1, y );
	auto b = planes[plane].value( x, y-1 );
	auto c = planes[plane].value( x+1, y-1 );
	
	auto p = a + b - c;
	auto pa = abs( p - a );
	auto pb = abs( p - b );
	auto pc = abs( p - c );
	
	if( pa <= pb && pa <= pb )
		return a;
	else if( pb <= pc )
		return b;
	else
		return c;
}

int AraImage::right_predict( int plane, unsigned x, unsigned y ) const{
	if( x == planes[plane].width-1 )
		return normal_predict( plane, x,y );
	return planes[plane].value( x+1, y );
}

int AraImage::prev_predict( int plane, unsigned x, unsigned y ) const{
	if( plane == 0 || x == 0 )
		return normal_predict( plane, x, y );
	return planes[plane-1].value( x-1, y );
}

int AraImage::strange_predict( int plane, unsigned x, unsigned y ) const{
	if( x == 0 || y == 0 )
		return up_predict( plane, x, y );
	
	auto a = planes[plane].value( x-1, y );
	auto b = planes[plane].value( x, y-1 );
	auto c = planes[plane].value( x-1, y-1 );
	
	if( a < b && b < c )
		return paeth_predict( plane, x, y );
	else
		return a + b - c;
}

int AraImage::strange_right_predict( int plane, unsigned x, unsigned y ) const{
	if( x == planes[plane].width-1 || y == 0 )
		return up_predict( plane, x, y );
	
	auto a = planes[plane].value( x+1, y );
	auto b = planes[plane].value( x, y-1 );
	auto c = planes[plane].value( x+1, y-1 );
	
	if( a < b && b < c )
		return paeth_right_predict( plane, x, y );
	else
		return a + b - c;
}

int AraImage::diff_predict( int plane, unsigned x, unsigned y, int dx, int dy ) const{
	return planes[plane].value( x + dx, y + dy );
}


int AraImage::color_predict( int p1, int p2, unsigned x, unsigned y ) const{
	return planes[p1].value( x, y ) - planes[p2].value( x, y );
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
		channels = GRAY;
		AraPlane plane( width, height );
		
		for( unsigned iy=0; iy<height; iy++ )
			for( unsigned ix=0; ix<width; ix++ )
				plane.setValue( ix, iy, qGray( img.pixel( ix, iy ) ) );
		
		planes.emplace_back( plane );
	}
	else{
		channels = RGB;
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
	
	channels = YUV;
	sub_sampling = 1; //TODO:
	//TODO: figure out chroma-subsambling
}

bool AraImage::read( QIODevice &dev ){
	//TODO: read magic
	auto m1 = read8( dev );
	auto m2 = read8( dev );
	auto m3 = read8( dev );
	if( m1 != m3 && m1 != 'a' && m2 != 'r' )
		return false;
	
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
	
	//skip if header is longer than read
	dev.seek( header_lenght + 6 ); //TODO: doesn't work for sequential devices

	//Read compressed data
	data_length = read32( dev );
	vector<uint8_t> buf( data_length );
	dev.read( (char*)buf.data(), buf.size() );
	
	auto data = lzmaDecompress( buf );
//	auto data = zpaqDecompress( buf );
	
	//TODO: read data
	unsigned lines = 0;
	for( unsigned p=0; p<plane_amount(); p++ )
		lines += plane_height( p );
	
	switch( compression ){
		case NONE:{
				vector<uint8_t> types( NORMAL, lines );
				read_lines( types, data, 0 );
				break;
			};
			
		case LINES:{
				vector<uint8_t> types;
				for( unsigned i=0; i<lines; i++ )
					types.push_back( data[i] );
				read_lines( types, data, lines );
				break;
			};
			
		case BLOCKS:{
				read_blocks( data );
				break;
			};
		
		default: cout << "Can't decompress! " << compression << endl;
	};
	
	outputPlanes().save( "readdata.png" );
	
	return false;
}

void AraImage::read_lines( vector<uint8_t> types, vector<uint8_t> data, unsigned offset ){
	planes.clear();
	
	debug_types( types );
	
	unsigned t = 0;
	unsigned limit = pow( 2, depth );
	
	for( unsigned p=0; p<plane_amount(); p++ ){
		planes.emplace_back( AraPlane( plane_width( p ), plane_height( p ) ) );
		
		for( unsigned iy=0; iy<planes[p].height; iy++ ){
			auto f = getFilter( (Type)types[t++] );
			
			for( unsigned ix=0; ix<planes[p].width; ix++ ){
				unsigned val = data[offset++] + 128;//(this->*f)( p, ix, iy );
				planes[p].setValue( ix, iy, val % limit );
			}
		}
		
	}
}

void AraImage::read_blocks( int plane
	,	unsigned width, unsigned height, unsigned x, unsigned y
	,	vector<uint8_t> data, unsigned& pos
	,	vector<uint8_t> types, unsigned& type_pos
	,	unsigned block_size ){
	
	unsigned blocks = ceil( width / (double)block_size ) * ceil( height / (double)block_size );
	unsigned limit = pow( 2, depth );
	
	unsigned ix=x, iy=y;
	for( unsigned b=0; b<blocks; b++ ){
	//	cout << ix << "x" << iy << endl;
		auto type = types[type_pos++];
	//	cout << "Type: "  << (int)type << endl;
		if( type == MULTI )
			read_blocks( plane, block_size, block_size, ix, iy, data, pos, types, type_pos, block_size/2 );
		else{
		//	int dx=0, dy=0; //Diff later...
			auto f = getFilter( (Type)type );
			
			//Truncate sizes to not go out of the plane
			unsigned b_w = min( ix+block_size, plane_width(plane) );
			unsigned b_h = min( iy+block_size, plane_height(plane) );
			
			for( unsigned jy=iy; jy < b_h; jy++ )
				for( unsigned jx=ix; jx < b_w; jx++ ){
				//	cout << "j: " << jx << "x" << jy << endl;
					unsigned val = data[pos++] + 128;// + (this->*f)( plane, jx, jy );
					planes[plane].setValue( jx, jy, val % limit );
				}
		}
		
		//Update position
		ix += block_size;
		if( ix >= width ){
			ix = x;
			iy += block_size;
		}
	}
	
}

void AraImage::read_blocks( vector<uint8_t> data ){
	unsigned pos = 0;
	
	unsigned block_size = data[pos++];
	cout << "Block size: " << block_size << endl;
	vector<uint8_t> types;
	vector<uint8_t> settings;
	
	unsigned blocks_count = 0;
	for( unsigned p=0; p<plane_amount(); p++ )
		blocks_count += ceil( plane_width(p) / (double)block_size ) * ceil( plane_height(p) / (double)block_size );
	cout << "Block amount: " << blocks_count << endl;
	
	unsigned settings_count = 0;
	for( unsigned i=0; i<blocks_count; i++ ){
		auto type = data[pos++];
		if( type == MULTI )
			blocks_count += 4;
		if( type == DIFF )
			settings_count += 2;
		types.push_back( type );
	}
	debug_types( types );
	
	for( unsigned i=0; i<settings_count; i++ )
		settings.push_back( data[pos++] );
	
	unsigned type_pos = 0;
	unsigned settings_pos = 0;
	vector<unsigned> decrease_size;
	for( unsigned p=0; p<plane_amount(); p++ ){
		planes.push_back( AraPlane( plane_width(p), plane_height(p) ) );
		read_blocks( p, plane_width(p), plane_height(p), 0,0, data, pos, types, type_pos, block_size );
	}
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
	
	
	//Generate output data
	vector<uint8_t> data;
	switch( compression ){
		case NONE:   data = compress_none(); break;
		case LINES:  data = compress_lines(); break;
		case BLOCKS: data = make_optimal_configuration(); //compress_blocks( config ); break;
	};
	
	//Write compressed data
//	auto out = data;
	auto out = lzmaCompress( data );
//	auto out = zpaqCompress( data );
	write32( dev, data_length = out.size() );
	dev.write( (char*)out.data(), data_length );
	
	
	outputPlanes();

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

AraImage::Chunk AraImage::compress_lines_chunk( int p, unsigned y, unsigned amount, int enabled_types ) const{
	Chunk chunk;
	
	for( unsigned iy=y; iy<min(y+amount, planes[p].height); iy++ ){
		//Try all the possibilities
		AraLine best( NORMAL, iy, *this, p );
		AraLine sub( SUB, iy, *this, p );
		
		//Pick the one which has the smallest sum
		if( sub.count < best.count && enabled_types & SUB_ON )
			best = sub;
		
		if( iy > 0 ){
			AraLine up( UP, iy, *this, p );
			AraLine avg( AVG, iy, *this, p );
			AraLine paeth( PAETH, iy, *this, p );
			
			if( up.count < best.count && enabled_types & UP_ON )
				best = up;
			if( avg.count < best.count && enabled_types & AVG_ON )
				best = avg;
			if( paeth.count < best.count && enabled_types & PAETH_ON )
				best = paeth;
		}
		
		//Write it out
		chunk.types.push_back( best.type ); //TODO: temp
		for( auto val : best.data )
			chunk.data.push_back( val );
	}
	
	return chunk;
}

vector<uint8_t> AraImage::compress_lines() const{
	vector<int> out;
	vector<uint8_t> types;
	
	unsigned amount = 1; //TODO:
	vector<int> enabled_types{ ALL_ON
		,	ALL_ON & ~NORMAL_ON
		,	ALL_ON & ~SUB_ON
		,	ALL_ON & ~UP_ON
		,	ALL_ON & ~AVG_ON
		,	ALL_ON & ~PAETH_ON
		
		//*
		,	ALL_ON & ~SUB_ON & ~UP_ON
		,	ALL_ON & ~AVG_ON & ~UP_ON
		,	ALL_ON & ~AVG_ON & ~SUB_ON
		//*/
		};
	
	for( unsigned p=0; p<planes.size(); p++ )
		for( unsigned iy=0; iy<planes[p].height; iy+=amount ){
			vector<Chunk> chunks;
			
			for( auto types : enabled_types )
				chunks.emplace_back( compress_lines_chunk( p, iy, amount, types ) );
			
			Chunk best;
			unsigned size = -1;
			for( auto chunk : chunks ){
				auto data = depth > 8 ? packTo16bit( chunk.data ) : packTo8bit( chunk.data );
				auto current = lzmaCompress( data ).size();
				if( current < size ){
					best = chunk;
					size = current;
				}
			}
			
			//Write it out
			for( auto val : best.types )
				types.push_back( val );
			for( auto val : best.data )
				out.push_back( val );
			
			cout << "Progress: " << iy << " of " << planes[p].height << " in plane " << p << endl;
		}
	
	auto data = depth > 8 ? packTo16bit( out ) : packTo8bit( out );
	cout << "Type amount: " << types.size() << endl;
	for( auto dat : data )
		types.push_back( dat );
	
	return types;
}

vector<uint8_t> AraImage::compress_blocks( Config config ) const{
	if( channels == RGB )
		return compressColorBlocks( config );
	
	vector<int> out;
	vector<uint8_t> types{ config.block_size };
	vector<uint8_t> settings;
	
	Entropy entropy;
	
	for( unsigned p=0; p<planes.size(); p++ )
		for( unsigned iy=0; iy<planes[p].height; iy+=config.block_size ){
			vector<AraBlock> blocks;
			
			//Make blocks
			for( unsigned ix=0; ix<planes[p].width; ix+=config.block_size ){
				auto block = best_block( ix, iy, p, config, entropy );
				entropy.add( block.entropy );
				blocks.emplace_back( block );
			}
			
			//Output
			for( auto block : blocks )
				for( auto data : block.data )
					out.push_back( data );
			
			for( auto block : blocks ){
				for( auto type : block.types )
					types.push_back( type );
				for( auto set : block.settings )
					settings.push_back( set );
			}
		}
	
	vector<uint8_t> packed;
	if( depth > 8 )
		packed = packTo16bit( out );
	else
		packed = packTo8bit( out );
	
//	cout << "count size: " << lzmaCompress( data ).size() << " - ?" << endl;
//	cout << "\ttypes size:    " << lzmaCompress( types ).size() << "\t - " << types.size() << endl;
//	cout << "\tsettings size: " << lzmaCompress( settings ).size() << "\t - " << settings.size() << endl;
	
	vector<uint8_t> data;
	//Add settings
	if( config.save_settings ){
	//	cout << "Types size: " << types.size() << endl;
	//	cout << "Settings size: " << settings.size() << endl;
	
		for( auto type : types )
			data.push_back( type );
		for( auto set : settings )
			data.push_back( set );
	}
	
	for( auto pack : packed )
		data.push_back( pack );
	
	return data;
}

vector<uint8_t> AraImage::compressColorBlocks( Config config ) const{
	vector<int> out, out2, out3;
	vector<uint8_t> types{ config.block_size };
	vector<uint8_t> settings;
	
	Entropy entropy;
	
	for( unsigned iy=0; iy<height; iy+=config.block_size ){
		cout << "line: " << iy << endl;
		for( unsigned ix=0; ix<width; ix+=config.block_size ){
			auto block = bestColorBlock( ix, iy, config, entropy );
			entropy.add( block.entropy );
			
			for( auto data : block.data_1 )
				out.push_back( data );
			for( auto data : block.data_2 )
				out2.push_back( data );
			for( auto data : block.data_3 )
				out3.push_back( data );
			types.push_back( block.type );
			settings.push_back( block.ctype );
		}
	}
	
	for( auto data : out2 )
		out.push_back( data );
	for( auto data : out2 )
		out.push_back( data );
	
	vector<uint8_t> packed;
	if( depth > 8 )
		packed = packTo16bit( out );
	else
		packed = packTo8bit( out );
	
//	cout << "count size: " << lzmaCompress( data ).size() << " - ?" << endl;
//	cout << "\ttypes size:    " << lzmaCompress( types ).size() << "\t - " << types.size() << endl;
//	cout << "\tsettings size: " << lzmaCompress( settings ).size() << "\t - " << settings.size() << endl;
	
	vector<uint8_t> data;
	//Add settings
	if( config.save_settings ){
	//	cout << "Types size: " << types.size() << endl;
	//	cout << "Settings size: " << settings.size() << endl;
	
		for( auto type : types )
			data.push_back( type );
		for( auto set : settings )
			data.push_back( set );
	}
	
	for( auto pack : packed )
		data.push_back( pack );
	
	return data;
}

AraImage::Chunk AraImage::compress_blocks_sub( unsigned p
	,	unsigned x, unsigned y, unsigned amount
	,	int enabled_types, AraImage::Config config
	) const{
	Chunk chunk;
	config.types = (EnabledTypes)enabled_types;
	
	for( unsigned iy=y; iy<min(y+config.block_size*amount, planes[p].height); iy+=config.block_size ){
		vector<AraBlock> blocks;
		
		//Make blocks
		for( unsigned ix=x; ix<planes[p].width; ix+=config.block_size )
			blocks.emplace_back( best_block( ix, iy, p, config, Entropy() ) );
		
		//Output
		for( auto block : blocks )
			for( auto data : block.data )
				chunk.data.push_back( data );
		
		for( auto block : blocks ){
			for( auto type : block.types )
				chunk.types.push_back( type );
			for( auto set : block.settings )
				chunk.settings.push_back( set );
		}
	}
	
	return chunk;
}

vector<uint8_t> AraImage::compress_blocks_extreme( AraImage::Config config ) const{
	vector<int> out;
	vector<uint8_t> types{ config.block_size };
	vector<uint8_t> settings;
	
	unsigned amount = 1;
	auto base = config.types;
	vector<int> enabled_types{
		//*
			base & ~AVG_ON & ~SUB_ON
		,	base & ~AVG_ON & ~UP_ON
		,	base & ~SUB_ON & ~UP_ON
		//*/
		
		,	base & ~PAETH_ON
		,	base & ~AVG_ON
		,	base & ~UP_ON
		,	base & ~SUB_ON
		,	base & ~DIFF_ON
		,	base & ~NORMAL_ON
		
		,	base
		};
	
	for( unsigned p=0; p<planes.size(); p++ )
		for( unsigned iy=0; iy<planes[p].height; iy+=config.block_size*amount ){
			Chunk best;
			unsigned best_size = -1;
			cout << "Line: " << iy << endl;
			for( auto t : enabled_types ){
				auto chunk = compress_blocks_sub( p, 0, iy, amount, t, config );
				
				vector<uint8_t> packed;
				if( depth > 8 )
					packed = packTo16bit( chunk.data );
				else
					packed = packTo8bit( chunk.data );
				auto new_size = lzmaCompress( packed ).size();
				if( new_size < best_size ){
					best_size = new_size;
			//		cout << "Picking new best chunk" << best_size << endl;
					best = chunk;
				}
			//	else if( new_size == best_size )
			//		cout << "Same Size" << endl;
			}
			
			//Output
			for( auto data : best.data )
				out.push_back( data );
			for( auto type : best.types )
				types.push_back( type );
			for( auto set : best.settings )
				settings.push_back( set );
		}
	
	vector<uint8_t> packed;
	if( depth > 8 )
		packed = packTo16bit( out );
	else
		packed = packTo8bit( out );
	
	vector<uint8_t> data;
	//Add settings
	if( config.save_settings ){
	//	cout << "Types size: " << types.size() << endl;
	//	cout << "Settings size: " << settings.size() << endl;
	
		for( auto type : types )
			data.push_back( type );
		for( auto set : settings )
			data.push_back( set );
	}
	
	for( auto pack : packed )
		data.push_back( pack );
	
	return data;
}

unsigned offset_dif( unsigned x, unsigned y, int dx, int dy, unsigned width, unsigned height, const AraImage& img, int plane ){
	unsigned count = 0;
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ )
			count += img.diff_filter( plane, ix, iy, dx, dy );
	
	return count;
}


AraImage::AraBlock::AraBlock( AraImage::Type t, unsigned x, unsigned y, unsigned size
	, const AraImage& img, int plane )
	:	AraBlock( t, x, y, img, plane, size ) {
	auto filter = img.getFilter( t );
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ ){
			auto val = img.planes[plane].value( ix, iy ) - (img.*filter)( plane, ix, iy );
			data.emplace_back( val );
			entropy.add( val );
			count += abs( val );
		}
	
	//Color decorrelation
	if( img.channels == RGB && plane != 1 ){ //TODO: do properly
		entropy = Entropy();
		count = 0;
		data.clear();
		for( unsigned iy=y; iy < y+height; iy++ )
			for( unsigned ix=x; ix < x+width; ix++ ){
				auto val = img.planes[1].value( ix, iy ) - (img.*filter)( 1, ix, iy );
				val -= img.planes[plane].value( ix, iy ) - (img.*filter)( plane, ix, iy );
				data.emplace_back( val );
				entropy.add( val );
				count += abs( val );
			}
	}
}


AraImage::AraBlock::AraBlock( unsigned x, unsigned y, const AraImage& img, int plane, AraImage::Config config )
	:	AraBlock( DIFF, x, y, img, plane, config.block_size ) {
	auto p_width = img.planes[plane].width;
	auto p_height = img.planes[plane].height;
	
	count = INT_MAX;
	double best = count; //TODO: dbl_max
	
	unsigned min_x = max( 0, (int)x - (int)config.search_size+1 );
	unsigned min_y = max( 0, (int)y - (int)config.search_size+1 );
	unsigned max_x = min( x+1, p_width-width );
	unsigned max_y = min( y+1, p_height-height );
	if( config.both_directions ){
		max_x = min( x + config.search_size, p_width-width );
		max_y = min( y + config.search_size, p_height-height );
	}
	
	unsigned bx = 0, by = 0;
	for( unsigned iy=min_y; iy < max_y; iy++ )
		for( unsigned ix=min_x; ix < max_x; ix++ ){
			if( ix == x && iy == y )
				continue;
			
			auto current = offset_dif( x, y, ix-x, iy-y, width, height, img, plane );
			
			double sum = img.weight_setting( (int)x-(int)bx ) + img.weight_setting( (int)y-(int)by );
			double w = img.weight( types.size(), current, 2, sum );
	//		cout << w << "\t" << current << endl;
			if( w < best ){
				count = current;
				best = w;
				bx = ix;
				by = iy;
			}
		}
	
//	cout << "Best: " << (int)bx-(int)x << "x" << (int)by-(int)y << endl;
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ )
			data.push_back( img.diff_filter( plane, ix, iy, (int)bx-(int)x, (int)by-(int)y ) );
	
	settings.push_back( (int)x-(int)bx + config.diff_save_offset );
	settings.push_back( (int)y-(int)by + config.diff_save_offset );
}

AraImage::AraBlock AraImage::makeMulti( unsigned x, unsigned y, int plane, AraImage::Config config, const Entropy& base ) const{
	AraBlock multi( MULTI, x, y, *this, plane, config.block_size );
	config.block_size /= 2;
	unsigned size = config.block_size;
	
	//Prevent it from creating it, if it cannot make all four blocks
	//TODO: Make this more flexible
	if( multi.width < size || multi.height < size || size < config.min_block_size ){
		multi.count = INT_MAX;
		return multi;
	}
	
	//Find the best one for each four pieces
	vector<AraBlock> blocks;
	blocks.emplace_back( best_block( x,      y,      plane, config, base ) );
	blocks.emplace_back( best_block( x+size, y,      plane, config, base ) );
	blocks.emplace_back( best_block( x,      y+size, plane, config, base ) );
	blocks.emplace_back( best_block( x+size, y+size, plane, config, base ) );
	
	//Combine all data into one block
	for( auto block : blocks )
		for( auto data : block.data )
			multi.data.push_back( data );
	
	//Add types
	for( auto block : blocks )
		for( auto type : block.types )
			multi.types.push_back( type );
	
	//Add any settings from the blocks
	for( auto block : blocks )
		for( auto setting : block.settings )
			multi.settings.push_back( setting );
	
	//Add count
	for( auto block : blocks )
		multi.count += block.count;
	multi.count *= config.multi_penalty;
	
	return multi;
}

double AraImage::weight_setting( int setting ) const{
	auto val = ceil(log2( setting+1 )) * setting_log_weight;
	return val * val;
}

double AraImage::weight( const AraImage::AraBlock& block, const Entropy& base ) const{
	return block.count;
//	return base.entropy( block.entropy );
	
	double settings_sum = 0.0;
	for( auto setting : block.settings )
		settings_sum += weight_setting( setting );
//	if( block.settings.size() != 0 )
//		cout << settings_sum * setting_log_weight << "\t" << block.count * count_weight << endl;
	
	return weight( block.types.size(), block.count, block.settings.size(), settings_sum );
}

double AraImage::weight( unsigned type_count, unsigned count, unsigned settings_count, double settings_sum ) const{
	return type_count * type_weight
		+	count * count_weight
		+	settings_count * setting_lin_weight
		+	settings_sum;
}

AraImage::AraBlock AraImage::best_block( unsigned x, unsigned y, int plane, AraImage::Config config, const Entropy& base ) const{
	auto types = config.types;
	
	AraBlock best( NORMAL, x, y, config.block_size, *this, plane );
	
	for( unsigned t=0; t<TYPE_COUNT-2; t++ ){
		if( isTypeOn( t, types ) ){
			if( !( typeIsRight( t ) && !config.both_directions ) ){
				AraBlock block( (Type)t, x, y, config.block_size, *this, plane );
				if( weight(block, base) < weight(best, base) )
					best = block;
			}
		}
	}
	
	/*
	if( types & MULTI_ON ){
		AraBlock multi = makeMulti( x, y, plane, config, base );
		if( weight(multi, base) < weight(best, base) )
			best = multi;
	}
	
	if( types & DIFF_ON ){
		AraBlock diff( x, y, *this, plane, config );
		if( weight(diff, base) < weight(best, base) )
			best = diff;
	}
	*/
	
	return best;
}

AraImage::AraColorBlock AraImage::bestColorBlock( unsigned x, unsigned y, AraImage::Config config, const Entropy& base ) const{
	auto types = config.types;
	
	AraColorBlock best( *this, NORMAL, x, y, config.block_size, base );
	
	for( unsigned t=0; t<TYPE_COUNT-2; t++ ){
		if( isTypeOn( t, types ) ){
			if( !( typeIsRight( t ) && !config.both_directions ) ){
				AraColorBlock block( *this, (Type)t, x, y, config.block_size, base );
				if( block.weight < best.weight )
					best = block;
			}
		}
	}
	
	return best;
}

std::vector<uint8_t> AraImage::make_optimal_configuration() const{
	vector<uint8_t> best;
	vector<uint8_t> buf;
	
	Config config;
	config.block_size = 8;
	config.min_block_size = 4;
	config.search_size = 4;
	config.types = EnabledTypes(ALL_ON & ~DIFF_ON & ~PAETH_ON /*& ~RIGHT_ON*/ & ~PREV_ON );
	config.multi_penalty = 0.1;
	config.both_directions = false;
	config.save_settings = true;
	config.diff_save_offset = 0;
	
		//Try several settings, and use best one
		for( unsigned i=8; i<=8; i*=2 )
			for( unsigned j=i; j>=i; j/=2 )
				for( unsigned k=2; k<=2; k*=2 )
//				for( double m=0.00; m<1.5; m+=0.25 )
				for( double l=0.1; l<1.0; l+=10.3 )
				{
//					type_weight = m;//0.025;
//					count_weight = m;
//					setting_lin_weight = m;
//					setting_log_weight = m;
//					config.multi_penalty = 0.1;
					config.block_size = i;
					config.min_block_size = j;
					config.multi_penalty = l;
					config.search_size = k;
			
				//	auto data = compress_blocks_extreme( config );
					auto data = compress_blocks( config );
					auto current = lzmaCompress( data );
					
					cout << "Result: " << (int)config.block_size << "-" << (int)config.min_block_size << ": \t" << current.size();
					if( current.size() < best.size() || best.size() == 0 ){
						best = current;
						buf = data;
						
						cout  << " !";
					}
					cout << "  \t" << k;
					cout << "  \t" << l;
//					cout << "  \t" << m;
					cout << endl;
					
					if( config.block_size == config.min_block_size )
						break;
				}
	
	return buf;
}

void AraImage::debug_types( vector<uint8_t> types ) const{
	vector<unsigned> counts( 16, 0 );
	
	for( auto type : types )
		counts[type]++;
	
	cout << "Types used:" << endl;
	for( auto count : counts )
		cout << "\t" << (int)count << endl;
}


AraImage::AraColorBlock::AraColorBlock( const AraImage& img, Type t, unsigned x, unsigned y, unsigned size, const Entropy& base )
	:	type(t) {
	vector<AraBlock> blocks{ AraBlock( t, x, y, size, img, 0 )
		,	AraBlock( t, x, y, size, img, 1 )
		,	AraBlock( t, x, y, size, img, 2 )
		};
	
	//Start with C___
	double best = img.weight( blocks[0], base ) + img.weight( blocks[1], base ) + img.weight( blocks[2], base );
	ctype = C___;
	AraBlock out1( blocks[0] ), out2( blocks[1] ), out3( blocks[2] );
	
	//Main color iteration
	for( int main=0; main<3; main++ ){
		int second = ( main != 0 ) ? 0 : 1;
		int third = ( main != 1 && second != 1 ) ? 1 : 2;
		
		double current = img.weight( blocks[main], base );
		
		// subtract main main
		auto b_mm1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_mm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_mm = current + img.weight( b_mm1, base ) + img.weight( b_mm2, base );
		if( w_mm < best ){
			best = w_mm;
			ctype = (CType)( main*3 + 1 );
			out1 = blocks[main];
			out2 = b_mm1;
			out3 = b_mm2;
		}
		
		// subtract main second
		auto b_ms1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_ms2 = AraBlock::subtract( blocks[third], blocks[second] );
		double w_ms = current + img.weight( b_ms1, base ) + img.weight( b_ms2, base );
		if( w_ms < best ){
			best = w_ms;
			ctype = (CType)( main*3 + 2 );
			out1 = blocks[main];
			out2 = b_ms1;
			out3 = b_ms2;
		}
		
		// subtract third main
		auto b_tm1 = AraBlock::subtract( blocks[second], blocks[third] );
		auto b_tm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_tm = current + img.weight( b_tm1, base ) + img.weight( b_tm2, base );
		if( w_tm < best ){
			best = w_tm;
			ctype = (CType)( main*3 + 3 );
			out1 = blocks[main];
			out2 = b_tm1;
			out3 = b_tm2;
		}
	}
	
	//Set data
	data_1 = out1.data;
	data_2 = out2.data;
	data_3 = out3.data;
	
	//Set statistics
	count = out1.count + out2.count + out3.count;
	entropy.add( out1.entropy );
	entropy.add( out2.entropy );
	entropy.add( out3.entropy );
	weight = best;
}


