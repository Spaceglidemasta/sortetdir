/*
    This file includes the Main logic of the program - reading out file sizes,
    assembling objects, invoking the main cmd loop, etc.

    Functions for UI purposes, aswell as some structues, are located in printing.cpp.

    Github - @spaceglidemasta
*/

//TODO Installation process for bash
//TODO "Did you mean ... (similar str)" for input strings


#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include "util.cpp"
#include "fetching.cpp"







/// @brief arg2 = "C:/programming;
///        arg3 =  scripts"
/// @return "C:/programming scripts"
std::string parserGetFullStrViaQuotes(int argc, const char* argv[], size_t start, int* args_skipped){

    if (start >= argc) {
        std::cerr << "(parserGetFullStrViaQuotes) Garbage function call: start >= argc\n";
        *args_skipped = 0;
        return "";
    }

    argv += start;
    argc -= start;

    
    if (*(argv[0]) != '\"') {

        *args_skipped = 1;

        return std::string(argv[0]);

    } else {

        argv[0]++;

    }

    std::stringstream outss;
    std::stringstream errss;
    bool done = false;

    for (int i = 0; i < argc; i++) {

        std::string arg = argv[i];

        for (char c : arg) {
            if (!done) {
                if (c == '\"') {
                    done = true;
                } else {
                    outss << c;
                }
            } else {
                errss << c;
            }
        }

        if (done) {
            *args_skipped = i + 1;
            break;
        }

        if (i < argc - 1) {
            outss << ' '; // Leerzeichen wiederherstellen, das die Shell zwischen den Args entfernt hat
        }
    }

    if (done) {
        if (!errss.str().empty()) {
            std::cerr << "The following garbage after the ending quotationmarks was ignored: \""
                       << errss.str() << "\"\n";
        }
        return outss.str();
    }

    
    *args_skipped = argc;
    return outss.str();
}



Session parseArgs(int argc, const char* argv[]) {

    Session clactx;
    clactx.extype = DEFAULT;
    clactx.destination = "";
    clactx.numarg = 0;
    clactx.needs_dir_size_calc = false;
    clactx.path_to_cfg = "";

    argsParsingState state = NEXT_ARG;

    if(argc == 1){
        clactx.extype = TABLE;
        clactx.needs_dir_size_calc = true;
    }


    std::string arg;
    for (size_t i = 1; i < argc; i++) {
        arg = argv[i];

        switch (state)
        {
        case NEXT_ARG:
            {
                if((arg == "-t") || (arg == "--table")){ 
                    if(clactx.extype == DEFAULT) {
                        clactx.needs_dir_size_calc = true;
                        clactx.extype = TABLE;
                    }

                    else {
                        print_syntax_error();

                        return clactx;
                    }

                }

                else if((arg == "-b") || (arg == "--tree")) {

                    if(clactx.extype == DEFAULT) {
                        clactx.needs_dir_size_calc = true;
                        clactx.extype = TREE;
                    }

                    else {
                        print_syntax_error();

                        return clactx;
                    }

                    state = INT_PARAMETER;
                }

                else if ((arg == "-h") || (arg == "--help")) {
                    
                    clactx.extype = HELP;
                    
                    return clactx;
                }

                else if ((arg == "-j") || (arg == "--config") || (arg == "--cfg")) {

                    state = STRING_PARAMETER;
                    
                    
                }

                else if ((arg == "-v") || (arg == "--version")) {

                    if(clactx.extype == DEFAULT) clactx.extype = VERSION_DISPLAY;

                    else {
                        print_syntax_error();

                        return clactx;
                    }

                }


                else if((arg == "-c") || (arg == "--cmd")) {
                    clactx.needs_dir_size_calc = true;
                    clactx.extype = CMDLINE;
                }

                else {

                    if (clactx.destination == ""){
                        clactx.destination = arg;
                    }
                    else {
                        print_syntax_error();
                        return clactx;
                    }

                }
                
                break;
            }
        
        case STRING_PARAMETER:
            {

                int skipped;
                clactx.path_to_cfg = parserGetFullStrViaQuotes(argc, argv, i, &skipped);

                #ifdef DEBUG
                std::cout << "String Parameter:" << clactx.path_to_cfg << std::endl;
                #endif

                i += (skipped - 1);
                state = NEXT_ARG;

                break;


            }
            
            
            

        case INT_PARAMETER:
            {
                char* endptr;
                const int base = OPTIONS::INPUT_BASE;

                clactx.numarg = strtol(arg.c_str(), &endptr, base) + 1; //+1 because without it the depth feels unintuitive
                

                if (*endptr != '\0') {

                    std::cerr << "WARNING: tree-depth \"" << arg << "\" is not an integer and will be ignored.\n"; 

                    clactx.numarg = 12;
                    
                }

                state = NEXT_ARG;
                continue; //skip to next arg
            }

            default:
            {
                std::cerr << "This should never happen. You memory is corrupted.\n";
                
                clactx.extype = INVALID;
                return clactx;
            }
        }

    }

    if(clactx.extype == DEFAULT && clactx.destination != "") {
        clactx.extype = TABLE;
        clactx.needs_dir_size_calc = true;
    }

    return clactx;

}


/// @brief calcs full size of given directory in Session object
/// @param destination name of the directory (std::filesystem syntax)
/// @return A contentdict containing all information
/// @throw `std::invalid_argument` in case of invalid destination path
DirElement calc_full_dir_size(std::string destination){

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

    DirElement cdict;
    

    Progress_bar prgbar(cwd_entry);

    return get_size(cwd_entry, &cdict, &prgbar);

}


