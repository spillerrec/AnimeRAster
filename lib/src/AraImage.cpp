/*	This file is part of AnimeRAster.

	AnimeRAster is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	AnimeRAster is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with AnimeRAster.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AraImage.hpp"

#include "device.hpp"
#include "transform.hpp"

using namespace std;


double blockWeight( const AraImage::AraBlock& block, const Entropy& base ){
	return block.count;
//	return base.entropy( block.entropy );
}

QImage ValuePlane::asImage() const{
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


void AraImage::initFromQImage( QImage img ){
	planes.clear();
	width = img.width();
	height = img.height();
	depth = 8;
	
	if( img.allGray() ){
		channels = GRAY;
		ValuePlane plane( width, height, depth );
		
		for( unsigned iy=0; iy<height; iy++ )
			for( unsigned ix=0; ix<width; ix++ )
				plane.setValue( ix, iy, qGray( img.pixel( ix, iy ) ) );
		
		planes.emplace_back( plane );
	}
	else{
		channels = RGB;
		ValuePlane r( width, height, depth );
		ValuePlane g( width, height, depth );
		ValuePlane b( width, height, depth );
		
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
		
		ValuePlane p( dump.getWidth(), dump.getHeight(), depth );
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
				readNone( data );
				break;
			};
			
		case BLOCKS:{
				if( channels == RGB )
					readColorBlocks( data );
				else
					readBlocks( data );
				break;
			};
		
		default: cout << "Can't decompress! " << compression << endl;
	};
	
	outputPlanes().save( "readdata.png" );
	
	return false;
}

void AraImage::readNone( vector<uint8_t> data ){
	planes.clear();
	unsigned pos = 0;
	
	for( unsigned p=0; p<plane_amount(); p++ ){
		planes.emplace_back( ValuePlane( plane_width( p ), plane_height( p ), depth ) );
		
		for( unsigned iy=0; iy<planes[p].height; iy++ )
			for( unsigned ix=0; ix<planes[p].width; ix++ )
				planes[p].setValue( ix, iy, data[pos++] );	
	}
}

void AraImage::readBlocks( vector<uint8_t> data ){
	unsigned pos = 0;
	unsigned block_size = data[pos++];
	
	vector<unsigned> type_offset;
	for( unsigned p=0; p<plane_amount(); p++ ){
		type_offset.push_back( pos );
		pos += ceil( plane_width(p) / (double)block_size ) * ceil( plane_height(p) / (double)block_size );
	}
	
	for( unsigned p=0; p<plane_amount(); p++ ){
		planes.emplace_back( ValuePlane( plane_width(p), plane_height(p), depth ) );
		planes[p].loadBlocks( data, block_size, type_offset[p], pos );
	}
}


void AraImage::readColorBlocks( vector<uint8_t> data ){
	PixelPlane image( width, height, depth );
	image.load( data );
	
	for( unsigned i=0; i<3; i++ ){
		ValuePlane plane( width, height, depth );
		for( unsigned iy=0; iy<height; iy++ )
			for( unsigned ix=0; ix<width; ix++ )
				plane.setValue( ix, iy, image.value( ix, iy ).color[i] );
		planes.emplace_back( plane );
	}
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


QImage AraImage::outputImage() const{
	QImage img( width, height, QImage::Format_RGB32 );
	img.fill( 0 );
	
	if( channels == RGB ){
		unsigned depth_offset = depth - 8;
		for( unsigned iy=0; iy<height; iy++ ){
			auto row = (QRgb*)img.scanLine( iy );
			for( unsigned ix=0; ix<width; ix++ ){
				row[ix] = qRgb(
						planes[0].value(ix,iy) >> depth_offset
					,	planes[1].value(ix,iy) >> depth_offset
					,	planes[2].value(ix,iy) >> depth_offset
					);
			}
		}
	}
	else{
		cout << "YUV images not yet implemented!" << endl;
		return QImage();
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

vector<uint8_t> AraImage::compress_blocks( Config config ) const{
	if( channels == RGB )
		return compressColorBlocks( config );
	/*
	//Disable right filters
	unsigned types = config.types;
	for( unsigned t=0; t<ValuePlane::TYPE_COUNT-2; t++ )
		if( ( ValuePlane::typeIsRight( t ) && !config.both_directions ) )
			types &= ~ValuePlane::enabledType( t );
	
	
	vector<uint8_t> out{ config.block_size };
	out.reserve( width * height * planes.size() );
	for( unsigned p=0; p<planes.size(); p++ )
		for( auto val : planes[p].saveBlocks( config.block_size, (ValuePlane::EnabledTypes)types ) )
			out.emplace_back( val );
		
	return out;
/*/
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
//*/
}

