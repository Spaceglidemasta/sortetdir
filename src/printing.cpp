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

//A fragment of when there was not dict for saving the Directory calculations,
//and there was a Row obj. needed to print the table.
// typedef struct _Row {
//     std::string name;   //First column
//     std::string type;   //Second column
//     uintmax_t size;     //Third column
// } Row;

typedef struct _cdict {
    
    std::string key = "";                       //Key / name of the file / directory.
    std::string type = UI::DEFAULT_TYPE_NAME;   //This is for printing only and does not effect code-logic. "DIR", "FILE" or default: "N/A"
    uintmax_t value = 0;                        //Size in Bits. Can be converted to more usefull size-units with size_ext(cdict.value) -> str.
    std::vector<struct _cdict> subdir;          //Content of the directory.

    struct _cdict* parent = nullptr;            //Pointer to parent dir, default is nullptr.
    bool is_invisible = false;                  //Some directories are not visible under some selected OS options. On Linux these files start with "."
    std::string path = UI::DEFAULT_TYPE_NAME;   //String of the Path; for printing only; does not effect code-logic. "N/A" is default.
    uint16_t symlinks_skipped = 0;              //Counts how many Symlinks have been skipped / are contained because of redundance.
    struct _cdict* home_dir = nullptr;          //the home directory / the dir the program starts in. Is passed onto every subdir.

} Contentdict;


bool load_json(){

    std::ifstream file("config.json");
    if(!file.is_open()){
        std::cerr << "ifstream: config.json could not be loaded." << std::endl;
        return 1;
    }

    json json_data;
    file >> json_data;

    if (json_data.contains("PCL")) {
        auto& pcl = json_data["PCL"];

        if (pcl.contains("RED"))                PCL::RED                     = pcl["RED"];
        if (pcl.contains("YELLOW"))             PCL::YELLOW                  = pcl["YELLOW"];
        if (pcl.contains("BLUE"))               PCL::BLUE                    = pcl["BLUE"];
        if (pcl.contains("GRAY"))               PCL::GRAY                    = pcl["GRAY"];
        if (pcl.contains("CYAN"))               PCL::CYAN                    = pcl["CYAN"];
        if (pcl.contains("END"))                PCL::END                     = pcl["END"];
        if (pcl.contains("UNDERLINE"))          PCL::UNDERLINE               = pcl["UNDERLINE"];
        if (pcl.contains("BOLD"))               PCL::BOLD                    = pcl["BOLD"];
        if (pcl.contains("ITALIC"))             PCL::ITALIC                  = pcl["ITALIC"];
        if (pcl.contains("NOFLUSH"))            PCL::NOFLUSH                 = pcl["NOFLUSH"];
    }

    if (json_data.contains("UI")) {
        auto& ui = json_data["UI"];

        if (ui.contains("DIR_TYPE_NAME"))        UI::DIR_TYPE_NAME           = ui["DIR_TYPE_NAME"];
        if (ui.contains("FILE_TYPE_NAME"))       UI::FILE_TYPE_NAME          = ui["FILE_TYPE_NAME"];
        if (ui.contains("DEFAULT_TYPE_NAME"))    UI::DEFAULT_TYPE_NAME       = ui["DEFAULT_TYPE_NAME"];
        if (ui.contains("GB_EXT"))               UI::GB_EXT                  = ui["GB_EXT"];
        if (ui.contains("MB_EXT"))               UI::MB_EXT                  = ui["MB_EXT"];
        if (ui.contains("KB_EXT"))               UI::KB_EXT                  = ui["KB_EXT"];
        if (ui.contains("B_EXT"))                UI::B_EXT                   = ui["B_EXT"];
        if (ui.contains("COMMAND_LINE_LINE"))    UI::COMMAND_LINE_LINE       = ui["COMMAND_LINE_LINE"];
        if (ui.contains("FIRST_ROW_STR"))        UI::FIRST_ROW_STR           = ui["FIRST_ROW_STR"];
        if (ui.contains("SEC_ROW_STR"))          UI::SEC_ROW_STR             = ui["SEC_ROW_STR"];
        if (ui.contains("THIRD_ROW_STR"))        UI::THIRD_ROW_STR           = ui["THIRD_ROW_STR"];
        if (ui.contains("PIPE_DOWN_STR"))        UI::PIPE_DOWN_STR           = ui["PIPE_DOWN_STR"];
        if (ui.contains("VERTICAL_PIPE_STR"))    UI::VERTICAL_PIPE_STR       = ui["VERTICAL_PIPE_STR"];
        if (ui.contains("DIR_ARROW_STR"))        UI::DIR_ARROW_STR           = ui["DIR_ARROW_STR"];
        if (ui.contains("CROSS_PIPE_STR"))       UI::CROSS_PIPE_STR          = ui["CROSS_PIPE_STR"];
        if (ui.contains("FILE_ARROW_STR"))       UI::FILE_ARROW_STR          = ui["FILE_ARROW_STR"];
        if (ui.contains("EMPTY_DEPTH_SEPSTR"))   UI::EMPTY_DEPTH_SEPSTR      = ui["EMPTY_DEPTH_SEPSTR"];
        if (ui.contains("FILLED_DEPTH_SEPSTR"))  UI::FILLED_DEPTH_SEPSTR     = ui["FILLED_DEPTH_SEPSTR"];
        if (ui.contains("DOTDOTDOT_STR"))        UI::DOTDOTDOT_STR           = ui["DOTDOTDOT_STR"];
        if (ui.contains("KEY_AND_VALUE_SEPSTR")) UI::KEY_AND_VALUE_SEPSTR    = ui["KEY_AND_VALUE_SEPSTR"];
    }

    return 0;
}


