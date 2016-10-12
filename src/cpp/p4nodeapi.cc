/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include <stdlib.h>

#include "clientapi.h"
#include "basicuser.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <string>

using namespace node;
using namespace v8;

class P4NodeApi: ObjectWrap
{
private:
    int m_count;
    BasicUser ui;
    ClientApi client;

public:

    static Persistent<FunctionTemplate> s_ct;
    static void Init(Handle<Object> target)
    {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("P4NodeApi"));

        NODE_SET_PROTOTYPE_METHOD(s_ct, "run", Run);
        NODE_SET_PROTOTYPE_METHOD(s_ct, "close", Close);

        target->Set(String::NewSymbol("P4NodeApi"),
                    s_ct->GetFunction());
    }

    P4NodeApi() :
        m_count(0)
    {
    }

    ~P4NodeApi()
    {
    }

    static Handle<Value> New(const Arguments& args)
    {
        HandleScope scope;
        P4NodeApi* hw = new P4NodeApi();
        int usetags = 1;
        hw->Wrap(args.This());
        Local<Object> params;

        if (args.Length() > 0)
        {
            if (!args[0]->IsObject())
                return ThrowException(Exception::TypeError(
                                          String::New("Argument 0 must be an Object")));
            else
                params = Local<Object>::Cast(args[0]);

            //* Override the environment P4 settings if parameters are set
            if (params->HasOwnProperty(String::New("port"))) {
                hw->client.SetPort(*String::Utf8Value(params->Get(String::New("port"))->ToString()));
            }

            if (params->HasOwnProperty(String::New("user"))) {
                hw->client.SetUser(*String::Utf8Value(params->Get(String::New("user"))->ToString()));
            }

            if (params->HasOwnProperty(String::New("password"))) {
                hw->client.SetPassword(*String::Utf8Value(params->Get(String::New("password"))->ToString()));
            }

            if (params->HasOwnProperty(String::New("client"))) {
                hw->client.SetClient(*String::Utf8Value(params->Get(String::New("client"))->ToString()));
            }
            if (params->HasOwnProperty(String::New("json"))) {
                char *json = *String::Utf8Value(params->Get(String::New("json"))->ToString());
                if (!strcmp("false",json) || !strcmp("False",json) || !strcmp("FALSE",json)) {
                    usetags = 0;
                }
            }
        }
        if (usetags) {
            hw->client.SetProtocol("tag", "");
        }

        Error e;

        // TODO We *might* need to put this on a work queue
        hw->client.Init( &e );

        if( e.Test() )
        {
            // e.Fmt( &msg );
            // fprintf( stderr, "client.Init %s\n", msg.Text() );
            hw->ui.OutputError("connection refused.");
            hw->client.Final( &e );
        }


        return args.This();
    }

    struct p4_baton_t {
        P4NodeApi *hw;
        int increment_by;
        int sleep_for;
        Persistent<Function> cb;
        char** myargv;
        int    myargc;
        char *command;
        char *inputData;
    };

    static Handle<Value> Run(const Arguments& args)
    {
        HandleScope scope;
        Local<Array> cmd;
        Local<String> inputString = String::New("",0);
        Local<Function> cb;

        P4NodeApi* hw = ObjectWrap::Unwrap<P4NodeApi>(args.This());

        if (args.Length() <=1)
            return ThrowException(Exception::SyntaxError(String::New
                                  ("Not enough arguments. [expected: (cmd, cb) || (cmd,inpt,cb)]")));

        if (!args[0]->IsArray())
            return ThrowException(Exception::TypeError(
                                      String::New("Usage is (cmd, [options, input], fxn) - did not see a string cmd")));
        else
            cmd = Local<Array>::Cast(args[0]);


        if (args[1]->IsFunction())
        {
            cb = Local<Function>::Cast(args[1]);
        }
        else if (args[1]->IsObject())
        {
            Local<Object> params = Local<Object>::Cast(args[1]);
            if (params->HasOwnProperty(String::New("user"))) {
                //std::cout << "Set user to: " << *String::Utf8Value(params->Get(String::New("user"))->ToString()) << std::endl;
                hw->client.SetUser(*String::Utf8Value(params->Get(String::New("user"))->ToString()));
            }

            if (params->HasOwnProperty(String::New("password"))) {
                //std::cout << "Set password to: " << *String::Utf8Value(params->Get(String::New("password"))->ToString()) << std::endl;
                hw->client.SetPassword(*String::Utf8Value(params->Get(String::New("password"))->ToString()));
            }

            if (params->HasOwnProperty(String::New("client"))) {
                hw->client.SetClient(*String::Utf8Value(params->Get(String::New("client"))->ToString()));
            }

            if (args[2]->IsString())
            {
                inputString = Local<String>::Cast(args[2]);
                if (args[3]->IsFunction())
                {
                    cb = Local<Function>::Cast(args[3]);
                }
                else
                {
                    return ThrowException(Exception::TypeError(String::New("Usage is (cmd, opts, input, fxn)")));
                }
            }
            else if (args[2]->IsFunction())
            {
                cb = Local<Function>::Cast(args[2]);
            }
            else
            {
                return ThrowException(Exception::TypeError(String::New("Usage is (cmd, opts, fxn)")));
            }
        }
        else if (args[1]->IsString())
        {
            inputString = Local<String>::Cast(args[1]);

            if (args[2]->IsFunction())
                cb = Local<Function>::Cast(args[2]);
            else
                ThrowException(Exception::TypeError(
                                   String::New("Usage is (cmd, [options, input,] fxn)")));
        }
        else
            return ThrowException(Exception::TypeError(
                                      String::New("Usage is (cmd, [input], fxn) did not see a callback method")));

        int parmCount = cmd->Length();
        char**  argv;
        int   argc;
        char *command;
        char *inputData;
        argv =  new char*[parmCount-1];
        argc = parmCount-1;

        //std::cout<< "Run() cmd IsArray? " <<cmd->IsArray() << " length=" << cmd->Length()<<std::endl;

        for (int i = 0 ; i < parmCount; i++) {
            String::AsciiValue p (cmd->Get(v8::Number::New(i)));
            //std::cout << i << " p: " << *p << " len: " << strlen(*p) << std::endl;
            char* ap;
            if ( i == 0) {
                command = (char*)malloc(strlen(*p) + 1);
                strcpy(command,*p);
            }
            else {
                ap = (char*)malloc(strlen(*p) + 1);
                strcpy(ap,*p);
                argv[i-1] = ap;
                //std::cout << i << " argv : " << argv[i-1] << " len: " << strlen(argv[i-1]) << std::endl;
            }

        }
        String::AsciiValue cmdstr(cmd);
        //std::cout << "Run .. cmd string " <<  *cmdstr << std::endl;

        p4_baton_t *baton = new p4_baton_t();
        baton->hw = hw;
        baton->increment_by = 2;
        baton->sleep_for = 1;
        baton->cb = Persistent<Function>::New(cb);
        baton->myargv = argv;
        baton->myargc = argc;
        baton->command = command;


        String::AsciiValue s (inputString);
        inputData = (char*)malloc(strlen(*s)+1);
        strcpy(inputData,*s);
        baton->inputData = inputData;

        hw->Ref();

        uv_work_t *work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
        work_req->data = baton;

        uv_queue_work(uv_default_loop(), work_req, EIO_P4, (uv_after_work_cb)EIO_AfterP4);

        return Undefined();
    }

    static Handle<Value> Close(const Arguments& args)
    {
        Error e;
        // TODO we probably need to place this on the work queue
        P4NodeApi* hw = ObjectWrap::Unwrap<P4NodeApi>(args.This());
        hw->client.Final(&e);

        return Undefined();
    }

    static void EIO_P4(uv_work_t *req)
    {
        Error e;
        StrBuf msg;
        char**  argv;
        int argc = 0;

        p4_baton_t *baton = static_cast<p4_baton_t *>(req->data);
        baton->hw->m_count += baton->increment_by;
        P4NodeApi *hw = baton->hw;                     // our class object

        argv =  new char*[argc];
        argv = baton->myargv;
        argc = baton->myargc;
        char *command = baton->command;
        //std::cout << " EIO_P4 command = " << command  <<std::endl;
        //for (int i = 0 ; i < argc; i++) {
        //    std::cout << " EIO_P4 argv : i  = " << argv[i]  <<std::endl;
        //}


        // fprintf( stderr, "client.Init %s\n", msg.Text() );
        // hw->client.Init( &e );

        // if( e.Test() )
        // {
        //     // e.Fmt( &msg );
        //     // fprintf( stderr, "client.Init %s\n", msg.Text() );
        //     hw->ui.OutputError("connection refused.");
        //     hw->client.Final( &e );
        // }
        // else {
            if (sizeof(baton->inputData)>0)
                (&hw->ui)->SetInputData(baton->inputData);

            hw->client.SetArgv( argc, argv );
            hw->client.Run( command, &hw->ui );

            // // Clean up connection
            // hw->client.Final( &e );
        // }

        // if( e.Test() )
        // {
        //     e.Fmt( &msg );
        //     fprintf( stderr, "client.Final %s\n", msg.Text() );
        // }
        return;

    }

    static void EIO_AfterP4(uv_work_t *req)
    {
        HandleScope scope;
        p4_baton_t *baton = static_cast<p4_baton_t *>(req->data);
        baton->hw->Unref();

        Local<Value> argv[2];

        // GetErr & GetInfo return the p4 command's err and/or data response(s) */
        argv[0]  = String::New(baton->hw->ui.GetErr());
        argv[1]  = String::New(baton->hw->ui.GetInfo());

        //String::AsciiValue info(argv[0]);
        //std::cout  << " EIO_AfterP4  " << *info ;

        TryCatch try_catch;

        baton->cb->Call(Context::GetCurrent()->Global(), 2, argv);

        if (try_catch.HasCaught()) {
            FatalException(try_catch);
        }

        baton->cb.Dispose();

        delete baton;
        return;
    }

};

Persistent<FunctionTemplate> P4NodeApi::s_ct;

extern "C" {
    static void init (Handle<Object> target)
    {
        P4NodeApi::Init(target);
    }

    NODE_MODULE(p4nodeapi, init);
}
