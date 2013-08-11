#include "file_manager.h"

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

// public instance "constructor"
FileManager& FileManager::GetInstance()
{
    // lazy instantiation of class instance
    static FileManager classInstance;

    // return by reference
    return classInstance;
}

// protected constructor
FileManager::FileManager()
{
    // create the file hash
    this->fileMap = new FileMap();

    // create file map mutex
    SyncCreateMutex(&(this->fileMapMutex), 0);
}

// destructor
FileManager::~FileManager()
{
    SyncLockMutex(&(this->fileMapMutex));
    delete this->fileMap;
    SyncUnlockMutex(&(this->fileMapMutex));

    SyncDestroyMutex(&(this->fileMapMutex));
}

void FileManager::LoadFile(uint32_t fileKey, char* filePath)
{
    fprintf(stdout, "FileManager::LoadFile - FileKey: %u FilePath: %s\n", fileKey, filePath);

    SyncLockMutex(&(this->fileMapMutex));

    // ensure this key does not already exist
    if(this->fileMap->find(fileKey) == this->fileMap->end())
    {
        // uv fs handle and fd
        uv_fs_t fs_req;
        uv_file fd;

        // open file
        uv_fs_open(uv_default_loop(), &fs_req, filePath, O_RDONLY, 0, NULL);
        fd = fs_req.result;

        // get file size
        if (fs_req.result != -1)
        {
            uv_fs_fstat(uv_default_loop(), &fs_req, fd, NULL);
        }
        else
        {
            fprintf(stderr, "FileManager::LoadFile - Error opening file: %d\n", fs_req.errorno);
        }

        // allocate buffer memory and read file
        uint64_t fileSize = 0;
        char *fileBuffer = 0;
        if (fs_req.result != -1)
        {
            // allocate buffer size to the file size + '\0'
            fileSize = fs_req.statbuf.st_size;
            fileBuffer = (char *)malloc(fileSize + 1);
            memset(fileBuffer, 0, fileSize + 1);

            // read file - synchronous
            uv_fs_read(uv_default_loop(), &fs_req, fd, fileBuffer, fileSize, -1, NULL);
        }

        // file-read successful
        if (fs_req.result >= 0)
        {
            uv_fs_close(uv_default_loop(), &fs_req, fd, NULL);
        }

        // cleanup uv file system handle
        uv_fs_req_cleanup(&fs_req);

        // store file string to file map
        this->fileMap->insert(make_pair(fileKey, fileBuffer));
        //fprintf(stdout, "FileManager::LoadFile - File loaded: %s\n", filePath);

        // de-allocate file buffer memory
        free(fileBuffer);
    }
    else
    {
        fprintf(stderr, "FileManager::LoadFile - File key already exists: %u\n", fileKey);
    }

    SyncUnlockMutex(&(this->fileMapMutex));
}

void FileManager::RemoveFile(uint32_t fileKey)
{
    fprintf(stdout, "FileManager::RemoveFile - FileKey: %u\n", fileKey);

    SyncLockMutex(&(this->fileMapMutex));

    // ensure this key does not already exist
    if(this->fileMap->find(fileKey) != this->fileMap->end())
    {
        this->fileMap->erase(fileKey);
    }

    SyncUnlockMutex(&(this->fileMapMutex));
}

const string* FileManager::GetFileString(uint32_t fileKey)
{
    //fprintf(stdout, "FileManager::GetFileString - FileKey: %u\n", fileKey);

    string *fileString = 0;

    SyncLockMutex(&(this->fileMapMutex));

    // ensure this key does not already exist
    if(this->fileMap->find(fileKey) != this->fileMap->end())
    {
        fileString = &((this->fileMap->find(fileKey))->second);
    }

    SyncUnlockMutex(&(this->fileMapMutex));

    return fileString;
}