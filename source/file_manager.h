#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

// C++
#include <string>
#ifdef __APPLE__
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

// libuv
#include <uv.h>

// threadpool
#include "synchronize.h"

// custom
#include "utilities.h"

#ifdef __APPLE__
typedef std::tr1::unordered_map<uint32_t, const FILE_INFO*> FileMap;
#else
typedef std::unordered_map<uint32_t, const FILE_INFO*> FileMap;
#endif

// success/fail of adding a task item to the queue
typedef enum LOAD_FILE_STATUS_ENUM
{
    // file successfully loaded
    LOAD_FILE_SUCCESS = 0,

    // file has already been loaded previously
    LOAD_FILE_EXISTS,

    // file was not able to load due to failure
    LOAD_FILE_FAIL

} LOAD_FILE_STATUS;

class FileManager
{
    public:

        // singleton instance of class
        static FileManager& GetInstance();

        // destructor
        virtual             ~FileManager();

        // load file and add to hash
        LOAD_FILE_STATUS    LoadFile(uint32_t fileKey, char *filePath);

        // remove file from hash
        void                RemoveFile(uint32_t fileKey);

        // get file string
        const FILE_INFO*    GetFileInfo(uint32_t fileKey);

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
