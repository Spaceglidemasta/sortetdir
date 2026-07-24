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

You can use `sortetdir.exe -h` to print a list of all flags and options.

### Table

```bash
sortetdir.exe
```
or
```bash
sortetdir.exe -t
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

    sortetdir.exe include -b 2

output:
```bash
[include]
╰─ nlohmann: 963.14 KB
        ├─ adl_serializer.hpp: 2.28 KB
        ├─ byte_container_with_subtype.hpp: 3.55 KB
        ├─ detail: 646.30 KB
        │        ...
        ├─ json.hpp: 205.10 KB
        ├─ json_fwd.hpp: 2.56 KB
        ├─ ordered_map.hpp: 11.77 KB
        ╰─ thirdparty: 91.58 KB
                 ...

Size of current directiory: 963.14 KB
```

### CLI

    sortetdir.exe -c

This will open a switch to an active CLI version, where you can use these commands:

- tree \<depth>
- table
- cd \<folder> (change directory to target)
- info \<file | folder>
- help
- q (quits the program)
- pwd (print current directory)
- cls (clear screen)


#### Example of the "info" command

```bash
(sortetdircpp\include\nlohmann)$> info detail

Size of detail: 646.30 KB
Creation time of detail: Wed Oct  8 11:26:43 2025
Contains: 39 Files, 6 Folders
```

_Please note that the "Creationtime" function is not yet available for linux._


__Almost every string of this is customizable in the config.json file.__
