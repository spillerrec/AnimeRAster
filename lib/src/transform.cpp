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

#include "transform.hpp"

#include "zpaq/libzpaq.h"
#include <lzma.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;


vector<uint8_t> packTo1bit( const vector<int>& in ){
	vector<uint8_t> out;
	out.reserve( in.size()/8 + 1 );
	
	unsigned buf = 0;
	unsigned count = 0;
	for( unsigned i=0; i<in.size(); i++ ){
		buf += in[i] << (count);
		count++;
		if( count == 8 ){
			out.push_back( buf );
			count = 0;
			buf = 0;
		}
	}
	
	if( buf != 0 )
		out.push_back( buf );
	
	return out;
}

static uint8_t compress4bit( uint8_t high, uint8_t low ){
	return (high << 4) + low;
}

vector<uint8_t> packTo4bit( const vector<int>& in ){
	vector<uint8_t> out;
	out.reserve( in.size()/2 + 1 );
	
	for( unsigned i=0; i<in.size(); i+=2 )
		out.push_back( compress4bit( in[i], in[i+1] ) );
	
	return out;
}

vector<uint8_t> packTo8bit( const vector<int>& in ){
	vector<uint8_t> out;
	out.reserve( in.size() );
	
	for( auto val : in )
		out.push_back( val );
	
	return out;
}

vector<uint8_t> packTo16bit( const vector<int>& in ){
	vector<uint8_t> out;
	out.reserve( in.size()*2 );
	
	for( auto val : in ){
		if( val < 0 )
			val = (-val)*2 + 1;
		else
			val = val*2;
		
		out.push_back( val & 0xFF );
		out.push_back( val >> 8 );
	}
	
	return reorder16bit( out );
}

vector<uint8_t> reorder16bit( const vector<uint8_t>& in ){
	vector<uint8_t> out;
	out.reserve( in.size() );
	
	for( unsigned i=0; i<in.size(); i+=2 )
		out.push_back( in[i] );
	for( unsigned i=1; i<in.size(); i+=2 )
		out.push_back( in[i] );
	
	return out;
}

#include <iostream>
#include <chrono>

// https://gist.github.com/gongzhitaao/7062087
class Timer {
	public:
		Timer() : beg_( clock_::now() ) { }
		void reset() { beg_ = clock_::now(); }
		double elapsed() const { 
			return std::chrono::duration_cast<second_>( clock_::now() - beg_ ).count();
		}

	private:
		typedef std::chrono::high_resolution_clock clock_;
		typedef std::chrono::duration<double, std::ratio<1> > second_;
		std::chrono::time_point<clock_> beg_;
};


vector<uint8_t> lzmaDecompress( const vector<uint8_t>& in ){
	vector<uint8_t> out;
	
	Timer t;
	//Initialize decoder
	lzma_stream strm = LZMA_STREAM_INIT;
	if( lzma_stream_decoder( &strm, UINT64_MAX, 0 ) != LZMA_OK )
		return out;
	
	//Decompress
	strm.next_in = (uint8_t*)in.data();
	strm.avail_in = in.size();
	
	//Output buffer
	vector<uint8_t> buf( 4096 );
	
	while( true ){
		strm.next_out = buf.data();
		strm.avail_out = buf.size();
		
		auto ret = lzma_code( &strm, LZMA_FINISH );
		
		if( strm.avail_out == 0 || ret == LZMA_STREAM_END ){
			auto amount = buf.size() - strm.avail_out;
			for( unsigned i=0; i<amount; i++ )
				out.push_back( buf[i] );
		}
		
		if( ret != LZMA_OK )
			break;
	}
	
	lzma_end(&strm);
	cout << "lzma decompress took: " << t.elapsed() << endl;
	
	
	return out;
}


vector<uint8_t> lzmaCompress( const vector<uint8_t>& in ){
	if( in.size() == 0 )
		return in;
	
	lzma_stream strm = LZMA_STREAM_INIT;
	if( lzma_easy_encoder( &strm, 9 | LZMA_PRESET_EXTREME, LZMA_CHECK_CRC64 ) != LZMA_OK )
		return vector<uint8_t>();
	
	strm.next_in = in.data();
	strm.avail_in = in.size();
	
	auto buf_size = strm.avail_in * 2;
	strm.avail_out = buf_size;
	vector<uint8_t> buf( buf_size );
	strm.next_out = buf.data();
	
	lzma_ret ret = lzma_code( &strm, LZMA_FINISH );
	if( ret != LZMA_STREAM_END ){
		cout << "Nooo, didn't finish compressing!" << endl;
		lzma_end( &strm ); //TODO: Use RAII
		return vector<uint8_t>();
	}
	
	auto final_size = buf_size - strm.avail_out;
	buf.resize( final_size );
	
	lzma_end( &strm );
	
	return buf;
}

void libzpaq::error(const char* msg) {  // print message and exit
	cout << "Oops: " << msg << endl;
	exit( 1 );
}

class zpaqIn : public libzpaq::Reader {
	private:
		unsigned pos{ 0 };
		const vector<uint8_t>& data;
		
	public:
		zpaqIn( const vector<uint8_t>& data ) : data( data ) { }
		
		int read( char* buf, int n ){
			unsigned amount = min( data.size() - pos, (vector<uint8_t>::size_type)n );
			memcpy( buf, data.data()+pos, amount );
			pos += amount;
			return amount;
		}
		
		int get(){ return pos < data.size() ? data[pos++] : -1; }
};

class zpaqOut : public libzpaq::Writer {
	public:
		vector<uint8_t> data;
		
		void write( char* buf, int n ){
			unsigned pos = data.size();
			data.resize( pos + n );
			memcpy( data.data()+pos, buf, n );
		}
		
		void put( int c ){ data.push_back( c ); }
};

vector<uint8_t> zpaqDecompress( const vector<uint8_t>& input ){
	zpaqIn in( input );
	zpaqOut out;
	
	Timer t;
	libzpaq::decompress( &in, &out );
	cout << "zpaq decompress took: " << t.elapsed() << endl;
	
	return out.data;
}

vector<uint8_t> zpaqCompress( const vector<uint8_t>& input ){
	zpaqIn in( input );
	zpaqOut out;
	
	libzpaq::compress( &in, &out, 3 );
	
	return out.data;
}


Statistics statistics( const std::vector<int>& data ){
	Statistics stats;
	
	for( auto val : data ){
		stats.min = min( stats.min, val );
		stats.max = max( stats.max, val );
		stats.avg += val;
	}
	stats.avg /= data.size();
	
	for( auto val : data )
		stats.variance += abs( val - stats.avg ); //TODO: squared?
	
	return stats;
}

vector<int> offsetData( const vector<int>& in, int offset ){
	vector<int> out;
	out.reserve( in.size() );
	
	for( auto val : in )
		out.emplace_back( val + offset );
	
	return out;
}

vector<int> offsetData( const std::vector<int>& data, const vector<int>& offsets ){
	vector<int> out;
	out.reserve( data.size() );
	
	for( unsigned i=0; i<data.size(); i++ )
		out.emplace_back( data[i] + offsets[i] );
	
	return out;
}

vector<int> invertData( const vector<int>& in ){
	vector<int> out;
	out.reserve( in.size() );
	
	for( auto val : in )
		out.emplace_back( -val );
	
	return out;
}


