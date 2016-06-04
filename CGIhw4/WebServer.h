#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <dirent.h>
#include "CGI.h" 

#define BUFSIZE 4096
#define QLEN    5

using std::string ;

class WebServer
{
	public:
		void Boot(int Port, string rootDir) ;
		
		int PassiveSock(int port) ;
		
		string ContentType(string fileName) ;
		
		void RequestHandler(int Socket) ;
		
		bool isWebPageExit(string webPage) ;
		
		bool IsFile(string Path) ;
	
		void StaticObject(string webPage) ;
	
		void Directory(string webPage, DIR* thisDir) ;
		
		
	private:
		CGI CGIhandler ;

} ;


#endif //* WEBSERVER_H *//