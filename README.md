> "You can be crueler than that, Jack!"
- Sundowner, Metal Gear Rising (2013)

## SUPPORTED TITLES
|Title|Model Viewing Capability|General File Editing|Console Details|
|-----|------------------------|--------------------|---------------|
|Metal Gear Rising|Yes (WMB2/WMB3)| Most files supported|No big-endian DDS (for now)|
|Bayonetta 1|Yes (WMB0)| Most file supported|No ASTC or GX2 textures|
|Bayonetta 2|Meshes, no textures (WMB0)|Most files supported|No ASTC or GX2 textures|
|Bayonetta 3|Hardly (WMB3)|Few files supported|N/A|
|Nier|Hardly (WMB3)|Few files supported|N/A|
|Astral Chain|Hardly (WMB3)|Few files supported|N/A|

Basic files such as DAT, BXM, EFF, etc are openable by default on all titles, in both regular and big-endian variants.

## Features
- Full WMB/SCR viewer with textures
- LY2 Editor
- DAT Editor
- BNK WEM Ripping
- BXM Editing
- BNK Viewer
- CPK Reading
  
## Planned Features
- Animation Viewer
- EST Editing
- MCD Editing
- Game-accurate shading
- Game-link

## Building From Source
### With Visual Studio
#### Requirements
- [DirectX Legacy SDK (2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812): The DXSDK_DIR environment variable will be created by the installer automatically, but if something goes wrong with that part, compilation will fail and you'll have to set it manually.
- [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk): The FBXSDK_DIR environment variable should be set manually. Set it to the path containing the include and lib folders.
- [Perl](https://learn.perl.org/installing/windows.html): The [Chocolatey package](https://community.chocolatey.org/packages/StrawberryPerl) is recommended due to ease of installation.
- [Visual Studio 2022](https://visualstudio.microsoft.com/) with the C++ toolset (Older versions may or may not be supported at a later date)
- [CMake](https://cmake.org/): The [Chocolatey package](https://community.chocolatey.org/packages/cmake) is recommmended due to ease of installation.

#### Instructions
1. Clone the repository recursively. If using git from a terminal, run `git clone --recursive https://github.com/ChloeZamorano/CruelerThanDAT`. Cloning recursively will ensure the cURL submodule won't be missing.
2. Run the command `premake5 vs2022` in the root folder of the repository. This will generate project files for Visual Studio 2022, including the cURL dependency and CruelerThanDAT itself.
3. Upon opening the generated Visual Studio solution file, you may open it normally like any other and compile a debug or release build by pressing F7.

### With GNU Make
#### Requirements
- [DirectX Legacy SDK (2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812): The DXSDK_DIR environment variable will be created by the installer automatically, but if something goes wrong with that part, compilation will fail and you'll have to set it manually.
- [FBX SDK](https://aps.autodesk.com/developer/overview/fbx-sdk): The FBXSDK_DIR environment variable should be set manually. Set it to the path containing the include and lib folders.
- [Perl](https://learn.perl.org/installing/windows.html): The [Chocolatey package](https://community.chocolatey.org/packages/StrawberryPerl) is recommended due to ease of installation.
- [CMake](https://cmake.org/): The [Chocolatey package](https://community.chocolatey.org/packages/cmake) is recommmended due to ease of installation.
- [GNU Make](https://www.gnu.org/software/make/): The [Chocolatey package](https://community.chocolatey.org/packages/make) is recommmended due to ease of installation.
- [LLVM](https://llvm.org/): For the Clang compiler specifically. The [Chocolatey package](https://community.chocolatey.org/packages/llvm) is recommmended due to ease of installation.

#### Instructions
1. Clone the repository recursively. If using git from a terminal, run `git clone --recursive https://github.com/ChloeZamorano/CruelerThanDAT`. Cloning recursively will ensure the cURL submodule won't be missing.
2. Run the command `premake5 gmake` in the root folder of the repository. This will generate project files for GNU Make, including the cURL dependency and CruelerThanDAT itself.
3. Compile with GNU Make:
    - To compile cURL, run `make config=debug cURL`, where `debug` can be replaced with `release`, and the `-j` option may be specified for faster build times; for example `-j4` to use 4 threads, or any desired amount.
    - To compile CruelerThanDAT, run `make config=debug CruelerThanDAT`. Note this will fail if you don't compile cURL first.
    - To compile everything, run `make config=debug all`. This will compile cURL and then CruelerThanDAT in that order the same as the above commands.
