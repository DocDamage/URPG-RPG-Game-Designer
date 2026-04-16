# CMake generated Testfile for 
# Source directory: C:/Users/dferr/dev/URPG - RPG Game Maker
# Build directory: C:/Users/dferr/dev/URPG - RPG Game Maker/build-local
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_tests-b12d07c_include.cmake")
include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_integration_tests-b12d07c_include.cmake")
include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_snapshot_tests-b12d07c_include.cmake")
include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_include.cmake")
subdirs("_deps/catch2-build")
subdirs("_deps/nlohmann_json-build")
subdirs("_deps/sdl2-build")
