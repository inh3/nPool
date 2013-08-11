#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

// C++
#include <string>
#ifdef __APPLE__
#include <tr1/unordered_map>
using namespace std::tr1;
#else
#include <unordered_map>
#endif
using namespace std;

// libuv
#include <uv.h>

// threadpool
#include "synchronize.h"

typedef unordered_map<uint32_t, string> FileMap;

class FileManager
{
    public:
        
        // singleton instance of class
        static FileManager& GetInstance();

        // destructor
        virtual             ~FileManager();

        // load file and add to hash
        void                LoadFile(uint32_t fileKey, char *filePath);

        // remove file from hash
        void                RemoveFile(uint32_t fileKey);

        // get file string
        const string*       GetFileString(uint32_t fileKey);

    protected:

        // ensure default constructor can't get called
        FileManager();

        // declare private copy constructor methods to ensure they can't be called
        FileManager(FileManager const&);
        void operator=(FileManager const&);

    private:

        FileMap             *fileMap;
        THREAD_MUTEX        fileMapMutex;
};

#endif /* _FILE_MANAGER_H_ */