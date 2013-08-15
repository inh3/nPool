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

	// Compile the source code.
	Handle<Script> script = Script::Compile(source);

	// Run the script to get the result.
	Handle<Value> scriptResult = script->Run();

	// free file buffer and close file ref
	fclose(requireFile);
	free(fileBuffer);

	return scope.Close(scriptResult);
}