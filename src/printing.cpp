/*
    This file contains functions and structs that are mostly for UI / printing purposes,
    or to modify and create strings. Exceptions are the Contentdict and Row structures.

    More constants may be located in constants.hpp.

    Github - @spaceglidemasta
*/

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
using cock = std::chrono::system_clock;

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
#include "constants.hpp"

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

//TODO remove cock and replace set_ctime (thingy) with print_ctime



/*
    Main structure of this program. Store most information needed for the directories.
*/
struct Contentdict {

    //!Beware of padding!

    std::string key = "";                       //Key / name of the file / directory.
    std::string type = UI::DEFAULT_TYPE_NAME;   //This is for printing only and does not effect code-logic. "DIR", "FILE" or default: "N/A"
    std::vector<Contentdict> subdir;          //Content of the directory.
    std::string path = UI::DEFAULT_TYPE_NAME;   //String of the Path; for printing only; does not effect code-logic. "N/A" is default.
    std::optional<cock::time_point> creation_date; //creation date of the dir / file in SYSEMTIME. Use print_ctime() to print to the stdout

    uint64_t value = 0;                        //Size in Bits. Can be converted to more usefull size-units with size_ext(cdict.value) -> str.
    uint64_t files_contained = 0;            //number of files contained
    uint64_t dirs_contained = 0;              //number of dirs contained
    Contentdict* parent = nullptr;            //Pointer to parent dir, default is nullptr.
    
    bool is_invisible = false;                  //Some directories are not visible under some selected OS options. On Linux these files start with "."
    uint16_t symlinks_skipped = 0;              //Counts how many Symlinks have been skipped / are contained because of redundance.

};


class Progress_bar {
    private:
    double step_size;
    double processed = 0.0;

    public:
    uintmax_t total = 0;
    

    static void clear() {

        std::cout << "\x1b[2K";
        std::cout << "\r";
        
    }


    bool update_progressbar() {
        if (UI::PRGBAR_LEN < 2) return true;

        if(std::floor(processed) == std::floor(processed + step_size)) {
            processed += step_size;
            return false;
        }

        processed += step_size;

        std::cout << "\r" << UI::PRGBAR_BEGINNING;
        for(int i = 0; i <= std::floor(processed); i++){
            std::cout << UI::PRGBAR_FILLER;
        }
        for(int i = std::floor(processed); i < UI::PRGBAR_LEN - 1; i++){
            std::cout << UI::PRGBAR_EMPTY;
        }
        std::cout << UI::PRGBAR_END;

        //if(std::floor(processed) >= UI::PRGBAR_LEN) std::cout << std::endl;

        return false;
    }

    Progress_bar(const fs::directory_entry& entry) {
        for (const auto& _ : fs::directory_iterator(entry.path(), fs::directory_options::skip_permission_denied))
            total++;

        step_size = double(UI::PRGBAR_LEN) / double(total);
    }
};

struct Session {
    Contentdict* homedir = nullptr;
};



//example: "Creation time of sortetdircpp: Thu Aug 14 21:51:43 2025" + std::endl
bool print_ctime(const Contentdict& cdict){
    if(!cdict.creation_date){
        if(OPTIONS::DEBUG)  std::cout << "Cdict has no creation_date: " << cdict.key << std::endl;
        return false;
    }
    std::time_t t = cock::to_time_t(cdict.creation_date.value());
    std::cout << "Creation time of " << cdict.key << ": " << std::ctime(&t);

    return true;
}

//returns the fs::path of the location of the .exe
fs::path get_path_of_exe() {
    char buffer[MAX_PATH];

    #ifdef _WIN32

        GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        fs::path exe_path(buffer);

    #elif __APPLE__
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0){
            fs::path exe_path(buffer);
        }
        else {
            // Fallback: buffer too small, allocate dynamically
            std::string path(size, '\0');
            _NSGetExecutablePath(path.data(), &size);
            fs::path exe_path(path);
        }
    #else
        ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (count != -1) {
            buffer[count] = '\0';
        }
        else {
            buffer[0] = '\0';
        }
        fs::path exe_path(buffer);
        
    #endif


    return exe_path.parent_path();
}




