# sortetdir

A terminal-based C++ application that shows how much space each directory takes up — perfect for digital spring-cleaning!

## Features
- Displays directory sizes in a clean, readable format  
- Helps identify large folders quickly  
- Lightweight and fast  



## Installation
```bash
git clone https://github.com/Spaceglidemasta/sortetdir
```

I also recommend adding the `bin` directory to PATH and changing the .exe
name to something that suits your usage.

## Compilation

#### Linux
```bash
make unix
```

#### Windows
```bat
make win
```


## Usage
### Table

```bash
sortetdir.exe
```
or 

```bash
sortetdir.exe table
```

output:
```bash

Name                          Type            Size
__________________________________________________
sortetdir.exe                 FILE         3.15 MB
.git                          DIR          1.60 MB
sortetdir                     FILE         1.30 MB
include                       DIR        963.14 KB
src                           DIR         36.20 KB
README.md                     FILE         6.25 KB
.vscode                       DIR          2.31 KB
config.json                   FILE         1.27 KB
Makefile                      FILE           187 B
.gitignore                    FILE            66 B
updates.txt                   FILE             3 B

Size of current directiory: 7.04 MB
```

### Treeview

    sortetdir.exe tree

output:
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
The command "detail" can then be used to find out usefull information about a desired file / directory.


### CLI

    sortetdir.exe cmd

This will open a switch to an active CLI version, where you can use these commands:

- tree \<depth>
- table
- cd \<folder>
- info \<file | folder>
- help
- q (quits the program)
- pwd
- cls


#### Example of the "info" command

```bash
(sortetdircpp\include\nlohmann)$> info detail

Size of detail: 646.30 KB
Creation time of detail: Wed Oct  8 11:26:43 2025
Contains: 39 Files, 6 Folders
```

_Please note that the "Creationtime" function is not yet available for linux._


__Almost every string of this is customizable in the config.json file.__
