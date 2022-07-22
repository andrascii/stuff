SET BUILD_DIRECTORY=%1

if [%1] == [] goto :no_build_dir

conan install . --install-folder %BUILD_DIRECTORY% --build=missing
goto :eof

:no_build_dir
echo "Expected argument determining the build directory to install conan libraries"