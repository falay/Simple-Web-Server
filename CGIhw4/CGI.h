#ifndef CGI_H
#define CGI_H

#include <string>
using std::string ;

class CGI
{
	public:
		
		bool IsCGI(string webPage) ;
		
		void CGIexectutor() ;
	
	private:
		string queryString ;
		string exeFile ;
} ;



#endif //* CGI_H *//