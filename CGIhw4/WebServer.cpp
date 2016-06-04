#include "WebServer.h"
#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <strings.h>
#include <stdio.h>
#include <fstream>
using namespace std ;

int myPort ;



void WebServer::Boot(int Port, string rootDir)
{
	/* Set the document root directory */
	if( chdir(rootDir.c_str()) == -1 )
	{
		cerr << "Cannot change root directory\n" ;
		exit(0) ;
	}	
			
	
	/* Bind a port and listen */
	myPort = Port ;
	int masterSocket = PassiveSock( Port ) ;	
	
	struct sockaddr_in clientAddr ;
	socklen_t addrLen = sizeof(clientAddr) ;
					
	while( true )
	{
		/* Accept an incomming client */
		int clientSocket ;
		if( (clientSocket = accept(masterSocket, (struct sockaddr*)&clientAddr, &addrLen)) < 0 )
		{
			cerr << "Fail to accept a new client\n" ;
			continue ;
		}
				
		/* Fork a child to handle the request */
		pid_t Pid ;
				
		if( (Pid = fork()) < 0 )
		{
			cerr << "Fail to fork a child\n" ;
			exit(0) ;
		}	
				
		else if( Pid == 0 )
		{
			RequestHandler( clientSocket ) ;
			
			if( masterSocket )
				close( masterSocket ) ;
			
			exit(0) ;
		}	
				
		else
			close( clientSocket ) ;
	}	
}


