# sortetdir

A terminal-based C++ application that shows how much space each directory takes up — perfect for digital spring-cleaning!

## Features
- Displays directory sizes in a clean, readable format  
- Helps identify large folders quickly  
- Lightweight and fast  

## Dependencies
This project uses the following external libraries:
- [`nlohmann/json`](https://github.com/nlohmann/json) – for parsing and handling JSON data

Make sure it’s installed or available in your include path before building.

If you’re using **vcpkg**, you can install it via:
```bash
vcpkg install nlohmann-json
```

## Installation
You can install this project with
```bash
git clone https://github.com/Spaceglidemasta/sortetdir
cd sortetdir\include
git clone https://github.com/nlohmann/json.git include/json
```

and then compile it using the C++ compiler of your choice.

I also recommend adding the directory to PATH and changing the .exe
name to something that suits your usage.

## Compilation

```bash
make unix
```
or
```bat
make win
```
if you somehow got hands on a windows makefile.

Alternativly, you can compile it manually via:
```bash
g++ src/main.cpp -o sortetdir -std=c++17 -lm -Iinclude
```



## Usage

```bash
sortetdir.exe
```
Example output:
```bash
Name                          Type            Size
--------------------------------------------------
main.exe                      FILE         3.06 MB
include                       DIR        963.14 KB
.git                          DIR        806.81 KB
src                           DIR         22.75 KB
.vscode                       DIR          2.31 KB
README.md                     FILE         1.03 KB
Makefile                      FILE            91 B
.gitignore                    FILE            21 B
config.json                   FILE            10 B

Size of current directiory: 4.82 MB
> // <- Command Line 
```
Then, use the _help_ command to see which other commands this program has to offer!
```bash
(sortetdir)$> help
Commands:
cd      -> Change Directory to arg1.
help    -> prints this.
q       -> quits the programm.
table   -> prints the standart sorted table
tree    -> prints a file tree of the current dir
what    -> <to be implemented...>
cls     -> clears the screen
pwd     -> prints working directory

(sortetdircpp)$> cd include
(sortetdircpp\include)$> cd nlohmann
``` 
Then we call "tree 3" to print a tree of the current directory,
while keeping a max-depth of 3. You can see this at "call_std",
which would normally contain a lot of annoying files.
```bash

(sortetdircpp\include\nlohmann)$> tree 3
nlohmann: 963.14 KB
├> adl_serializer.hpp: 2.28 KB
├> byte_container_with_subtype.hpp: 3.55 KB
├> detail: 646.30 KB
│      ├> abi_macros.hpp: 4.13 KB
├──────┼> conversions: 78.30 KB
│      │      ├> from_json.hpp: 21.82 KB
│      │      ├> to_chars.hpp: 38.78 KB
│      │      ├> to_json.hpp: 17.70 KB
│      ├> exceptions.hpp: 10.62 KB
│      ├> hash.hpp: 4.05 KB
├──────┼> input: 234.99 KB
│      │      ├> binary_reader.hpp: 106.25 KB
│      │      ├> input_adapters.hpp: 19.79 KB
│      │      ├> json_sax.hpp: 31.38 KB
│      │      ├> lexer.hpp: 55.85 KB
│      │      ├> parser.hpp: 20.74 KB
│      │      ├> position_t.hpp: 997 B
├──────┼> iterators: 43.18 KB
│      │      ├> internal_iterator.hpp: 1.08 KB
│      │      ├> iteration_proxy.hpp: 8.11 KB
│      │      ├> iterator_traits.hpp: 1.77 KB
│      │      ├> iter_impl.hpp: 25.03 KB
│      │      ├> json_reverse_iterator.hpp: 3.90 KB
│      │      ├> primitive_iterator.hpp: 3.28 KB
│      ├> json_custom_base_class.hpp: 1.19 KB
│      ├> json_pointer.hpp: 37.33 KB
│      ├> json_ref.hpp: 1.85 KB
│      ├> macro_scope.hpp: 51.51 KB
│      ├> macro_unscope.hpp: 1.32 KB
├──────┼> meta: 47.41 KB
│      ├──────┼> call_std: 942 B
│      │      │       ...
│      │      ├> cpp_future.hpp: 5.22 KB
│      │      ├> detected.hpp: 2.13 KB
│      │      ├> identity_tag.hpp: 549 B
│      │      ├> is_sax.hpp: 6.95 KB
│      │      ├> std_fs.hpp: 820 B
│      │      ├> type_traits.hpp: 30.24 KB
│      │      ├> void_t.hpp: 623 B
├──────┼> output: 117.07 KB
│      │      ├> binary_writer.hpp: 72.70 KB
│      │      ├> output_adapters.hpp: 4.12 KB
│      │      ├> serializer.hpp: 40.25 KB
│      ├> string_concat.hpp: 5.96 KB
│      ├> string_escape.hpp: 2.20 KB
│      ├> string_utils.hpp: 877 B
│      ├> value_t.hpp: 4.34 KB
├> json.hpp: 205.10 KB
├> json_fwd.hpp: 2.56 KB
├> ordered_map.hpp: 11.77 KB
├> thirdparty: 91.58 KB
├──────┼> hedley: 91.58 KB
│      │      ├> hedley.hpp: 86.05 KB
│      │      ├> hedley_undef.hpp: 5.53 KB
Size of current directiory: 963.14 KB
```
```bash
(sortetdircpp\include\nlohmann)$> cd detail
(sortetdircpp\include\nlohmann\detail)$> table
```
```bash
Name                          Type            Size
__________________________________________________
input                         DIR        234.99 KB
output                        DIR        117.07 KB
conversions                   DIR         78.30 KB
macro_scope.hpp               FILE        51.51 KB
meta                          DIR         47.41 KB
iterators                     DIR         43.18 KB
json_pointer.hpp              FILE        37.33 KB
exceptions.hpp                FILE        10.62 KB
string_concat.hpp             FILE         5.96 KB
value_t.hpp                   FILE         4.34 KB
abi_macros.hpp                FILE         4.13 KB
hash.hpp                      FILE         4.05 KB
string_escape.hpp             FILE         2.20 KB
json_ref.hpp                  FILE         1.85 KB
macro_unscope.hpp             FILE         1.32 KB
json_custom_base_class.hpp    FILE         1.19 KB
string_utils.hpp              FILE           877 B

Size of current directiory: 646.30 KB
(sortetdircpp\include\nlohmann\detail)$>
```
Almost every string of this is customizable in the config.json file.