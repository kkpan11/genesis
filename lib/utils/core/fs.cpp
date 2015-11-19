/**
 * @brief Implementation of file system functions.
 *
 * @file
 * @ingroup utils
 */

#include "utils/core/fs.hpp"

#include <dirent.h>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <sys/stat.h>

#include "utils/core/logging.hpp"

namespace genesis {

// =================================================================================================
//     File Access
// =================================================================================================

bool file_exists (const std::string& fn)
{
    std::ifstream infile(fn);
    return infile.good();
}

std::string file_read (const std::string& fn)
{
    std::ifstream infile(fn);
    std::string   str;

    if (!infile.good()) {
        LOG_WARN << "Cannot read from file '" << fn << "'.";
        return "";
    }

    infile.seekg(0, std::ios::end);
    str.reserve(infile.tellg());
    infile.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(infile)),
                std::istreambuf_iterator<char>());
    return str;
}

bool file_write (const std::string& fn, const std::string& content)
{
    // TODO check if path exists, create if not (make a function for that)
    // TODO check if file exists, trigger warning?, check if writable

    std::ofstream outfile(fn);
    if (!outfile.good()) {
        LOG_WARN << "Cannot write to file '" << fn << "'.";
        return false;
    }
    outfile << content;
    return true;
}

bool file_append (const std::string& fn, const std::string& content)
{
    // TODO check if path exists, create if not (make a function for that)
    // TODO check if file exists, trigger warning?, check if writable
    // TODO maybe merge with file_write and use mode as optional parameter.

    std::ofstream outfile(fn, std::ofstream::app);
    if (!outfile.good()) {
        LOG_WARN << "Cannot write to file '" << fn << "'.";
        return false;
    }
    outfile << content;
    return true;
}

bool dir_exists (const std::string& dir)
{
    struct stat info;
    if (stat (dir.c_str(), &info) != 0) {
        return false;
    }
    return info.st_mode & S_IFDIR;

    // DIR* dp = opendir(dir);
    // if (dp) {
    //     closedir(dir);
    //     return dp;
    // } else {
    //     return false;
    // }
}

bool dir_list_files (const std::string& dir, std::vector<std::string>& list)
{
    DIR*           dp;
    struct dirent* dirp;

    if((dp  = opendir(dir.c_str())) == nullptr) {
        LOG_WARN << "Cannot open directory '" << dir << "' (Error " << errno << ").";
        return false;
    }
    while ((dirp = readdir(dp)) != nullptr) {
        std::string fn = std::string(dirp->d_name);
        if (fn == "." || fn == "..") {
            continue;
        }
        list.push_back(fn);
    }
    closedir(dp);
    //~ std::sort(list.begin(), list.end());
    return true;
}

// =================================================================================================
//     File Information
// =================================================================================================

std::unordered_map<std::string, std::string> file_info (std::string filename)
{
    std::string basename = file_basename(filename);
    std::unordered_map<std::string, std::string> res;

    res["path"]      = file_path(filename);
    res["basename"]  = basename;
    res["filename"]  = file_filename(basename);
    res["extension"] = file_extension(basename);

    return res;
}

size_t file_size (std::string filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return static_cast<size_t>(in.tellg());
}

std::string file_path (std::string filename)
{
    const size_t idx = filename.find_last_of("\\/");
    if (idx != std::string::npos)
    {
        filename.erase(idx);
    }
    return filename;
}

std::string file_basename (std::string filename)
{
    const size_t idx = filename.find_last_of("\\/");
    if (idx != std::string::npos)
    {
        filename.erase(0, idx + 1);
    }
    return filename;
}

std::string file_filename (std::string filename)
{
    const size_t idx = filename.rfind('.');
    if (idx != 0 && idx != std::string::npos)
    {
        filename.erase(idx);
    }
    return filename;
}

std::string file_extension (std::string filename)
{
    const size_t idx = filename.rfind('.');
    if (idx != 0 && idx != std::string::npos)
    {
        filename.erase(0, idx + 1);
    }
    return filename;
}

} // namespace genesis