/*
    loads all the vars from config.json
    uses nlohmann/json
*/
bool load_json(){

    #ifdef _WIN32

        const char* appd = std::getenv("APPDATA");

    #else  //linux & macos

        const char* appd = std::getenv("HOME");

    #endif

    std::ifstream file(std::string(appd != nullptr ? appd : ".") + "\\sortetdir\\config.json");

    if(!file.is_open()){
        std::cerr << "ifstream: config.json could not be loaded.: " << get_path_of_exe() / "config.json" << std::endl;
        return 1;
    }

    json json_data;
    file >> json_data;

    if(json_data.contains("OPTIONS")) {
        auto& options = json_data["OPTIONS"];
        if(options.contains("DEBUG")) OPTIONS::DEBUG = options["DEBUG"];
    }

    if (json_data.contains("PCL")) {
        auto& pcl = json_data["PCL"];

        #define JSON_CHECK_N_SET(name) if (pcl.contains(#name)) PCL::name = pcl[#name];

        JSON_CHECK_N_SET(RED)
        JSON_CHECK_N_SET(YELLOW)
        JSON_CHECK_N_SET(BLUE)
        JSON_CHECK_N_SET(GRAY)
        JSON_CHECK_N_SET(CYAN)
        JSON_CHECK_N_SET(END)
        JSON_CHECK_N_SET(UNDERLINE)
        JSON_CHECK_N_SET(BOLD)
        JSON_CHECK_N_SET(ITALIC)

        #undef JSON_CHECK_N_SET
    }

    if (json_data.contains("UI")) {
        auto& ui = json_data["UI"];

        #define JSON_CHECK_N_SET(name) if (ui.contains(#name)) UI::name = ui[#name];

        JSON_CHECK_N_SET(DIR_TYPE_NAME)
        JSON_CHECK_N_SET(FILE_TYPE_NAME)
        JSON_CHECK_N_SET(DEFAULT_TYPE_NAME)
        JSON_CHECK_N_SET(GB_EXT)
        JSON_CHECK_N_SET(MB_EXT)
        JSON_CHECK_N_SET(KB_EXT)
        JSON_CHECK_N_SET(B_EXT)
        JSON_CHECK_N_SET(COMMAND_LINE_LINE)
        JSON_CHECK_N_SET(PRGBAR_LEN)
        JSON_CHECK_N_SET(PRGBAR_BEGINNING)
        JSON_CHECK_N_SET(PRGBAR_FILLER)
        JSON_CHECK_N_SET(PRGBAR_EMPTY)
        JSON_CHECK_N_SET(PRGBAR_END)

        JSON_CHECK_N_SET(PRE_PROMPT)
        JSON_CHECK_N_SET(POST_PROMPT)
        JSON_CHECK_N_SET(FIRST_ROW_STR)
        JSON_CHECK_N_SET(SEC_ROW_STR)
        JSON_CHECK_N_SET(THIRD_ROW_STR)
        JSON_CHECK_N_SET(PIPE_DOWN_STR)
        JSON_CHECK_N_SET(VERTICAL_PIPE_STR)
        JSON_CHECK_N_SET(DIR_ARROW_STR)
        JSON_CHECK_N_SET(CROSS_PIPE_STR)
        JSON_CHECK_N_SET(FILE_ARROW_STR)
        JSON_CHECK_N_SET(EMPTY_DEPTH_SEPSTR)
        JSON_CHECK_N_SET(FILLED_DEPTH_SEPSTR)
        JSON_CHECK_N_SET(DOTDOTDOT_STR)
        JSON_CHECK_N_SET(KEY_AND_VALUE_SEPSTR)

        JSON_CHECK_N_SET(TABLE_LINE_CHAR)
        JSON_CHECK_N_SET(TABLE_HEADER_COLOR)
        JSON_CHECK_N_SET(TABLE_LINE_COLOR)

        #undef JSON_CHECK_N_SET
    }

    return 0;
}


/*
    returns the path of the cdict relative to its .home_dir

    Example:
    cdict.path = "C:Users\Nutzer\Documents\programming scripts\C++"
    cdict.home_dir = "C:Users\Nutzer\Documents"

    returns -> "Documents\programming scripts\C++"
*/
std::string short_path(const Session& ses, const Contentdict& cdict) {
    //if we are at the start / home directory
    if (ses.homedir == &cdict) {
        return cdict.key;
    }

    std::string spath = cdict.path;
    size_t len = ses.homedir -> path.length();

    //C++ string manip is a pain in the a$$
    if(len != std::string::npos){
        spath.erase(0, len + 1); //@Python spath = spath[len + 1:]
    }


    return ses.homedir -> key + "\\" + spath;
}

//truncates the string to a max size of param:width, and adds a "..."
std::string truncate(const std::string& s, std::size_t width) {
    if (s.size() <= width) return s;
    else if (width <= 3) return s.substr(0, width);
    else return s.substr(0, width - 4) + "...";
}

//magic regex function :)
//fun-fact: The regex u learn in uni does not help you with this!
uint16_t ansii_code_length(std::string str){
    static const std::regex clrrgx("\033\\[[0-9;]*m");
    return std::regex_replace(str, clrrgx, "").length();
}

