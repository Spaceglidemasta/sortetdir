/*
    This file includes the Main logic of the program - reading out file sizes,
    assembling objects, invoking the main cmd loop, etc.

    Functions for UI purposes, aswell as some structues, are located in printing.cpp.

    Github - @spaceglidemasta
*/

//TODO structure code & FIX INSTALLATION PROGRESS


#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <memory>
#include "printing.cpp"


namespace fs = std::filesystem;

//Command struct with .name and .args
struct Command {
    std::string name;
    std::vector<std::string> args;
};

//returns the cmd-prompt string, using the UI and PCL namespace
std::string get_cmd_prompt(Contentdict cdict){
    
    std::ostringstream oss;
    oss << PCL::BLUE << UI::PRE_PROMPT << PCL::CYAN << short_path(cdict) << PCL::BLUE << UI::POST_PROMPT << PCL::END << UI::COMMAND_LINE_LINE;

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
bool cdict_set_filetime(Contentdict& cdict, const fs::directory_entry& entry){

    #ifdef _WIN32

        std::string entry_name = entry.path().string();

        FILETIME ft_creation_time;

        HANDLE entryhandle;

        if(cdict.type == UI::DIR_TYPE_NAME){
            entryhandle = CreateFileA(
                                entry_name.c_str(),
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
                                entry_name.c_str(),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                            );
        }
        else{
            if(OPTIONS::DEBUG) std::cout << "File " << entry_name << " cannot be accessed because its type is N/A\n";
            return false;
        }
        
        

        if(entryhandle == INVALID_HANDLE_VALUE) {
            if(OPTIONS::DEBUG) std::cout << "CreateFile failed: " << entry_name << std::endl;
            return false;
        }
        
        //return false if the GetFileTime process failed.
        if(!GetFileTime(entryhandle, &ft_creation_time, NULL, NULL)){
            if(OPTIONS::DEBUG) std::cout << warning_str("GetFileTime failed: ") << entry_name << std::endl;
            CloseHandle(entryhandle);
            return false;
        }
        
        ULARGE_INTEGER ull;
        ull.LowPart  = ft_creation_time.dwLowDateTime;
        ull.HighPart = ft_creation_time.dwHighDateTime;

        // Windows to Unix epoch offset (in 100-ns ticks)
        static constexpr ULONGLONG EPOCH_DIFF = 116444736000000000ULL;

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
        if (stat(entry_name.c_str(), &st) != 0) {
            if(OPTIONS::DEBUG) std::cout << "stat failed: " << entry_name << std::endl;
            return false;
        }

        // st_birthtimespec = struct timespec { tv_sec, tv_nsec }
        auto timepoint = std::chrono::system_clock::time_point{
            std::chrono::seconds(st.st_birthtimespec.tv_sec) +
            std::chrono::nanoseconds(st.st_birthtimespec.tv_nsec)
        };
        
        cdict.creation_date = timepoint;

    #else //linux

        //im not gonna mess with all these FS, yall really need to implement this globally.

    #endif

    return true;
}




Contentdict get_size(const fs::directory_entry& entry, Contentdict* phomedir = nullptr, Progress_bar* pprgbar = nullptr, uint16_t depth = 0) {

    Contentdict currentdict;
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
            
            for (const auto& current_entry : fs::directory_iterator(entry.path(), fs::directory_options::skip_permission_denied)) {

                //start the next recursion
                Contentdict nextdict = get_size(current_entry, phomedir, pprgbar, depth);

                //only load pprgbar in depth 1. Can be changed in the future to handle more accuracy, but needs to be extended to count these to "total"
                if(depth == 1 && pprgbar) pprgbar -> update_progressbar();

                //home directory for :cmd:"cd"
                nextdict.home_dir = phomedir;

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

    currentdict.entry = entry;

    return currentdict;
}


int main(int argc, char const *argv[]){

    load_json();
    /*
        Init the fs::directory_entry for get_size() and call.
        This is always done
    */
    fs::directory_entry cwd_entry(fs::current_path());
    
    Contentdict cdict;
    Contentdict* pcdict = &cdict;

    Progress_bar prgbar(cwd_entry);
    cdict  = get_size(cwd_entry, pcdict, &prgbar); //* ------> Core of the program

    std::cout << std::endl;

    //Print the final table
    print_cdict_table(cdict);

    //json-object-a-like to store {command-name : command-func}
    std::unordered_map<std::string, std::function<void(const Command&, Contentdict*&)>> COMMANDS;

    //shows descriptions for every command
    COMMANDS["help"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout   << PCL::BOLD << "Commands:\n" << PCL::END 
                    << "cd      -> Change Directory to target.\n"
                    << "           default: cd's into the home directory.\n"
                    << "           arg1: specifies the target.\n"
                    << "           \"cd ..\": cd's one directory up.\n"
                    << "help    -> prints this?\n"
                    << "q       -> "<< bold_str("q") << "uits the programm.\n"
                    << "table   -> prints the standart sorted table\n"
                    << "tree    -> prints a file tree of the current dir\n"
                    //<< "what    -> uses the (in)famous \"what\" program on arg1\n"
                    << "cls     -> " << bold_str("cl") << "ears the " << bold_str("s") << "creen\n"
                    << "pwd     -> "<< bold_str("p") << "rints " << bold_str("w") << "orking " << bold_str("d") << "irectory\n"
                    << "info    -> Gives information about the creation of the file / dir, and how\n"
                    << "           many files are in the dir.\n"
                    << "           arg1: specifies the target, default is the current path.\n"
                    << std::endl;
    };
    
    //quits the program
    COMMANDS["q"] = [](const Command& cmd, Contentdict*& cdict){
        std::exit(0);
    };

    /*
        Uses pointer logic to change up in down in the cdict directory hirachy.
        Behaves like "cd" ON LINUX.

        Used to be handled by having the whole contendict system beeing pointer based,
        which turned out to be to complicated for its simple use.
    */
    COMMANDS["cd"] = [](const Command& cmd, Contentdict*& cdict){

        //goes to the home directory, just like in Linux. Because pwd'ing on "cd" is stupid.
        if (cmd.args.empty()) {
            cdict = cdict->home_dir; //cd logic
            return;
        }

        //Combines all args into one so paths with spaces are fine.
        //Python's " ".join(cmd.args)
        std::string fullargs = merge_str(cmd.args);

        //goes back 1 directory
        if(fullargs == ".."){

            if(cdict -> parent == nullptr){
                std::cout   << warning_str("Highest Parent directory reached.\n")
                            << "Start program in a higher directory if you wish to calculate said dir.\n";
                return;
            }

            //cd logic
            cdict = cdict -> parent;
            return;
        }

        //if fullargs is (or is not) a subdir of cdict.
        //normal cd behavior
        for(auto& entry : cdict -> subdir){
            if(entry.key == fullargs){
                //cd logic
                entry.parent = cdict;
                cdict = &entry;
                return;
            }
        }

        //else (given directory was not found)
        std::cout << "Subdir \"" << fullargs << "\" was not found.\n"; 

    };

    //the "what" program is not yet released  / shit af, so you can safely ignore this and the "what" command
    COMMANDS["what"] = [](const Command& cmd, Contentdict*& cdict){

        if(!WHAT_ENABLED){
            std::cout   << warning_str("Enable \"WHAT_ENABLED\" and install the \"what\" program from repo.\n")
                        << "@spaceglidemasta on github - UNRELEASED, IGNORE" << std::endl;
        }

        if (cmd.args.empty()) {
            std::cout << warning_str("what: missing argument") << std::endl;
            return;
        }

        std::string fcmd = "what " + cmd.args[0];

        int ret = std::system(fcmd.c_str());
        if (ret != 0){
            std::cerr << warning_str("what: unknown error accured while executing the command.") << std::endl;
        }
    };

    //prints a tree view of the cdict. Try it.
    COMMANDS["tree"] = [](const Command& cmd, Contentdict*& cdict){

        if((cmd.args.empty())){
            print_cdict_tree(*cdict, TREE_DEFAULT_MAX_DEPTH, TREE_DEFAULT_DEPTH, true);
            return;
        }
        else if(cmd.args.size() > 1){
            std::cout << info_str("This command only takes 0 or 1 arg, the rest were ignored.") << std::endl;
        }

        try{
            print_cdict_tree(*cdict, std::stoi(cmd.args[0]), TREE_DEFAULT_DEPTH, true);
        }
        catch(std::runtime_error){
            std::cout << warning_str("tree: Runtime Error, misused arg\n");
        }
        
    };
    

    //prints a table view of the cdict.
    COMMANDS["table"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        print_cdict_table(*cdict);
    };

    //Clears the screen. Windows's "cls"
    COMMANDS["cls"] = [](const Command& cmd, Contentdict*& cdict){

        #ifdef _WIN32
            std::system("cls");
        #else // linux & apple
            std::system("clear");
        #endif

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }
        
    };

    //Prints Working Directory. Linux's "pwd". I hate that on w64 "cd" pwd's
    COMMANDS["pwd"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout << cdict -> path << std::endl;
    };
        
    std::string cmd_input; //cmd-line input

    COMMANDS["info"] = [](const Command& cmd, Contentdict*& cdict){

        if(cmd.args.empty()){

            cdict_set_filetime(*cdict, cdict -> entry);

            std::cout << "\nSize of current directiory: " << size_ext(cdict -> value) << std::endl;

            print_ctime(*cdict);

            std::cout << "Contains: " << cdict -> files_contained << " Files, " << cdict -> dirs_contained << " Folders" << std::endl;
            
            return;
        }
        //else: args are given

        
        //Combines all args into one so paths with spaces are fine.
        //Python's " ".join(cmd.args)
        std::string fullargs = merge_str(cmd.args);

        for(auto& entry : cdict -> subdir){
            if(entry.key == fullargs){
                std::cout << "\nSize of "<< entry.key << ": " << size_ext(entry.value) << std::endl;

                cdict_set_filetime(entry, entry.entry);
                print_ctime(entry);

                if(entry.type == UI::DIR_TYPE_NAME)
                    std::cout << "Contains: " << entry.files_contained << " Files, " << entry.dirs_contained << " Folders" << std::endl;

                return;
            }
        }

        //else (given directory was not found)
        std::cout << "Subdir \"" << fullargs << "\" was not found.\n";

        
        
    };


    //command line UI
    while (true){

        std::cout << get_cmd_prompt(*pcdict);
        if(!std::getline(std::cin, cmd_input)){
            throw std::invalid_argument("Input could not be gathered.");
        }
        
        //"Command arg1 arg2" -> ["Command","arg1","arg2"]
        std::vector<std::string> cmd_input_split = split_by_space(cmd_input);

        //ignore empty new-lines
        if (cmd_input_split.empty()) {
            continue;
        }

        //vector - vector[0]
        std::vector<std::string> rest(cmd_input_split.begin() + 1, cmd_input_split.end());
        Command fcmd;

        //fcmd.name = "Command", fcmd.args = ["arg1", ...]
        fcmd.name = cmd_input_split[0];
        fcmd.args = rest;

        //idk how the f this object is called. Auto does tho.
        auto it = COMMANDS.find(fcmd.name);
        if (it != COMMANDS.end()) {
            it -> second(fcmd, pcdict);
        }
        else {
            std::cout << warning_str("Unknown command: ") << fcmd.name << "\n";
        }
    }
    
    return 0;
}

