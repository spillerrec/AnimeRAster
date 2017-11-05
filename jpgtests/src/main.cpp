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


auto load( QString path, auto func ){
	QFile file( path );
	if( !file.open( QIODevice::ReadOnly ) ){
		qDebug() << "Could not open file" << path;
		std::exit( -1 );
	}
	
	return func( file );
};

static int extract_coeffs( QStringList files ){
	//Save each i,j index of the coeffs as a seperate image
	auto jpg = load( files[0], from_jpeg );
	for( unsigned i=0; i<jpg.planes.size(); i++ )
		planeToQImage( jpg.planes[i].toPlane() ).save( "original" + QString::number(i) + ".png" );
	
	//* JPEG saved as PNG
	PngImage png{ QImage(files[0]) };
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
}

static int multi_img_encode( QStringList files ){
	//Encode several jpeg images in one compressed block
	// Multi-image encode
	vector<JpegImage> imgs;
	imgs.reserve( files.count() );
	for( auto filepath : files ){
		imgs.push_back( load( filepath, from_jpeg ) );
	}
	writeData( "multiple.bin", multiJpegEncode( imgs ) );
	return 0;
}

static int single_img_encode( QStringList files ){
	//Encode each file seperately and do statistics
	CsvFile csv( "results.csv" );
	csv.addLine( "File", "Jpg-size", "block", "planar", "best" );
	
	for( auto filepath : files ){
		qDebug() << filepath;
		auto img = load( filepath, from_jpeg );
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
		
		csv.addLine( QFileInfo(filepath).fileName().toUtf8().constData(), QFileInfo( filepath ).size(), data1.size(), data2.size(), data.size() );
	}
	
	return 0;
}

template<typename T>
std::pair<int,int> pixel_diff_count( const Overmix::PlaneBase<T>& p1, const Overmix::PlaneBase<T>& p2 ){
	auto size = p1.getSize().min( p2.getSize() );
	int count = 0;
	for( unsigned iy=0; iy<size.height(); iy++ ){
		auto r1 = p1.scan_line( iy );
		auto r2 = p2.scan_line( iy );
		for( unsigned ix=0; ix<size.width(); ix++ ){
			if( r1[ix] == r2[ix] )
				count++;
		}
	}
	
	return { count, size.width() * size.height() };
}

static int dct_check( QString filepath ){
	
	auto coeffs = load( filepath, from_jpeg );
	auto libjpeg = load( filepath, from_jpeg_decode );
	planeToQImage( libjpeg[0] ).save( "dct_check_libjpeg.png" );
	planeToQImage( coeffs.planes[0].toPlane() ).save( "dct_check_overmix.png" );
	
	auto spacial_diff = pixel_diff_count( libjpeg[0], coeffs.planes[0].toPlane() );
	qDebug() << "Difference spartial:" << spacial_diff.first << "/" << spacial_diff.second << "=" << double(spacial_diff.first) / spacial_diff.second * 100 << "%";
	
	
	return 0;
}


int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	//Expand folders and remove all other than .jpg files
	QStringList args = app.arguments();
	args.pop_front();
	auto files = expandFolders( ".", args, {"*.jpg"} );
	
	return dct_check( files[0] );
	return extract_coeffs( files );
	//*/
	return multi_img_encode( files );
	//*/
	return single_img_encode( files );
	
	return 0;
}
