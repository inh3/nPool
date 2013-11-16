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

LOAD_FILE_STATUS FileManager::LoadFile(uint32_t fileKey, char* filePath)
{
    //fprintf(stdout, "[ FileManager ] - FileKey: %u FilePath: %s\n", fileKey, filePath);

    SyncLockMutex(&(this->fileMapMutex));

    // default to success
    LOAD_FILE_STATUS fileStatus = LOAD_FILE_SUCCESS;

    // ensure this key does not already exist
    if(this->fileMap->find(fileKey) == this->fileMap->end())
    {
        // get file path info
        FILE_INFO* fileInfo = Utilities::GetFileInfo(filePath);

        // file was invalid
        if(fileInfo->fullPath == 0)
        {
            fprintf(stderr, "[ FileManager ] - Error opening file: %s\n", fileInfo->fullPath);
            fileStatus = LOAD_FILE_FAIL;
        }
        else
        {
            // store file string to file map
            this->fileMap->insert(FileMap::value_type(fileKey, fileInfo));
            //fprintf(stdout, "[ FileManager ] - File loaded: %s (%u)\n", fileInfo->fullPath, fileKey);
        }
    }
    else
    {
        fprintf(stderr, "[ FileManager ] - File key already exists: %u\n", fileKey);
        fileStatus = LOAD_FILE_EXISTS;
    }

    SyncUnlockMutex(&(this->fileMapMutex));

    // return load status
    return fileStatus;
}

void FileManager::RemoveFile(uint32_t fileKey)
{
    fprintf(stdout, "FileManager::RemoveFile - FileKey: %u\n", fileKey);

    SyncLockMutex(&(this->fileMapMutex));

    // ensure this key does not already exist
    if(this->fileMap->find(fileKey) != this->fileMap->end())
    {
        const FILE_INFO* fileInfo = this->fileMap->find(fileKey)->second;
        Utilities::FreeFileInfo(fileInfo);
        this->fileMap->erase(fileKey);
    }

    SyncUnlockMutex(&(this->fileMapMutex));
}

const FILE_INFO* FileManager::GetFileInfo(uint32_t fileKey)
{
    //fprintf(stdout, "FileManager::GetFileString - FileKey: %u\n", fileKey);

    const FILE_INFO* fileInfo = 0;

    SyncLockMutex(&(this->fileMapMutex));

    // ensure this key does exist
    if(this->fileMap->find(fileKey) != this->fileMap->end())
    {
        fileInfo = this->fileMap->find(fileKey)->second;
    }

    SyncUnlockMutex(&(this->fileMapMutex));

    return fileInfo;
}