std::string short_path(const Contentdict& cdict) {
    if (cdict.home_dir == nullptr) {
        return cdict.key;
    }

    std::string spath = cdict.path;
    size_t len = cdict.home_dir -> path.length();

    if(len != std::string::npos){
        spath.erase(0, len - 1);
    }

    return cdict.home_dir -> key + "\\" + spath;
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
    static const std::regex clrrgx("\033\\[[0-9;]*m|.");
    return std::regex_replace(str, clrrgx, "$1").length();
}

//makes sense out of 232328 bits. 232328 -> "226.88 KB"
std::string size_ext(uintmax_t size){

    //rare cpp tutorium usage
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2); //2 digit decimal-point precision

    if(size >= GB){
        //warns the user of big file sizes
        if(size / GB > GB_BORDER_RED_COLOR) oss << PCL::RED << (size / GB) << PCL::END << UI::GB_EXT;
        else if(size / GB > GB_BORDER_YELLOW_COLOR) oss << PCL::YELLOW << (size / GB) << PCL::END << UI::GB_EXT;
        else oss << (size / GB) << UI::GB_EXT;
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
    for(const auto& str : vstr){
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
            std::cout << stringtimes(UI::VERTICAL_PIPE_STR + UI::EMPTY_DEPTH_SEPSTR, depth + 1) << UI::DOTDOTDOT_STR << PCL::NOFLUSH;
            return;
        }
        
        for(const auto& subdict : cdict.subdir){
            print_cdict_tree(subdict, max_depth, depth + 1);
        }
    }
    //is a normal file
    else{
        std::cout   << stringtimes(UI::VERTICAL_PIPE_STR + UI::EMPTY_DEPTH_SEPSTR, depth) << UI::PIPE_DOWN_STR << UI::FILE_ARROW_STR
                    << PCL::GRAY <<  cdict.key << UI::KEY_AND_VALUE_SEPSTR << size_ext(cdict.value) << PCL::END << PCL::NOFLUSH;
    }

    if(first) std::cout << "\nSize of current directiory: " << size_ext(cdict.value) << std::endl;

};


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

    //the final table that is printed using setw()
    std::vector<Contentdict> table = cdict.subdir;

    //stable sort by cdict.value
    std::stable_sort(table.begin(), table.end(),[](const Contentdict& a, const Contentdict& b) {return a.value > b.value;});

    //Header
    std::cout << std::left
              << std::setw(MAX_NAME_LENGTH) << UI::FIRST_ROW_STR
              << std::setw(MAX_TYPE_LENGTH) << UI::SEC_ROW_STR
              << std::right << std::setw(MAX_SIZE_LENGTH) << UI::THIRD_ROW_STR << PCL::NOFLUSH;
    std::cout << std::string(MAX_NAME_LENGTH + MAX_TYPE_LENGTH + MAX_SIZE_LENGTH, '-') << PCL::NOFLUSH;

    //Less memory usage :)
    std::string sizestr;
    //Table content
    for (const auto& row : table) {

        sizestr = size_ext(row.value);

        if(row.is_invisible) std::cout << PCL::GRAY;

        //I hate setwhite
        std::cout << std::left
                  << std::setw(MAX_NAME_LENGTH) << truncate(row.key, MAX_NAME_LENGTH)
                  << std::setw(MAX_TYPE_LENGTH) << row.type
                  << std::right << std::setw(MAX_SIZE_LENGTH - ansii_code_length(sizestr)) << sizestr
                  << PCL::NOFLUSH << PCL::END;
                  
    }

    std::cout << "\nSize of current directiory: " << size_ext(cdict.value) << std::endl;

    if(cdict.symlinks_skipped) std::cout << "Number of Dirs / Symlinks skipped: " << cdict.symlinks_skipped << std::endl;

}