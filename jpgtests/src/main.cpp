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

#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QDebug>

#include "JpegImage.hpp"

using namespace std;


QImage planeToQImage( const Overmix::PlaneBase<uint8_t>& p ){
	QImage out( p.get_width(), p.get_height(), QImage::Format_RGB32 );
	out.fill( qRgb(0,0,0) );
	
	for( int iy=0; iy<out.height(); iy++ ){
		auto out_row = (QRgb*)out.scanLine( iy );
		auto in      = p.scan_line( iy );
		for( int ix=0; ix<out.width(); ix++ )
			out_row[ix] = qRgb( in[ix], in[ix], in[ix] );
	}
	
	return out;
}

int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList files = app.arguments();
	files.pop_front();
	
	for( auto filepath : files ){
		qDebug() << filepath;
		QFile file( filepath );
		if( !file.open( QIODevice::ReadOnly ) )
			return -1;
		
		auto img = AnimeRaster::from_jpeg( file );
		
		for( unsigned i=0; i<img.planes.size(); i++ )
			planeToQImage( img.planes[i].toPlane() ).save( "test" + QString::number(i) + ".png" );
	}
	
	return 0;
}