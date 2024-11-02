# How to build
You need: 
    - Visual Studio 2022(or any other IDE, but code is designed to work with VC++ compiler)
    - Static libraries and dynamic link libraries, included with repository in lib and bin folders
    - OpenSSL headers, which can be downloaded here: https://github.com/CristiFati/Prebuilt-Binaries/tree/master/OpenSSL

Tutorial:
    -First, create Visual studio solution and configure it for Win32 target platform.
    -Second, add static libraries to project as described in client/lib/about.md
    -Third, add openssl/include folder as additional include directory by going into
    project properties -> c++ -> general -> additional include directories and add folder
    there
    -Fourth, copy DLL-s from server/bin folder to debug folder
    -Now you can add source files from repository to your project.
    -Now, you can build project.
    
Enjoy!