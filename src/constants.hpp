#pragma once
#include <iostream>

//IMPORTANT: https://tenor.com/view/aple-gif-27463919

/*
    Yes most of these "constants" are not constant.
    This is for purpose of the json read logic.
*/

bool WHAT_ENABLED = false; //Enables support for my upcoming what program.

constexpr double_t KB  = 1024.0;
constexpr double_t MB  = (1024.0 * KB);
constexpr double_t GB  = (1024.0 * MB);

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

    size_t PROGRESS_BAR_LENGTH          = 23;

    std::string FIRST_ROW_STR           = "Name";
    std::string SEC_ROW_STR             = "Type";
    std::string THIRD_ROW_STR           = "Size";
    std::string PIPE_DOWN_STR           = "├";
    std::string VERTICAL_PIPE_STR       = "│";
    std::string DIR_ARROW_STR           = "┼> ";
    std::string CROSS_PIPE_STR          = "┼";
    std::string FILE_ARROW_STR          = "> ";
    std::string EMPTY_DEPTH_SEPSTR      = "      ";
    std::string FILLED_DEPTH_SEPSTR     = "──────";
    std::string DOTDOTDOT_STR           = " ...";
    std::string KEY_AND_VALUE_SEPSTR    = ": ";

    std::string TABLE_LINE_CHAR          = "-";

}