uint16_t ansii_code_len_by_val(uintmax_t size){

    std::string big_warning = PCL::RED;
    std::string small_warning = PCL::YELLOW;

    if(size >= GB){
        //warns the user of big file sizes
        if(size / GB > GB_BORDER_RED_COLOR) {
            return big_warning.length();
        }
        else if(size / GB > GB_BORDER_YELLOW_COLOR) {
            return small_warning.length();
        }
    }

    return 0;

}

//makes sense out of 232328 bits. 232328 -> "226.88 KB"
std::string size_ext(uintmax_t size){

    //rare cpp tutorium usage
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2); //2 digit decimal-point precision

    std::string big_warning = PCL::RED;
    std::string small_warning = PCL::YELLOW;

    if(size >= GB){
        //warns the user of big file sizes
        if(size / GB > GB_BORDER_RED_COLOR) {
            oss << big_warning << (size / GB) << PCL::END << UI::GB_EXT;
        }
        else if(size / GB > GB_BORDER_YELLOW_COLOR) {
            oss << small_warning << (size / GB) << PCL::END << UI::GB_EXT;
        }
        else {
            oss << (size / GB) << UI::GB_EXT;
        }
    }
    else if(size >= MB){
        oss << (size / MB) << UI::MB_EXT;
    }
    else if(size >= KB){
        oss << (size / KB) << UI::KB_EXT;
    }
    else{
        oss << size << UI::B_EXT;
    }

    return oss.str();
}

//repeats a string :param:times
std::string stringtimes(std::string str, int times){

    std::ostringstream oss;

    for (uint16_t i = 0; i < times; i++){
        oss << str;
    }
    
    return oss.str();
}

