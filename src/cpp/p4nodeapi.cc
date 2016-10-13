/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
 * LICENSE file.
 */

#include <uv.h>
#include <v8.h>
#include <node.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <string.h>
#include <string>

#undef SetPort

#include "p4nodeapi.h"

// using namespace node;
using namespace v8;

struct p4_baton_t {
    P4NodeApi *hw;
    int increment_by;
    int sleep_for;
    Persistent<Function> cb;
    Isolate *isolate;
    char** myargv;
    int    myargc;
    char *command;
    char *inputData;
};

Persistent<Function> P4NodeApi::constructor;

P4NodeApi::P4NodeApi(): m_count(0) {}

P4NodeApi::~P4NodeApi() {

}

void P4NodeApi::Init(Handle<Object> exports) {
    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "P4NodeApi"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "run", Run);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);

    constructor.Reset(isolate, tpl->GetFunction());

    exports->Set(String::NewFromUtf8(isolate, "P4NodeApi"), tpl->GetFunction());
}

void P4NodeApi::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    
    P4NodeApi* hw = new P4NodeApi();
    int usetags = 1;
    hw->Wrap(args.This());
    Local<Object> params;

    if (args.Length() > 0)
    {
        if (!args[0]->IsObject())
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Argument 0 must be an Object")));
        else
            params = Local<Object>::Cast(args[0]);

        //* Override the environment P4 settings if parameters are set
        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "port"))) {
            hw->client.SetPort(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "port"))->ToString()));
        }

        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "user"))) {
            hw->client.SetUser(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "user"))->ToString()));
        }

        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "password"))) {
            hw->client.SetPassword(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "password"))->ToString()));
        }

        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "client"))) {
            hw->client.SetClient(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "client"))->ToString()));
        }
        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "json"))) {
            char *json = *String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "json"))->ToString());
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

    args.GetReturnValue().Set(args.This());
}

void P4NodeApi::Run(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    P4NodeApi* hw = ObjectWrap::Unwrap<P4NodeApi>(args.This());

    Local<Array> cmd;
    Local<String> inputString = String::NewFromUtf8(isolate, "");
    Local<Function> cb;

    if (args.Length() <=1)
        isolate->ThrowException(Exception::SyntaxError(String::NewFromUtf8
                                (isolate, "Not enough arguments. [expected: (cmd, cb) || (cmd,inpt,cb)]")));

    if (!args[0]->IsArray())
        isolate->ThrowException(Exception::TypeError(
                                    String::NewFromUtf8(isolate, "Usage is (cmd, [options, input], fxn) - did not see a string cmd")));
    else
        cmd = Local<Array>::Cast(args[0]);


    if (args[1]->IsFunction())
    {
        cb = Local<Function>::Cast(args[1]);
    }
    else if (args[1]->IsObject())
    {
        Local<Object> params = Local<Object>::Cast(args[1]);
        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "user"))) {
            //std::cout << "Set user to: " << *String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "user"))->ToString()) << std::endl;
            hw->client.SetUser(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "user"))->ToString()));
        }

        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "password"))) {
            //std::cout << "Set password to: " << *String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "password"))->ToString()) << std::endl;
            hw->client.SetPassword(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "password"))->ToString()));
        }

        if (params->HasOwnProperty(String::NewFromUtf8(isolate, "client"))) {
            hw->client.SetClient(*String::Utf8Value(params->Get(String::NewFromUtf8(isolate, "client"))->ToString()));
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
                isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Usage is (cmd, opts, input, fxn)")));
            }
        }
        else if (args[2]->IsFunction())
        {
            cb = Local<Function>::Cast(args[2]);
        }
        else
        {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Usage is (cmd, opts, fxn)")));
        }
    }
    else if (args[1]->IsString())
    {
        inputString = Local<String>::Cast(args[1]);

        if (args[2]->IsFunction())
            cb = Local<Function>::Cast(args[2]);
        else
            isolate->ThrowException(Exception::TypeError(
                                String::NewFromUtf8(isolate, "Usage is (cmd, [options, input,] fxn)")));
    }
    else
        isolate->ThrowException(Exception::TypeError(
                                    String::NewFromUtf8(isolate, "Usage is (cmd, [input], fxn) did not see a callback method")));

    int parmCount = cmd->Length();
    char**  argv;
    int   argc;
    char *command;
    char *inputData;
    argv =  new char*[parmCount-1];
    argc = parmCount-1;

    //std::cout<< "Run() cmd IsArray? " <<cmd->IsArray() << " length=" << cmd->Length()<<std::endl;

    for (int i = 0 ; i < parmCount; i++) {
        String::Utf8Value p (cmd->Get(v8::Number::New(isolate, i)));
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
    String::Utf8Value cmdstr(cmd);
    //std::cout << "Run .. cmd string " <<  *cmdstr << std::endl;

    p4_baton_t *baton = new p4_baton_t();
    baton->isolate = isolate;
    baton->hw = hw;
    baton->increment_by = 2;
    baton->sleep_for = 1;
    baton->cb.Reset(isolate, cb);
    baton->myargv = argv;
    baton->myargc = argc;
    baton->command = command;


    String::Utf8Value s (inputString);
    inputData = (char*)malloc(strlen(*s)+1);
    strcpy(inputData,*s);
    baton->inputData = inputData;

    hw->Ref();

    uv_work_t *work_req = (uv_work_t *) (calloc(1, sizeof(uv_work_t)));
    work_req->data = baton;

    uv_queue_work(uv_default_loop(), work_req, EIO_P4, (uv_after_work_cb)EIO_AfterP4);
}

void P4NodeApi::Close(const FunctionCallbackInfo<Value>& args) {
    Error e;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    // TODO we probably need to place this on the work queue
    P4NodeApi* hw = ObjectWrap::Unwrap<P4NodeApi>(args.This());
    hw->client.Final(&e);
}

void P4NodeApi::EIO_P4(uv_work_t *req)
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

void P4NodeApi::EIO_AfterP4(uv_work_t *req)
{
    p4_baton_t *baton = static_cast<p4_baton_t *>(req->data);
//    baton->hw->Unref();

	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

    Local<Value> argv[2];

    // GetErr & GetInfo return the p4 command's err and/or data response(s) */
    argv[0]  = String::NewFromUtf8(isolate, baton->hw->ui.GetErr());
    argv[1]  = String::NewFromUtf8(isolate, baton->hw->ui.GetInfo());

    // String::Utf8Value info(argv[0]);
    // std::cout  << " EIO_AfterP4  " << *info ;

    TryCatch try_catch;

	Local<Function>::New(isolate, baton->cb)->Call(isolate->GetCurrentContext()->Global(), 2, argv);

    if (try_catch.HasCaught()) {
		isolate->ThrowException(try_catch.Exception());
    }

	baton->cb.Reset();

    delete baton;
    return;
}

// Persistent<FunctionTemplate> P4NodeApi::s_ct;

extern "C" {
    static void init (Handle<Object> exports)
    {
        P4NodeApi::Init(exports);
    }

    NODE_MODULE(p4nodeapi, init);
}
