#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <memory>

#include "printing.cpp"

//returns the cmd-prompt string, using the UI and PCL namespace
std::string get_cmd_prompt(const Session& ses, const DirElement& cdict){
    
    std::ostringstream oss;
    oss << PCL::BLUE << UI::PRE_PROMPT << PCL::CYAN << short_path(ses, cdict) << PCL::BLUE << UI::POST_PROMPT << PCL::END << UI::COMMAND_LINE_LINE;

    return oss.str();
}


//Tests if the given directory is hidden. Uses a Libary from 1985 for this.
bool is_hidden(const std::filesystem::directory_entry& entry) {

    //debug:
    //if(entry.path().filename().string() == ".git")std::cout << entry.path().filename().string() << attrs << ((attrs & FILE_ATTRIBUTE_HIDDEN ) != 0) << std::endl;

    #ifdef _WIN32
        DWORD attrs = GetFileAttributesW(entry.path().wstring().c_str());
        return (attrs & (FILE_ATTRIBUTE_HIDDEN)) != 0; //return (attrs == FAH)

    #else //unix
        //unix and macos hidden filenames and dirs start with '.'
        const auto filename = entry.path().filename().string();
        return !filename.empty() && filename[0] == '.';

    #endif
}


//sets the time attributes of the cdict via reference
bool cdict_set_filetime(DirElement& cdict, std::string pathname){

    #ifdef _WIN32

        FILETIME ft_creation_time;

        HANDLE entryhandle;

        if(cdict.type == UI::DIR_TYPE_NAME){
            entryhandle = CreateFileA(
                                pathname.c_str(),
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS,
                                NULL
                            );
        }
        else if(cdict.type == UI::FILE_TYPE_NAME){
            entryhandle = CreateFileA(
                                pathname.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                            );
        }
        else{
            if(OPTIONS::DEBUG) std::cout << "File " << pathname << " cannot be accessed because its type is N/A\n";
            return false;
        }
        
        

        if(entryhandle == INVALID_HANDLE_VALUE) {
            if(OPTIONS::DEBUG) std::cout << "CreateFile failed: " << pathname << std::endl;
            return false;
        }
        
        //return false if the GetFileTime process failed.
        if(!GetFileTime(entryhandle, &ft_creation_time, NULL, NULL)){
            if(OPTIONS::DEBUG) std::cout << warning_str("GetFileTime failed: ") << pathname << std::endl;
            CloseHandle(entryhandle);
            return false;
        }
        
        ULARGE_INTEGER ull;
        ull.LowPart  = ft_creation_time.dwLowDateTime;
        ull.HighPart = ft_creation_time.dwHighDateTime;

        // Windows to Unix epoch offset (in 100-ns ticks)
        static constexpr unsigned long long EPOCH_DIFF = 116444736000000000ULL;

        // Convert windows ticks to unix ns
        ULONGLONG unixTime100ns = ull.QuadPart - EPOCH_DIFF;

        auto timepoint = std::chrono::system_clock::time_point(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::nanoseconds(unixTime100ns * 100)
            )
        );

        cdict.creation_date = timepoint;
        CloseHandle(entryhandle);

    #elif __APPLE__

        struct stat st{};
        if (stat(pathname.c_str(), &st) != 0) {
            if(OPTIONS::DEBUG) std::cout << "stat failed: " << pathname << std::endl;
            return false;
        }

        // st_birthtimespec = struct timespec { tv_sec, tv_nsec }
        auto timepoint = std::chrono::system_clock::time_point{
            std::chrono::seconds(st.st_birthtimespec.tv_sec) +
            std::chrono::nanoseconds(st.st_birthtimespec.tv_nsec)
        };
        
        cdict.creation_date = timepoint;

    #else //linux

        

    #endif

    return true;
}

DirElement get_size(
    const fs::directory_entry& entry,
    DirElement* phomedir = nullptr,
    Progress_bar* pprgbar = nullptr,
    uint16_t depth = 0
) {

    DirElement currentdict;
    depth++;


    fs::file_status status = entry.symlink_status();
    if (fs::is_symlink(status)){
        currentdict.symlinks_skipped +=1;
        return currentdict;
    }

    if (fs::is_directory(status)){

        try {

            currentdict.key = entry.path().filename().string();

            currentdict.type = UI::DIR_TYPE_NAME;
            
            for (const fs::directory_entry& current_entry : fs::directory_iterator(entry.path(), fs::directory_options::skip_permission_denied)) {

                //start the next recursion
                DirElement nextdict = get_size(current_entry, phomedir, pprgbar, depth);

                //only load pprgbar in depth 1. Can be changed in the future to handle more accuracy, but needs to be extended to count these to "total"
                if(depth == 1 && pprgbar) pprgbar -> update_progressbar();

                //calculate the size (the important part here)
                currentdict.value += nextdict.value;

                //increment the dirs & files contained;
                if(nextdict.type == UI::DIR_TYPE_NAME){
                    currentdict.dirs_contained += nextdict.dirs_contained + 1;
                    currentdict.files_contained += nextdict.files_contained;
                }
                else currentdict.files_contained++;

                currentdict.subdir.push_back(std::move(nextdict));
            }
        }
        catch (const fs::filesystem_error& _) {
            
            //some dirs can not be scanned because they are symlinks.
            //these are catched here and increase .symlinks_skipped
            currentdict.symlinks_skipped += 1;
        }
    } 
    else if (fs::is_regular_file(status)){

        currentdict.key = entry.path().filename().string();
        currentdict.value +=  entry.file_size(); //final filesize that gets recursed up
        currentdict.type = UI::FILE_TYPE_NAME;
    }

    currentdict.is_invisible = is_hidden(entry);
    //is always done, doesnt care about entry type
    currentdict.path = entry.path().string();

    return currentdict;
}