vector<uint8_t> AraImage::compressColorBlocks( Config config ) const{
	PixelPlane image( width, height, depth );
	for( unsigned iy=0; iy<height; iy++ )
		for( unsigned ix=0; ix<width; ix++ ){
			Pixel p;
			for( unsigned i=0; i<3; i++ )
				p.color[i] = planes[i].value( ix, iy );
			image.setValue( ix, iy, p );
		}
	return image.save();
	/*
	vector<int> out, out2, out3;
	vector<int> types{ config.block_size };
	vector<int> settings;
	
	Entropy entropy;
	
	for( unsigned iy=0; iy<height; iy+=config.block_size ){
	//	cout << "line: " << iy << endl;
		for( unsigned ix=0; ix<width; ix+=config.block_size ){
			auto block = bestColorBlock( ix, iy, config, entropy );
			entropy.add( block.entropy );
			
			for( auto data : block.data )
				out.push_back( data );
			types.push_back( block.type );
			types.push_back( block.ctype );
			for( auto set: block.settings )
				settings.push_back( set );
		//	cout << (int)block.settings[0] << "\t" << (int)block.type << endl;
		//	if( block.settings[0] != 0 )
		//		cout << (int)block.type << endl;
		}
	}
	
	vector<uint8_t> packed;
	if( depth > 8 )
		packed = packTo16bit( out );
	else
		packed = packTo8bit( out );
	
//	cout << "count size: " << lzmaCompress( packed ).size() << " - ?" << endl;
//	cout << "\ttypes size:    " << lzmaCompress( types ).size() << "\t - " << types.size() << endl;
//	cout << "\tsettings size: " << lzmaCompress( settings ).size() << "\t - " << settings.size() << endl;
	
	vector<uint8_t> data;
	//Add settings
	if( config.save_settings ){
	//	cout << "Types size: " << types.size() << endl;
	//	cout << "Settings size: " << settings.size() << endl;
		
//		Remap type_map( types );
//		Remap color_map( settings );
		
		for( auto type : types )
			data.push_back( type );
//			data.push_back( type_map.translate( type ) );
		for( auto set : settings )
			data.push_back( set );
//			data.push_back( color_map.translate( set ) );
	}
	
	for( auto pack : packed )
		data.push_back( pack );
	
	return data;
	*/
}


AraImage::AraBlock::AraBlock( AraImage::Type t, unsigned x, unsigned y, unsigned size
	, const AraImage& img, int plane, int dx, int dy )
	:	AraBlock( t, x, y, img, plane, size ) {
	auto filter = img.getFilter( t );
	for( unsigned iy=y; iy < y+height; iy++ )
		for( unsigned ix=x; ix < x+width; ix++ ){
			if( t == NORMAL && ( dx != 0 || dy != 0 ) ){
				auto val = img.planes[plane].value( ix, iy ) - img.planes[plane].value( ix+dx, iy+dy );
				data.emplace_back( val );
				entropy.add( val );
				count += abs( val );
			}
			else{
				auto val = img.planes[plane].value( ix, iy ) - (img.*filter)( plane, ix+dx, iy+dy );
				data.emplace_back( val );
				entropy.add( val );
				count += abs( val );
			}
		}
}

AraImage::AraBlock AraImage::best_block( unsigned x, unsigned y, int plane, AraImage::Config config, const Entropy& base ) const{
	auto types = config.types;
	
	AraBlock best( NORMAL, x, y, config.block_size, *this, plane );
	
	for( unsigned t=0; t<TYPE_COUNT-2; t++ ){
		if( isTypeOn( t, types ) ){
			if( !( typeIsRight( t ) && !config.both_directions ) ){
				AraBlock block( (Type)t, x, y, config.block_size, *this, plane );
				if( blockWeight(block, base) < blockWeight(best, base) )
					best = block;
			}
		}
	}
	
	return best;
}

AraImage::AraColorBlock AraImage::bestColorBlock( unsigned x, unsigned y, AraImage::Config config, const Entropy& base ) const{
	auto types = config.types;
	
	AraColorBlock best( *this, NORMAL, x, y, config.block_size, base );
	
	for( unsigned t=0; t<TYPE_COUNT-2; t++ ){
		if( isTypeOn( t, types ) ){
			if( !( typeIsRight( t ) && !config.both_directions ) ){
				AraColorBlock block( *this, (Type)t, x, y, config.block_size, base );
		//		cout << "w: " << block.weight << endl;
				if( block.weight < best.weight )
					best = block;
			}
		}
	}
	//			cout << "-------------" << endl;
	
	return best;
}