//"Command arg1 arg2" -> ["Command","arg1","arg2"]
std::vector<std::string> split_by_space(const std::string& input) {
    std::istringstream iss(input);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string warning_str(const std::string& str){
    return PCL::BOLD + PCL::RED + str + PCL::END;
}

std::string info_str(const std::string& str){
    return PCL::BOLD + PCL::YELLOW + str + PCL::END;
}

std::string bold_str(const std::string& str){
    return PCL::BOLD + str + PCL::END;
}

//joins the strings in :param:vstr with spaces
std::string merge_str(const std::vector<std::string>& vstr){
    std::string retstr = "";
    bool first = true;
    for(const std::string& str : vstr){
        if(first){
            retstr += str;
            first = false;
        }
        else retstr += (" " + str);
    }
    return retstr;
}

/**
 * # print_cdict_tree
 * 
 * Prints the contentdict in a tree-like view
 * 
 * @param cdict The contentdict to print in a tree-like view.
 * @param max_depth Maximum depth to print (default 12).
 * @param depth Current recursion depth (optional, default 0). Can be invoked with -1 to create a more flat tree.
 * @param first True if this is the first call (optional, default false).
 */
void print_cdict_tree(const Contentdict& cdict, short int max_depth = 12, short int depth = 0, bool first = false){

    // transfere to when loading bar is over
    if (depth == 0) std::cout << std::endl;

    if(cdict.subdir.empty() && first == true){
        std::cout << info_str("This directory is empty.") << std::endl;
        return;
    }

    //is a dir
    if(cdict.subdir.size()){

        if(first) std::cout << PCL::BOLD << cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::END << PCL::NOFLUSH;
        else if(depth <= 0){
            std::cout   << UI::PIPE_DOWN_STR << UI::FILE_ARROW_STR
                        << cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::NOFLUSH;
        }
        else{
            std::cout   << stringtimes(UI::VERTICAL_PIPE_STR + UI::EMPTY_DEPTH_SEPSTR, depth - 1)
                        << UI::PIPE_DOWN_STR + UI::FILLED_DEPTH_SEPSTR << UI::DIR_ARROW_STR
                        << cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::NOFLUSH;
        }

        /*
            checking that the max_depth is not surpassed.

            yes, this leaves out recursions, but this doesnt mather, because
            since Version 2.0 printing and the calc. of the cdict are seperate.
        */
        if(depth + 1 == max_depth){
            std::cout << PCL::GRAY <<stringtimes(UI::VERTICAL_PIPE_STR + UI::EMPTY_DEPTH_SEPSTR, depth + 1) << UI::DOTDOTDOT_STR << PCL::END <<PCL::NOFLUSH;
            return;
        }
        
        for(const Contentdict& subdict : cdict.subdir){
            print_cdict_tree(subdict, max_depth, depth + 1);
        }
    }
    //is a normal file
    else{
        std::cout   << stringtimes(UI::VERTICAL_PIPE_STR + UI::EMPTY_DEPTH_SEPSTR, depth) << UI::PIPE_DOWN_STR << UI::FILE_ARROW_STR
                    <<  cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::END << PCL::NOFLUSH;
    }

    if(first) std::cout << "\nSize of current directiory: " << size_ext(cdict.value) << std::endl;

}

void print_new_tree(
    const Contentdict& cdict,
    uint16_t max_depth = 12,
    uint16_t depth = 0,
    bool first = false,
    bool last = false,
    std::string indentstr = ""
){
    
    if(first) std::cout << std::endl;
    

    std::cout << indentstr << (last ? "╰─>" : "├─>")  << (cdict.subdir.empty() ? "" : PCL::BLUE) << cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::END << PCL::NOFLUSH;
    



    if(cdict.subdir.empty() && first == true){
        std::cout << info_str("This directory is empty.") << std::endl;
        return;
    }

    if (cdict.subdir.size()) {

        size_t i = 0;

        for(const Contentdict& subdict : cdict.subdir){

            i++;

            std::string newindentstr = indentstr + (last ? "\t" : "│\t");

            print_new_tree(
                subdict,
                max_depth,
                depth + 1,
                false,
                (i == cdict.subdir.size()),
                newindentstr
            );
            
        }
    }


    if(first) std::cout << "\nSize of current directiory: " << size_ext(cdict.value) << std::endl;

}

void print_cmdargs_help() {
    std::cout << "Wrong usage! Correct usage: ls <flag>" << std::endl;
    std::cout << "Flags: tree, table (standard), cmd";
}


/**
 * # print_cdict_table
 * 
 * Prints the contendict as a table, sorted by cdict.value.
 * 
 * @param cdict The contentdict that is supposed to be printed.
 */
void print_cdict_table(const Contentdict& cdict){

    if(cdict.subdir.empty()){
        std::cout << info_str("This directory is empty.") << std::endl;
        return;
    }

    Progress_bar::clear();

    //the final table that is printed using setw()
    std::vector<Contentdict> table = cdict.subdir;

    //stable sort by cdict.value
    std::stable_sort(
        table.begin(),
        table.end(),
        [](const Contentdict& a, const Contentdict& b) {
            return a.value > b.value;
        }
    );


    std::string primcol;
    std::string seccol; 

    //personal modification of mine. Compile via "make winper / uniper" to use
    #ifdef _PERSONAL_MODE

        char* primcolenv = std::getenv("primcol");
        char* seccolenv = std::getenv("seccol");

        if (primcolenv == NULL) primcol = UI::TABLE_HEADER_COLOR;
        else                    primcol = "\033[" + std::string(primcolenv) + "m";
             
        if (seccolenv == NULL) seccol = UI::TABLE_LINE_COLOR;
        else                   seccol = "\033[" + std::string(seccolenv)  + "m";

    #else

        primcol = UI::TABLE_HEADER_COLOR;
        seccol = UI::TABLE_LINE_COLOR;

    #endif


    //Header
    std::cout << std::left << primcol
              << std::setw(MAX_NAME_LENGTH) << UI::FIRST_ROW_STR
              << std::setw(MAX_TYPE_LENGTH) << UI::SEC_ROW_STR
              << std::right << std::setw(MAX_SIZE_LENGTH) << UI::THIRD_ROW_STR << PCL::NOFLUSH << seccol;
    std::cout << std::string(MAX_NAME_LENGTH + MAX_TYPE_LENGTH + MAX_SIZE_LENGTH, (UI::TABLE_LINE_CHAR)[0]) << PCL::NOFLUSH;
    std::cout << PCL::END;

    
    std::string sizestr;
    //Table content
    for (const Contentdict& cdict_row : table) {

        sizestr = size_ext(cdict_row.value);

        uint16_t visible_len = ansii_code_length(sizestr);
        uint16_t pad = (visible_len < MAX_SIZE_LENGTH) ? (MAX_SIZE_LENGTH - visible_len) : 0;

        
        std::cout << (cdict_row.is_invisible ? PCL::GRAY : "") << std::left
                  << std::setw(MAX_NAME_LENGTH) << truncate(cdict_row.key, MAX_NAME_LENGTH)
                  << std::setw(MAX_TYPE_LENGTH) << cdict_row.type
                  << std::string(pad, ' ') << sizestr
                  << PCL::NOFLUSH << PCL::END;             
    }

    std::cout << "\nSize of current directiory: " << size_ext(cdict.value) << std::endl;

    if(cdict.symlinks_skipped) std::cout << "Number of Dirs / Symlinks skipped: " << cdict.symlinks_skipped << std::endl;

}