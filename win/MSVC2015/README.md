# Solution for MS Visual Studio 2015

This is a fairly complete solution file for usage with MSVC 2015. Either commercial or community edition will work.
It compiles against [ActiveTcl](https://www.activestate.com/activetcl/downloads), both 32 and 64 editions.

Two environment variables are necessary in order to build: 
* TCL32_HOME specifies where the 32 bit edition of ActiveTcl is installed
* TCL64_HOME specifies where the 64 bit edition of ActiveTcl is installed

Both editions can be used in parallel by switching the Platform type.

### Command line use

Open the "Developer Command Prompt for VS2015" from the start menu and cd to this directory.

    $ msbuild /t:Build /p:Configuration=(Release|Debug) /p:Platform=(Win32|x64) tclgraphs.sln

