#include "CGI.h"
#include <sstream>
#include <iostream>
#include <unistd.h>
using namespace std ;

bool CGI::IsCGI(string webPage)
{
	if( 
		(webPage.find(".cgi") != string::npos) || 
		(webPage.find(".sh") != string::npos)  || 
		(webPage.find(".php") != string::npos) 
	  )
	{
		stringstream SS(webPage) ;
		getline(SS, exeFile, '?') ;
		getline(SS, queryString, ' ') ;
				
		return true ;
	}  
	  
		
	else
		return false ;			
}

void CGI::CGIexectutor()
{
	if( chdir(".") != 0 ) 
		cerr << "Cannot change directory\n" ;
	
	setenv( "PATH", ".", 1 ) ;	
	setenv("QUERY_STRING", queryString.c_str(), 0) ;
	if( execvp(exeFile.c_str(), NULL) < 0 )
	{
		cerr << "Fail to execute the cgi\n" ;
		exit(0) ;
	}	
}
