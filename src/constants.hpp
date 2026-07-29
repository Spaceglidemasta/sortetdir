#pragma once
#include <iostream>


#include <string>
#include <iostream>
#include <regex>
#include <iomanip>
#include <sstream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <ctime>

#include <chrono>
using sysclock = std::chrono::system_clock;

#ifdef _WIN32
    #include <windows.h>
#elif __APPLE__
    #include <mach-o/dyld.h>
    #include <sys/stat.h>
#else //unix
    #include <unistd.h>
#endif

//normal part of <windows.h>
#ifndef MAX_PATH
    #define MAX_PATH 260
#endif


namespace fs = std::filesystem;

#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <fstream>


constexpr uint16_t MAX_NAME_LENGTH  = 30;
constexpr uint16_t MAX_TYPE_LENGTH  = 8;
constexpr uint16_t MAX_SIZE_LENGTH  = 12;

constexpr uint16_t GB_BORDER_RED_COLOR =  10;       // (default: 10) the border for when filesizes in GB start to be displayed in PCL::RED.
constexpr uint16_t GB_BORDER_YELLOW_COLOR =  2;     // (default: 2) the border for when filesizes in GB start to be displayed in PCL::YELLOW.
constexpr uint16_t TREE_DEFAULT_MAX_DEPTH =  12;    // (default: 12) default max depth of the tree-view.
constexpr short int TREE_DEFAULT_DEPTH =  -1;       // (default: -1) the default depth to start printing tree-views. No, -1 is not an error. This still worked when this was a uint16_t btw

//A fragment of when there was no cdict for saving the Directory calculations,
//and there was a Row obj. needed to print the table.
// typedef struct _Row {
//     std::string name;   //First column
//     std::string type;   //Second column
//     uintmax_t size;     //Third column
// } Row;



/*
    Yes most of these "constants" are not constant.
    This is for purpose of the json read logic.
*/

#define __DEF2STR(d) #d
#define DEF2STR(d) __DEF2STR(d)

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

enum argsParsingState{
    NEXT_ARG,
    STRING_PARAMETER,
    INT_PARAMETER
};



bool WHAT_ENABLED = false; //Enables support for my upcoming "what" program.

constexpr double_t KB  = 1024.0;
constexpr double_t MB  = (1024.0 * KB);
constexpr double_t GB  = (1024.0 * MB);


//Options namespace containing options that CANNOT be changed in the json file
namespace CMPT_OPTIONS
{

}

//Options namespace containing options that can be changed in the json file
namespace OPTIONS
{
    bool DEBUG    = 0;
    int INPUT_BASE = 10;
    
    #ifndef _JSONPATH
    
    std::string JSON_PATH = "%appdata%\\sortetdir\\config.json";
    
    #else
    
    std::string JSON_PATH = DEF2STR(_JSONPATH);
    
    #endif //_JSONPATH
    
}


//Printcolor namespace. Already contains default values.
namespace PCL
{
    std::string RED                         = "\033[91m";
    std::string BLUE                        = "\033[94m";
    std::string YELLOW                      = "\033[93m";
    std::string CYAN                        = "\033[96m";
    std::string GRAY                        = "\033[90m";
    std::string END                         = "\033[0m";
    std::string UNDERLINE                   = "\033[4m";
    std::string BOLD                        = "\033[1m";
    std::string ITALIC                      = "\033[3m";
    std::string NOFLUSH                     = "\n"; //this is just "\n". Also not included in config.json
}

//UI namespace. Already contains default values.
namespace UI
{
    std::string DIR_TYPE_NAME           = "DIR";
    std::string FILE_TYPE_NAME          = "FILE";
    std::string DEFAULT_TYPE_NAME       = "N/A";
    std::string GB_EXT                  = " GB";
    std::string MB_EXT                  = " MB";
    std::string KB_EXT                  = " KB";
    std::string B_EXT                   = " B";
    
    std::string COMMAND_LINE_LINE       = "$> ";
    std::string PRE_PROMPT              = "[";
    std::string POST_PROMPT             = "]";
    
    size_t      PRGBAR_LEN              = 23;
    std::string PRGBAR_BEGINNING        = "[";
    std::string PRGBAR_FILLER           = "|";
    std::string PRGBAR_EMPTY            = " ";
    std::string PRGBAR_END              = "]";
    
    std::string FIRST_ROW_STR           = "Name";
    std::string SEC_ROW_STR             = "Type";
    std::string THIRD_ROW_STR           = "Size";
    std::string LTITLESYMBOL            = "[";
    std::string RTITLESYMBOL            = "]";
    std::string NOTLAST_RPIPE           = "├─ ";
    std::string LAST_RPIPE              = "╰─ ";
    std::string EMPTY_FILLER            = "\t";
    std::string FILLED_FILLER           = "│\t";
    std::string DOWNPIPE                = "│";
    std::string DOTDOTDOT_STR           = " ...";
    std::string KEY_AND_VALUE_SEPSTR    = ": ";
    
    std::string TABLE_HEADER_COLOR      = PCL::CYAN;
    std::string TABLE_LINE_COLOR        = PCL::BLUE;
    std::string TABLE_LINE_CHAR         = "-";
    
    
    
}

/*
Main structure of this program. Stores most information needed for the program.
*/
struct DirElement {
    
    //!Beware of padding!
    
    std::string key = "";                       //Key / name of the file / directory.
    std::string type = UI::DEFAULT_TYPE_NAME;   //This is for printing only and does not effect code-logic. "DIR", "FILE" or default: "N/A"
    std::vector<DirElement> subdir;          //Content of the directory.
    std::string path = UI::DEFAULT_TYPE_NAME;   //String of the Path; for printing only; does not effect code-logic. "N/A" is default.
    std::optional<sysclock::time_point> creation_date; //creation date of the dir / file in SYSEMTIME. Use print_ctime() to print to the stdout
    
    uint64_t value = 0;                        //Size in Bits. Can be converted to more usefull size-units with size_ext(cdict.value) -> str.
    uint64_t files_contained = 0;            //number of files contained
    uint64_t dirs_contained = 0;              //number of dirs contained
    DirElement* parent = nullptr;            //Pointer to parent dir, default is nullptr.
    
    bool is_invisible = false;                  //Some directories are not visible under some selected OS options. On Linux these files start with "."
    uint16_t symlinks_skipped = 0;              //Counts how many Symlinks have been skipped / are contained because of redundance.
    
};

/// @brief Command-Line-Argument-Context containing results of parsing args
struct Session {
    
    DirElement root;
    
    Execution_t extype;
    std::string destination;
    int numarg;
    std::string path_to_cfg;
    bool needs_dir_size_calc;
    DirElement* homedir = nullptr;
    
};

typedef std::unordered_map<std::string, std::function<void(const Session&, const Command&, DirElement*&)>> CommandList;