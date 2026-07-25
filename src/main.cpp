/*
    This file includes the Main logic of the program - reading out file sizes,
    assembling objects, invoking the main cmd loop, etc.

    Functions for UI purposes, aswell as some structues, are located in printing.cpp.

    Github - @spaceglidemasta
*/

//TODO FIX INSTALLATION PROGRESS
//TODO update README about cla


#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include "fetching.cpp"




//Command struct with .name and .args
struct Command {
    std::string name;
    std::vector<std::string> args;
};

enum Execution_t {
    TABLE,
    TREE,
    CMDLINE,
    INVALID,
    DEFAULT,
    HELP,
    VERSION_DISPLAY
};

struct Runmode {

    Execution_t extype;
    std::string destination;
    int numarg;
    bool needs_dir_size_calc;

};


typedef std::unordered_map<std::string, std::function<void(const Session&, const Command&, Contentdict*&)>> CommandList;

CommandList registerCommands(){

    CommandList commands;
    //shows descriptions for every command
    commands["help"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

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
                    << "cls     -> " << bold_str("cl") << "ears the " << bold_str("s") << "creen\n"
                    << "pwd     -> "<< bold_str("p") << "rints " << bold_str("w") << "orking " << bold_str("d") << "irectory\n"
                    << "info    -> Gives information about the creation of the file / dir, and how\n"
                    << "           many files are in the dir.\n"
                    << "           arg1: specifies the target, default is the current path.\n"
                    << std::endl;
    };
    
    //quits the program
    commands["q"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){
        std::exit(0);
    };

    /*
        Uses pointer logic to change up in down in the cdict directory hirachy.
        Behaves like "cd" ON LINUX.

        Used to be handled by having the whole contendict system beeing pointer based,
        which turned out to be to complicated for its simple use.
    */
    commands["cd"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

        //goes to the home directory, just like in Linux.
        if (cmd.args.empty()) {
            cdict = ses.homedir; //cd logic
            return;
        }

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
        for(Contentdict& entry : cdict -> subdir){
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
    commands["what"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

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
    commands["tree"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

        if((cmd.args.empty())){
            print_cdict_tree(*cdict, TREE_DEFAULT_MAX_DEPTH);
            return;
        }
        else if(cmd.args.size() > 1){
            std::cout << info_str("This command only takes 0 or 1 arg, the rest were ignored.") << std::endl;
        }
        else /* 2 args */ {
    
            char* endptr;

            int depth = strtol((const char*) cmd.args[0].c_str(), &endptr, OPTIONS::INPUT_BASE);

            if(*endptr == '\0') {
                print_cdict_tree(*cdict, depth);            
            }
            else {
                std::cerr << warning_str("Invalid tree depth given: ") << cmd.args[0] << std::endl;
            }


            
        }
        
    };
    

    //prints a table view of the cdict.
    commands["table"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        print_cdict_table(*cdict);
    };

    //Clears the screen. Windows's "cls"
    commands["cls"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

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
    commands["pwd"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout << cdict -> path << std::endl;
    };
        
    

    commands["info"] = [](const Session& ses, const Command& cmd, Contentdict*& cdict){

        if(cmd.args.empty()){

            cdict_set_filetime(*cdict, cdict -> path);

            std::cout << "\nSize of current directiory: " << size_ext(cdict -> value) << std::endl;

            print_ctime(*cdict);

            std::cout << "Contains: " << cdict -> files_contained << " Files, " << cdict -> dirs_contained << " Folders" << std::endl;
            
            return;
        }
        //else: args are given

        
        std::string fullargs = merge_str(cmd.args);

        for(Contentdict& entry : cdict -> subdir){
            if(entry.key == fullargs){
                std::cout << "\nSize of "<< entry.key << ": " << size_ext(entry.value) << std::endl;

                cdict_set_filetime(entry, entry.path);
                print_ctime(entry);

                if(entry.type == UI::DIR_TYPE_NAME)
                    std::cout << "Contains: " << entry.files_contained << " Files, " << entry.dirs_contained << " Folders" << std::endl;

                return;
            }
        }

        //else (given directory was not found)
        std::cout << "Subdir \"" << fullargs << "\" was not found.\n";

        
        
    };

    return commands;

}



Runmode parseArgs(int argc, const char* argv[]) {

    Runmode rm;
    rm.extype = DEFAULT;
    rm.destination = "";
    bool take_num_in = false;
    rm.numarg = 0;
    rm.needs_dir_size_calc = false;

    if(argc == 1){
        rm.extype = TABLE;
        rm.needs_dir_size_calc = true;
    }

    for (size_t i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (take_num_in) {

            char* endptr;
            const int base = OPTIONS::INPUT_BASE;

            rm.numarg = strtol(arg, &endptr, base) + 1; //+1 because without it the depth feels unintuitive
            

            if (*endptr != '\0') {

                std::cerr << "WARNING: tree-depth \"" << arg << "\" is not an integer and will be ignored.\n"; 

                rm.numarg = 12;
                
            }

            take_num_in = false;
            continue; //skip to next arg
        }

        
        if(!strcmp(arg, "-t") || !strcmp(arg, "--table")){ 
            if(rm.extype == DEFAULT) {
                rm.needs_dir_size_calc = true;
                rm.extype = TABLE;
            }

            else {
                print_syntax_error();

                return rm;
            }

        }

        else if(!strcmp(arg, "-b") || !strcmp(arg, "--tree")) {

            if(rm.extype == DEFAULT) {
                rm.needs_dir_size_calc = true;
                rm.extype = TREE;
            }

            else {
                print_syntax_error();

                return rm;
            }

            take_num_in = true;
        }

        else if (!strcmp(arg, "-h") || !strcmp(arg, "--help")) {
            
            rm.extype = HELP;
            
            return rm;
        }

        else if (!strcmp(arg, "-v") || !strcmp(arg, "--version")) {

            if(rm.extype == DEFAULT) rm.extype = VERSION_DISPLAY;

            else {
                print_syntax_error();

                return rm;
            }

        }


        else if(!strcmp(arg, "-c") || !strcmp(arg, "--cmd")) {
            rm.needs_dir_size_calc = true;
            rm.extype = CMDLINE;
        }

        else {

            if (rm.destination == ""){
                rm.destination = arg;
            }
            else {
                print_syntax_error();
                return rm;
            }

        }

    }

    if(rm.extype == DEFAULT && rm.destination != "") {
        rm.extype = TABLE;
        rm.needs_dir_size_calc = true;
    }
 

    return rm;

}

/// @brief calcs full size of given directory in Runmode object
/// @param destination name of the directory (std::filesystem syntax)
/// @return A contentdict containing all information
/// @throw `std::invalid_argument` in case of invalid destination path
Contentdict calc_full_dir_size(std::string destination){

    fs::directory_entry cwd_entry;

    if(destination != "") {
        fs::path p(destination);

        
        if (!fs::exists(p)){
            
            throw std::invalid_argument("Invalid Path given.");
        }

        cwd_entry = fs::directory_entry(p);

    }
    else {
        cwd_entry = fs::directory_entry(fs::current_path());
    }

    Contentdict cdict;
    

    Progress_bar prgbar(cwd_entry);

    return get_size(cwd_entry, &cdict, &prgbar);

}



int main(int argc, const char* argv[]){

    
    Runmode rm = parseArgs(argc, argv);

    Contentdict cdict;

    if(rm.needs_dir_size_calc){
        
        try {

            cdict = calc_full_dir_size(rm.destination);
            
        } catch(std::invalid_argument& ia) {

            std::cerr << "Given directory was not found: " << rm.destination << std::endl;
            return 1;

        }

        load_json();
    }

    Contentdict* pcdict = &cdict;
    Session mainses {pcdict};

    
    switch (rm.extype)
    {
    case TABLE:
        print_cdict_table(cdict);
        return 0;

    case TREE:
        print_cdict_tree(cdict, rm.numarg);
        return 0;  

    case CMDLINE:
        Progress_bar::clear();
        break;
    
    case HELP:
        print_help();
        return 0;
        
    case VERSION_DISPLAY:
        print_version();
        return 0;

    default:
        std::cout << "this should not have happened\n";
        return 1;
    }


    CommandList commands = registerCommands();

    std::string cmd_input; //cmd-line input

    //command line UI
    while (true){

        std::cout << get_cmd_prompt(mainses, *pcdict);
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


        auto it = commands.find(fcmd.name);
        if (it != commands.end()) { 
            it -> second(mainses, fcmd, pcdict);//<---- command parameters here
        }
        else {
            std::cout << warning_str("Unknown command: ") << fcmd.name << "\n";
        }
    }
    
    return 0;
}