CommandList registerCommands(){

    CommandList commands;
    //shows descriptions for every command
    commands["help"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout   << PCL::BOLD << "Commands:\n" << PCL::END 
                    << "cd <arg>        -> Change Directory to arg.\n"
                    << "                   default: cd's into the home directory.\n"
                    << "                   arg1: specifies the target.\n"
                    << "                   \"cd ..\": cd's one directory up.\n"
                    << "help            -> prints this message\n"
                    << "q               -> "<< bold_str("q") << "uits the programm.\n"
                    << "table           -> prints the standart sorted table\n"
                    << "tree <arg>      -> prints a file tree of the current dir with a given depth <arg>\n"
                    << "cls             -> " << bold_str("cl") << "ears the " << bold_str("s") << "creen\n"
                    << "pwd             -> "<< bold_str("p") << "rints " << bold_str("w") << "orking " << bold_str("d") << "irectory\n"
                    << "info <arg>      -> Gives information about the creation of arg, and how\n"
                    << "                   many files are in the directory if arg is one.\n"
                    << "                   arg1: specifies the target, default is the current path.\n"
                    << "reload <arg>    -> reloads strings from the given path or the default path of the config if no arg given\n"

                    << std::endl;
    };
    
    //quits the program
    commands["q"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){
        std::exit(0);
    };

    /*
        Uses pointer logic to change up in down in the cdict directory hirachy.
        Behaves like "cd" ON LINUX.

        Used to be handled by having the whole contendict system beeing pointer based,
        which turned out to be to complicated for its simple use.
    */
    commands["cd"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

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
        for(DirElement& entry : cdict -> subdir){
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

    #ifdef _PERSONAL_MODE
    //the "what" program is not yet released  / shit af, so you can safely ignore this and the "what" command
    commands["what"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

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
    #endif

    //TODO target selection
    //prints a tree view of the cdict. Try it.
    commands["tree"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

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
    commands["table"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        if(cmd.args.empty()){
            print_cdict_table(*cdict); 
        }
        else {

            std::string fullargs = merge_str(cmd.args);

            for(DirElement& entry : cdict -> subdir){
                if(entry.key == fullargs){
                    
                    print_cdict_table(entry);

                    return;
                }
            }

        //else (given directory was not found)
        std::cout << "Target \"" << fullargs << "\" was not found.\n";

        }

        
    };

    //Clears the screen. Windows's "cls"
    commands["cls"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

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
    commands["pwd"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        if(!(cmd.args.empty())){
            std::cout << info_str("This command does not take args. They were ignored.") << std::endl; 
        }

        std::cout << cdict -> path << std::endl;
    };

    //Reload json config
    commands["reload"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        std::string fullargs = merge_str(cmd.args);

        std::string printpath = (fullargs.empty() ? parseEnvVars(OPTIONS::JSON_PATH) : parseEnvVars(fullargs));

        if(!load_json(fullargs)){
            std::cerr << "json config was reloaded from \"" << printpath << "\"\n";
        }
        else {
            std::cerr << info_str("json config could not be reloaded from ") << "\"" << printpath << "\" successfully.\n";
        }

    };

    #ifdef _PERSONAL_MODE

    //test and debugg
    commands["test"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        std::string fullargs = merge_str(cmd.args);

        int skipped;
        const char** argtext = (const char**) strvecToCharPtrArr(cmd.args);

        int i = 0;
        for(std::string s : cmd.args){

            if( s == "-j"){

                std::string out = parserGetFullStrViaQuotes(cmd.args.size(), argtext, i + 1, &skipped);

                std::cout << "Skipped: " << skipped << std::endl;
                std::cout << "Args: " << cmd.args.size() << std::endl;
                std::cout << ":: " << out << std::endl;

            }

            i++;
        }

    };

    #endif
        
    

    commands["info"] = [](const Session& ses, const Command& cmd, DirElement*& cdict){

        if(cmd.args.empty()){

            cdict_set_filetime(*cdict, cdict -> path);

            std::cout << "\nSize of current directiory: " << size_ext(cdict -> value) << std::endl;

            print_ctime(*cdict);

            std::cout << "Contains: " << cdict -> files_contained << " Files, " << cdict -> dirs_contained << " Folders" << std::endl;
            
            return;
        }
        //else: args are given

        
        std::string fullargs = merge_str(cmd.args);

        for(DirElement& entry : cdict -> subdir){
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


int main(int argc, const char* argv[]){

    
    Session mainsession = parseArgs(argc, argv);


    if(mainsession.needs_dir_size_calc){
        
        try {

            mainsession.root = calc_full_dir_size(mainsession.destination);
            mainsession.homedir = &mainsession.root;
            
        } catch(std::invalid_argument& ia) {

            std::cerr << "Given directory was not found: " << mainsession.destination << std::endl;
            return 1;

        }

        if(load_json(mainsession.path_to_cfg)) std::cerr << "ifstream: config.json could not be loaded.: " << get_path_of_exe() / "config.json" << std::endl;
        
    }

    //create session obj. with this directory as home-directory
    DirElement* proot = &(mainsession.root);   
           

    
    switch (mainsession.extype)
    {
    case TABLE:
        print_cdict_table(mainsession.root);
        return 0;

    case TREE:
        print_cdict_tree(mainsession.root, mainsession.numarg);
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

    //Map: std::string -> void (*_) (const Session&, const Command&, DirElement*&)
    CommandList commands = registerCommands();

    std::string cmd_input; //cmd-line input

    //command line UI
    while (true){

        std::cout << get_cmd_prompt(mainsession, *proot);
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
            it -> second(mainsession, fcmd, proot);//<---- command parameters here
        }
        else {
            std::cout << warning_str("Unknown command: ") << fcmd.name << "\n";
        }
    }
    
    return 0;
}