int WebServer::PassiveSock(int Port)
{
	struct protoent* getProto ;
	if( (getProto = getprotobyname("tcp")) == 0 )
	{
		cerr << "Cannot use TCP socket\n" ;
		exit(0) ;
	}	
	
	/* Open a TCP socket */
	int masterSocket ;
	if( (masterSocket = socket(PF_INET, SOCK_STREAM, getProto->p_proto)) < 0 )
	{
		cerr << "Fail to open a TCP socket\n" ;
		exit(0) ;
	}	
	
	/* Bind the local address */
	struct sockaddr_in serverAddr ;
	bzero((char*)&serverAddr, sizeof(serverAddr)) ;
	serverAddr.sin_family      = AF_INET ;
	serverAddr.sin_addr.s_addr = htonl( INADDR_ANY ) ;
	serverAddr.sin_port        = htons( Port ) ; 
	
	if( bind(masterSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
	{
		cerr << "Fail to bind the local address\n" ;
		exit(0) ;
	}	
	
	/* Listen on the socket */
	if( listen(masterSocket, QLEN) < 0 )
	{
		cerr << "Cannot listen on the socket\n" ;
		exit(0) ;
	}	
	
	return masterSocket ;
}


string WebServer::ContentType(string fileName) 
{
	if( fileName.find("html") != string::npos )
		return "text/html" ;
	
	else if( fileName.find("txt") != string::npos )
		return "text/plain" ;
	
	else if( fileName.find("png") != string::npos )
		return "image/png" ;
	
	else if( fileName.find("mp4") != string::npos )
		return "video/mp4" ;
	
	else if( fileName.find("ogg") != string::npos )
		return "audio/ogg" ;
	
	else
		return "" ;
}




void WebServer::StaticObject(string webPage)
{
	ifstream webPageFile( webPage ) ;
					
	if( webPageFile.is_open() )
	{				
		cout << "HTTP/1.1 200 OK\r\nContent-Type:" << ContentType( webPage ) << "\r\nresponse head\r\n\r\n" ;
						
		string line ;
		while( getline(webPageFile, line) )
			cout << line << endl ;
	
		webPageFile.close() ;
	}	
	
	else
	{
		cout << "HTTP/1.1 404 Not Found\r\nresponse head\r\n\r\n" ;
		cout << "<!doctype html>" << "<html><head>Inaccessible file</head><body></body></html>";
	}	
	
}


void WebServer::Directory(string webPage, DIR* thisDir)
{
	/* Check the existence of index.html */	
	
	string indexHTML = webPage + "/index.html" ;
	ifstream indexFile( indexHTML.c_str() ) ;
	
	if( indexFile.is_open() )
	{
		string line ;
		while( getline(indexFile, line) )
			cout << line << endl ;
		
		indexFile.close() ;
	}	
	
	/* Check if "no index.html" or "index.html is not readable" */
	else
	{	
		struct dirent* dir ;
		string htmlPage = "<!doctype html><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body>" ;
		
		while( (dir = readdir(thisDir)) != NULL )
		{
			/*  index.html exists, but it's not readable */
			if( dir->d_name == "index.html" )
			{
				cout << "HTTP/1.1 403 Forbidden\r\nresponse head\r\n\r\n" ; 	
				cout << "<!doctype html>" << "<html><head>Not readable page</head><body></body></html>";
				return ;
			}	
			
			htmlPage += "<p><a href=\"" ;
			htmlPage += "http://" + string(getenv("HOST")) + ":" + to_string(myPort) + "/" + webPage + "/" + string(dir->d_name) ;
			htmlPage += "\">" + string(dir->d_name) + "</a></p><br>" ;	
		}	
		htmlPage += "</body></html>" ;
		
		cout << htmlPage << endl ;
	}	
}

bool WebServer::isWebPageExit(string webPage)
{
	size_t Pos ;
	string webDir ;
	if( (Pos = webPage.find_first_of("/")) != string::npos )
	{	
		webDir = webPage.substr(0, Pos) ;
		webPage = webPage.substr(Pos+1, string::npos) ;
	}
	else
		webDir = "." ;	
	
	DIR* curDir ;
	struct dirent* dir ;
	bool isExist = false ;
		
	if( (curDir = opendir(webDir.c_str())) != 0 )
	{
		while( (dir = readdir(curDir)) != NULL )
		{
			if( dir->d_name == webPage )
			{
				isExist = true ;					
				break ;
			}	
		}	
			
		closedir( curDir ) ;		
	}		
	
	return isExist ;
}


bool WebServer::IsFile(string filePath)
{
	struct stat pathStat ;
	stat( filePath.c_str(), &pathStat ) ;
	
	return S_ISREG( pathStat.st_mode ) ;	
}


void WebServer::RequestHandler(int Socket)
{
	int tmpStdin  = dup(0) ;
	int tmpStdout = dup(1) ;
	dup2(Socket, 0) ;
	dup2(Socket, 1) ;
			
	char Buffer[BUFSIZE] ;
	int retLen ;
			
	/* Try to get the request mesg from client */
	if( (retLen = read(Socket, Buffer, BUFSIZE)) <= 0 )
	{
		dup2(tmpStdin, 0) ;
		dup2(tmpStdout, 1) ;
		close( tmpStdin ) ;
		close( tmpStdout ) ;
				
		return ;
	}	
	else
	{
		string Request(Buffer, retLen) ;		
		stringstream SS(Request) ;
		string webPage ;
		bool hasSlash = false ;
		
		getline(SS, webPage, '/') ;
		getline(SS, webPage, ' ') ;
		
		/* Extract query string if it's cgi */
		bool cgiOrNot = CGIhandler.IsCGI( webPage ) ;
		
		
		/* Remove the '?' character */
		webPage = webPage.substr(0, webPage.find_first_of("?")) ;
		
		if( webPage[webPage.length()-1] == '/' )
		{	
			hasSlash = true ;
			webPage.pop_back() ;
		}	
		
		
		if( isWebPageExit(webPage) )
		{		
			/* A cgi request */
			if( cgiOrNot )
			{	
				cout << "HTTP/1.1 200 OK\r\nContent-Type:" << ContentType( webPage ) << "\r\nresponse head\r\n\r\n" ;
				CGIhandler.CGIexectutor() ;	
			}
			/* A static object or directory */
			else 
			{						
				/* check whether it's a directory */
				if( !IsFile( webPage ) )
				{
					DIR* thisDir ;
					if( (thisDir = opendir(webPage.c_str())) != 0 ) 
					{	
						if( !hasSlash )
							cout << "HTTP/1.1 301 Moved Permanently\r\nContent-Type:" << ContentType( webPage ) << "\r\nresponse head\r\n\r\n" ;	
						else
							cout << "HTTP/1.1 200 OK\r\nContent-Type:" << ContentType( webPage ) << "\r\nresponse head\r\n\r\n" ;
						
						Directory( webPage, thisDir ) ;
					}
					
					/* Not readable directory */
					else
					{
						cout << "HTTP/1.1 404 Not Found\r\nresponse head\r\n\r\n" ;
						cout << "<!doctype html>" << "<html><head>Inaccessible page</head><body></body></html>"; 	
					}
				}
				
				
				/* static object */
				else
					StaticObject( webPage ) ;			
			}	
			 		
		}
		else
		{
			cout << "HTTP/1.1 403 Forbidden\r\nresponse head\r\n\r\n" ; 	
			cout << "<!doctype html>" << "<html><head>Not existed web page</head><body></body></html>";								
		}	
		
	}					
}