std::vector<uint8_t> AraImage::make_optimal_configuration() const{
	vector<uint8_t> best;
	vector<uint8_t> buf;
	
	Config config;
	config.block_size = 8;
	config.min_block_size = 4;
	config.search_size = 4;
//	config.types = EnabledTypes(ALL_ON & ~DIFF_ON & ~PAETH_ON & ~UP_ON & ~SUB_ON /*& ~RIGHT_ON*/ & ~PREV_ON );
//	config.types = EnabledTypes(NORMAL_ON | AVG_ON | STRANGE_ON | PAETH_ON);
	config.types = EnabledTypes(NORMAL_ON);
//	config.types = EnabledTypes(ALL_ON & ~DIFF_ON & ~UP_ON & ~SUB_ON & ~UP_LEFT_ON & ~RIGHT_ON & ~UP_RIGHT_ON & ~PREV_ON );
	config.multi_penalty = 0.1;
	config.both_directions = false;
	config.save_settings = true;
	config.diff_save_offset = 0;
	
		//Try several settings, and use best one
		for( unsigned i=4; i<=8; i*=2 )
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

int box_position( int x, int y ){
	int line = max( x, y );
	return line * (2 + 2*(line-1)) / 2 + y + line - x;
}

AraImage::AraColorBlock::AraColorBlock( const AraImage& img, Type t, unsigned x, unsigned y, unsigned size, const Entropy& base )
	:	type(t) {
	vector<AraBlock> blocks{ AraBlock( t, x, y, size, img, 0 )
		,	AraBlock( t, x, y, size, img, 1 )
		,	AraBlock( t, x, y, size, img, 2 )
		};
	
	//Start with untouched colors
	double best = blockWeight( blocks[0], base ) + blockWeight( blocks[1], base ) + blockWeight( blocks[2], base );
	double best1 = blockWeight( blocks[0], base );
	double best_zero = best1;
	int best_x=0, best_y=0;
	/*
	for( int dy=0; dy<8; dy++ )
		for( int dx=-8; dx<8; dx++ ){
			if( int(x)-dx < 0 || int(y)-dy < 0 )
				continue;
		//	if( dx == 1 || dy == 1 || dx == -1 )
		//		continue;
			vector<AraBlock> test{ AraBlock( t, x, y, size, img, 0, -dx, -dy )
				,	AraBlock( t, x, y, size, img, 1, -dx, -dy )
				,	AraBlock( t, x, y, size, img, 2, -dx, -dy )
				};
			
			double current = blockWeight( test[0], base ) + blockWeight( test[1], base ) + blockWeight( test[2], base );
			if( current + abs(best_x) + abs(best_y) < best1*0.75 ){ //TODO: weight?
				best_x = dx;
				best_y = dy;
				best1 = current;
			}
		}*/
//	if( best1 < best_zero )
//		cout << best_x << "x" << best_y << " - " << best1 << " < " << best_zero << endl;
//	settings.push_back( best_x );
//	settings.push_back( best_y );
//	settings.push_back( best_x << 4 + best_y );
//	settings.push_back( box_position( best_x, best_y ) );
	
	blocks = { { t, x, y, size, img, 0, -best_x, -best_y }
		,	{ t, x, y, size, img, 1, -best_x, -best_y }
		,	{ t, x, y, size, img, 2, -best_x, -best_y }
		};
	
	ctype = 0;
	vector<AraBlock> out = blocks;
	
	//Main color iteration
	for( int main=0; main<0; main++ ){
		int second = ( main != 0 ) ? 0 : 1;
		int third = ( main != 1 && second != 1 ) ? 1 : 2;
		
		double current = blockWeight( blocks[main], base );
		
		// subtract main main
		auto b_mm1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_mm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_mm = current + blockWeight( b_mm1, base ) + blockWeight( b_mm2, base );
		if( w_mm < best ){
			best = w_mm;
			ctype = main*3 + 1;
			out[main]   = blocks[main];
			out[second] = b_mm1;
			out[third]  = b_mm2;
		}
		
		// subtract main second
		auto b_ms1 = AraBlock::subtract( blocks[second], blocks[main] );
		auto b_ms2 = AraBlock::subtract( blocks[third], blocks[second] );
		double w_ms = current + blockWeight( b_ms1, base ) + blockWeight( b_ms2, base );
		if( w_ms < best ){
			best = w_ms;
			ctype = main*3 + 2;
			out[main]   = blocks[main];
			out[second] = b_ms1;
			out[third]  = b_ms2;
		}
		
		// subtract third main
		auto b_tm1 = AraBlock::subtract( blocks[second], blocks[third] );
		auto b_tm2 = AraBlock::subtract( blocks[third], blocks[main] );
		double w_tm = current + blockWeight( b_tm1, base ) + blockWeight( b_tm2, base );
		if( w_tm < best ){
			best = w_tm;
			ctype = main*3 + 3;
			out[main]   = blocks[main];
			out[second] = b_tm1;
			out[third]  = b_tm2;
		}
	}
	
	//Set data
	for( unsigned i=0; i<out[0].data.size(); i++ ){
		data.push_back( out[0].data[i] );
		data.push_back( out[1].data[i] );
		data.push_back( out[2].data[i] );
	}
	
	//Set statistics
	count = out[0].count + out[1].count + out[2].count;
	entropy.add( out[0].entropy );
	entropy.add( out[1].entropy );
	entropy.add( out[2].entropy );
	weight = best;
}


