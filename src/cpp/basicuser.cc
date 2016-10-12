# include <iostream>
# include <fstream>
# include <vector>
# include <sstream>
# include <errno.h>
# include <map>
# include <set>
# include <algorithm>
# include <string>

# include "clientapi.h"
# include "options.h"
# include "strops.h"
# include "basicuser.h"


using namespace std;


std::string escape(std::string const &s)
{
    std::size_t n = s.length();
    std::string escaped;
    escaped.reserve(n * 2);        // pessimistic preallocation

    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] == '\\' || s[i] == '\"' || s[i] == '/') 
        {
            escaped += '\\';
            escaped += s[i];
        }
        else if (s[i] == '\n')
        {
            escaped += "\\n";
        }
        else if (s[i] == '\t')
        {
            escaped += "\\t";
        }
        else if (s[i] == '\r')
        {
            escaped += "\\r";
        }
        else if (s[i] == '\b')
        {
            escaped += "\\b";
        }
        else if (s[i] == '\f')
        {
            escaped += "\\f";
        }
        else
        {
            escaped += s[i];
        }
    }
    return escaped;
}

void printDict( StrDict *dict )
{
	StrRef var, val;
	for( int i = 0; dict->GetVar( i, var, val ); i++ )
	    printf( "%s:  %s\n", var.Text(), val.Text() );
}


BasicUser::BasicUser()
: ClientUser()
{}

BasicUser::~BasicUser()
{}

void BasicUser::InputData( StrBuf *strbuf, Error *e )
{
    if (sizeof(inputData)==0)
    {
        e->Set( E_FAILED, "No user-input supplied." );
        eresult.append("Error: No input data. Did you forget to call .setInputData()? \n");
    }
    
    strbuf->Set(inputData);
}

void BasicUser::OutputInfo( char  level , const char *data )
{
	result.append(data);
	result.append("\n");
}

void BasicUser::OutputError( const char *err )
{
    eresult.append(err);
    eresult.append("\n");
}

void BasicUser::OutputStat( StrDict *dict )
{
	StrRef var, val;
    string stat, statcopy, s;

    // remove the [...]   array markers
    if (!result.empty() ) {
        statcopy.append(result,1,result.length()-2);
    }

	stat.append("{");
	for( int i = 0; dict->GetVar( i, var, val ); i++ ) {
        if (i > 0) stat.append(",");
        stat.append("\"");
        s = var.Text();
        stat.append(escape(s));
        stat.append("\":");
        stat.append("\"");
        s = val.Text();
        stat.append(escape(s));
        stat.append("\"");
    }

	stat.append("}");
    result = "[";
    if (!statcopy.empty()) {
        result.append(statcopy);
        result.append(",");
    }

    result.append(stat);
    result.append("]");
}


char * BasicUser::GetInfo( )
{
    char* cstr;
    cstr = new char [result.size()+1];
    strcpy (cstr, result.c_str());
    result.erase(0);
    return cstr;
}
char * BasicUser::GetErr( )
{
    char* cstr;
    cstr = new char [eresult.size()+1];
    strcpy (cstr, eresult.c_str());
    eresult.erase(0);
    return cstr;
}

void BasicUser::SetInputData(char * inptDt)
{
    inputData = inptDt;
}
