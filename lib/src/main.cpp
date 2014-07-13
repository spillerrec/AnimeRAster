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

#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>

#include "dump/DumpPlane.hpp"
#include "AraImage.hpp"
using namespace std;


QStringList input_formats = QStringList() << "*.dump" << "*.png" << "*.webp";

int main( int argc, char* argv[] ){
	QCoreApplication app( argc, argv );
	
	QStringList args = app.arguments();
	QStringList files;
	
	for( int i=1; i<args.count(); ++i ){
		QFileInfo info( args[i] );
		if( info.isDir() ){
			//Add entire folder
			QDir dir( args[i]);
			auto list = dir.entryInfoList( input_formats );
			for( auto file : list )
				files << file.filePath();
		}
		else if( input_formats.contains( "*." + info.suffix() ) )
			files << args[i];
	}
	
	
	int i = 1;
	for( auto file : files ){
		AraImage img;
		QFileInfo info( file );
		
		cout << "[" << i++ << "/" << files.count() << "] Processing: " << file.toLocal8Bit().constData() << "\n";
		QFile f( file );
		QString new_name = info.dir().absolutePath() + "/" + info.baseName() + ".ara";
		QString temp_name = new_name + ".temp";
		
		//Read
		if( f.open( QIODevice::ReadOnly ) ){
			if( info.suffix() == "dump" ){
				vector<DumpPlane> dumps;
				while( true ){
					DumpPlane p;
					if( !p.read( f ) )
						break;
					dumps.emplace_back( p );
				}
				
				img.initFromDump( dumps );
			}
			else{
				//Read ordinary raster image
				QImage qimg;
				qimg.load( (QIODevice*)&f, info.suffix().toUtf8() );
				
				img.initFromQImage( qimg );
			}
			f.close();
		}
		
		//TODO: skip if not loaded
		
		//Debug
		img.outputPlanes().save( "planes.png" );
		
		//Write
		QFile copy( temp_name );
		if( copy.open( QIODevice::WriteOnly ) ){
			img.write( copy );
			copy.close();
		}
		
		if( QFileInfo( temp_name ).size() > 0 ){
		//	QFile::remove( file );
			QFile::remove( new_name );
			QFile::rename( temp_name, new_name );
		}
	}
	
	return 0;
}