#include <FpConfig.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Types/BasicTypes.hpp>
#include <Os/File.hpp>
#include <Os/FileSystem.hpp>

namespace Os {

namespace FileSystem {

Status createDirectory(const char* path) { return OTHER_ERROR; }

Status removeDirectory(const char* path) { return OTHER_ERROR; }

Status readDirectory(const char* path, const U32 maxNum,
                     Fw::EightyCharString fileArray[]) {
    return OTHER_ERROR;
}

Status removeFile(const char* path) { return OTHER_ERROR; }

Status moveFile(const char* originPath, const char* destPath) {
    return OTHER_ERROR;
}

Status handleFileError(File::Status fileStatus) { return OTHER_ERROR; }

Status copyFile(const char* originPath, const char* destPath) {
    return OTHER_ERROR;
}

Status getFileSize(const char* path, U64& size) { return OTHER_ERROR; }

Status changeWorkingDirectory(const char* path) { return OTHER_ERROR; }

Status getFileCount(const char* directory, U32& fileCount) {
    return OTHER_ERROR;
}

}  // namespace FileSystem

}  // namespace Os