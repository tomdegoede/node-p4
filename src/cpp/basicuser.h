#include <string>

class BasicUser : public ClientUser
{
	public:
	             BasicUser();
	    virtual ~BasicUser();
        void OutputInfo( char level, const char *data );
        void OutputError( const char *err );
        void OutputStat( StrDict *dict );
        void InputData( StrBuf *strbuf, Error *e );
		char * GetInfo( );
        char * GetErr( );
        void SetInputData(char *iD);

    private:
        std::string eresult;
        std::string result;
		char * inputData;
};

