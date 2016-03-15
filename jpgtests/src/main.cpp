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

#include "Converters.hpp"
#include "JpegImage.hpp"
#include "PngImage.hpp"
#include "PlaneExtras.hpp"
#include "Encoders.hpp"
#include "CsvFile.hpp"

using namespace std;
using namespace AnimeRaster;

QStringList expandFolders( QString dir, QStringList files, const QStringList& filters ){
	QStringList out;
	for( auto file : files )
		if( QFileInfo( file ).isDir() )
			out << expandFolders( file, QDir(file).entryList( filters ), filters );
		else
			out << dir + "/" + file;
	return out;
}

static bool writeData( QString filepath, const vector<uint8_t>& data ){
	QFile outfile( filepath );
	if( !outfile.open( QIODevice::WriteOnly ) )
		return false;
	outfile.write( (const char*)data.data(), data.size() ); //TODO: check if succeded?
	return true;
}


int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList args = app.arguments();
	args.pop_front();
	auto files = expandFolders( ".", args, {"*.jpg"} );
	
	CsvFile csv( "results.csv" );
	csv.addLine( "File", "Jpg-size", "block", "planar", "best" );
	
	
	QFile file( args[0] );
	if( !file.open( QIODevice::ReadOnly ) )
		return -1;
	auto jpg = from_jpeg( file );
	for( unsigned i=0; i<jpg.planes.size(); i++ )
		planeToQImage( jpg.planes[i].toPlane() ).save( "original" + QString::number(i) + ".png" );
	
	//* JPEG saved as PNG
	PngImage png{ QImage(args[0]) };
	png.saveRaw( "component-" );
	
	//Back to JPEG
	auto img = png.toImage( jpg.planes[0].quant );
	for( unsigned i=0; i<img.planes.size(); i++ )
		planeToQImage( img.planes[i].toPlane() ).save( "recompressed" + QString::number(i) + ".png" );
	
	for( unsigned iy=0; iy<1; iy++ )
		for( unsigned ix=0; ix<1; ix++ ){
			planeToQImage( normalized( coeffsFromOffset( jpg.planes[0], {ix,iy} ) ) ).save( "jpg-coeff" + QString::number(ix)+ "x" + QString::number(iy) + ".png" );
			planeToQImage( normalized( coeffsFromOffset( img.planes[0], {ix,iy} ) ) ).save( "rec-coeff" + QString::number(ix)+ "x" + QString::number(iy) + ".png" );
		}
	
	return 0;
	//*/
	
	/* Multi-image encode
	vector<JpegImage> imgs;
	imgs.reserve( files.count() );
	for( auto filepath : files ){
		QFile file( filepath );
		if( !file.open( QIODevice::ReadOnly ) )
			return -1;
		
		imgs.push_back( from_jpeg( file ) );
	}
	writeData( "multiple.bin", multiJpegEncode( imgs ) );
	return 0;
	//*/
	
	for( auto filepath : files ){
		qDebug() << filepath;
		QFile file( filepath );
		if( !file.open( QIODevice::ReadOnly ) )
			return -1;
		
		auto img = from_jpeg( file );
		/*
		for( unsigned i=0; i<img.planes.size(); i++ ){
			planeToQImage( img.planes[i].toPlane() ).save( "test" + QString::number(i) + ".png" );
		}
		for( unsigned iy=0; iy<1; iy++ )
			for( unsigned ix=0; ix<1; ix++ )
				planeToQImage( normalized( coeffsFromOffset( img.planes[0], {ix,iy} ) ) ).save( "coeff" + QString::number(ix)+ "x" + QString::number(iy) + ".png" );
		*/
		auto data1 = blockJpegEncode( img );
		auto data2 = planarJpegEncode( img );
		auto data = (data1.size()<data2.size()) ? data1 : data2;
		
		writeData( "compressed.bin", data );
		
		csv.addLine( QFileInfo(filepath).fileName().toUtf8().constData(), file.size(), data1.size(), data2.size(), data.size() );
	}
	
	return 0;
}
