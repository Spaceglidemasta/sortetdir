/*
    This file includes the Main logic of the program - reading out file sizes,
    assembling objects, invoking the main cmd loop, etc.

    Functions for UI purposes, aswell as some structues, are located in printing.cpp.

    Github - @spaceglidemasta
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include "printing.cpp"



//TODO LINUX COMPATIBILITY!!!!!
#ifdef _WIN32
#include <windows.h>
#endif


namespace fs = std::filesystem;




//Command struct with .name and .args
typedef struct _Command {
    std::string name;
    std::vector<std::string> args;
} Command;

//Tests if the given :param:entry is hidden. Uses a Libary from 1985 for this.
bool is_hidden(const std::filesystem::directory_entry& entry) {
    const auto& path = entry.path();

    #ifdef _WIN32
        DWORD attrs = GetFileAttributesW(path.wstring().c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) return false;
        return (attrs & FILE_ATTRIBUTE_HIDDEN) != 0;
    //Linux / Unix case
    #else
        //invisible files start with '.' on unix
        std::string name = path.filename().string();
        return !name.empty() && name[0] == '.';
    #endif
}


Contentdict get_size(const fs::directory_entry& entry) {

    Contentdict currentdict;

    fs::file_status status = entry.symlink_status();
    if (fs::is_symlink(status)){
        return currentdict;
    }

    

    if (fs::is_directory(status)){

        try {

            currentdict.key = entry.path().filename().string();

            currentdict.type = UI::DIR_TYPE_NAME;
            
            for (const auto& current_entry : fs::directory_iterator(entry.path(), fs::directory_options::skip_permission_denied)) {

                Contentdict nextdict = get_size(current_entry);

                if(is_hidden(current_entry)) currentdict.is_invisible = true;

                currentdict.value += nextdict.value;

                currentdict.subdir.push_back(std::move(nextdict));    
            }

        }
        catch (const fs::filesystem_error& e) {
            
            currentdict.symlinks_skipped += 1;
        }
    } 
    else if (fs::is_regular_file(status)){

        currentdict.key = entry.path().filename().string();
        currentdict.value +=  entry.file_size();
        currentdict.type = UI::FILE_TYPE_NAME;
    }

    //is always done, doesnt care about entry type
    currentdict.path = entry.path().string();

    return currentdict;
}

Contentdict g_home_dir;

int main(int argc, char const *argv[]){


    



    /*
        Init the fs::directory_entry for get_size() and call.
        This is always done
    */
    fs::directory_entry cwd(fs::current_path());
    
    Contentdict cwd_dict = get_size(cwd);
    Contentdict* p_cwd_dict = &cwd_dict;

    g_home_dir = cwd_dict;




    //Print the final table
    print_cdict_table(cwd_dict);

    
    


    //json-object-a-like to store {command-name : command-func}
    std::unordered_map<std::string, std::function<void(const Command&, Contentdict*&)>> COMMANDS;

    //shows descriptions for every command
    COMMANDS["help"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout   << PCL::BOLD << "Commands:\n" << PCL::END 
                    << "cd      -> Change Directory to arg1.\n"
                    << "help    -> prints this?\n"
                    << "q       -> "<< bold_str("q") << "uits the programm.\n"
                    << "table   -> prints the standart sorted table\n"
                    << "tree    -> prints a file tree of the current dir\n"
                    << "what    -> uses the (in)famous \"what\" program on arg1\n"
                    << "cls     -> " << bold_str("cl") << "ears the " << bold_str("s") << "creen\n"
                    << "pwd     -> "<< bold_str("p") << "rints " << bold_str("w") << "orking " << bold_str("d") << "irectory"
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
            cdict = &g_home_dir; //cd logic
            print_cdict_table(*cdict);
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
            print_cdict_table(*cdict);
            return;
        }

        //if fullargs is (or is not) a subdir of cdict.
        //normal cd behavior
        for(auto& entry : cdict -> subdir){
            if(entry.key == fullargs){
                //cd logic
                entry.parent = cdict;
                cdict = &entry;
                print_cdict_table(*cdict);
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

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        print_cdict_tree(*cdict, TREE_DEFAULT_MAX_DEPTH, TREE_DEFAULT_DEPTH, true);
    };

    //prints a table view of the cdict. Try it. NOW!
    COMMANDS["table"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        print_cdict_table(*cdict);
    };

    //Clears the screen. Windows's "cls"
    COMMANDS["cls"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::system("cls"); // is this safe?
    };

    //Prints Working Directory. Linux's "pwd". I hate that on w64 "cd" pwd's
    COMMANDS["pwd"] = [](const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout << cdict -> path << std::endl;
    };
        
    std::string cmd_input; //cmd-line input
    //command line UI
    while (true){

        std::cout << UI::COMMAND_LINE_LINE;
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
            it -> second(fcmd, p_cwd_dict);
        }
        else {
            std::cout << warning_str("Unknown command: ") << fcmd.name << "\n";
        }
    }
    
    return 0;
}

