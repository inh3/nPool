#include "nrequire.h"

// C
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

Handle<Value> Require::RequireFunction(const Arguments& args)
{
	HandleScope scope;

	// validate input
    if((args.Length() != 1) || !args[0]->IsString())
    {
        ThrowException(Exception::TypeError(String::New("nRequire() - Expects 1 arguments: 1) file name (string)")));
        return scope.Close(Undefined());
    }

	// get filename string
	Local<String> v8FileName = (args[0])->ToString();
	String::AsciiValue fileName(v8FileName);
	//printf("[%u] Extra File Name: %s\n", SyncGetThreadId(), *fileName);

	// get file size
	struct stat fileInfo;
	int objectTypeRef = open(*fileName, O_RDONLY);
	fstat(objectTypeRef, &fileInfo);
	close(objectTypeRef);
	//printf("Extra File Size: %ld\n", fileInfo.st_size);

	// open file for reading
	FILE *requireFile = fopen(*fileName, "r");

	// allocate file buffer
	char *fileBuffer = (char *)malloc(fileInfo.st_size + 1);
	memset(fileBuffer, 0, fileInfo.st_size + 1);

	// get file contents
	fread(fileBuffer, 1, fileInfo.st_size, requireFile);

	// Create a string containing the JavaScript source code.
	Handle<String> source = String::New(fileBuffer);

	// compile the source code.
	Handle<Script> script = Script::Compile(source);

	// execute the script
	//printf("[%u] *** START - FILE BEING RUN!: %s\n", SyncGetThreadId(), *fileName);
	script->Run();
	//printf("[%u] *** END - FILE WAS RUN!: %s\n", SyncGetThreadId(), *fileName);

	// free file buffer and close file ref
	fclose(requireFile);
	free(fileBuffer);

	// get reference to global context
  	Handle<Object> globalContext = Context::GetCurrent()->Global();
  	Handle<Object> module = Handle<Object>::Cast(globalContext->Get(String::New("module")));

	// return the exports similar to node.js
	return scope.Close(module->Get(String::New("exports")));
}