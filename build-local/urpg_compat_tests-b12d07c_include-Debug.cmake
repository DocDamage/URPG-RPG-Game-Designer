if(EXISTS "C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/Debug/urpg_compat_tests.exe")
  if(NOT EXISTS "C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_tests-Debug.cmake" OR
     NOT "C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_tests-Debug.cmake" IS_NEWER_THAN "C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/Debug/urpg_compat_tests.exe" OR
     NOT "C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_tests-Debug.cmake" IS_NEWER_THAN "${CMAKE_CURRENT_LIST_FILE}")
    include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/_deps/catch2-src/extras/CatchAddTests.cmake")
    catch_discover_tests_impl(
      TEST_EXECUTABLE [==[C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/Debug/urpg_compat_tests.exe]==]
      TEST_EXECUTOR [==[]==]
      TEST_WORKING_DIR [==[C:/Users/dferr/dev/URPG - RPG Game Maker/build-local]==]
      TEST_SPEC [==[]==]
      TEST_EXTRA_ARGS [==[]==]
      TEST_PROPERTIES [==[LABELS;weekly]==]
      TEST_PREFIX [==[]==]
      TEST_SUFFIX [==[]==]
      TEST_LIST [==[urpg_compat_tests_TESTS]==]
      TEST_REPORTER [==[]==]
      TEST_OUTPUT_DIR [==[]==]
      TEST_OUTPUT_PREFIX [==[]==]
      TEST_OUTPUT_SUFFIX [==[]==]
      CTEST_FILE [==[C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_tests-Debug.cmake]==]
      TEST_DL_PATHS [==[]==]
      CTEST_FILE [==[]==]
    )
  endif()
  include("C:/Users/dferr/dev/URPG - RPG Game Maker/build-local/urpg_compat_tests-b12d07c_tests-Debug.cmake")
else()
  add_test(urpg_compat_tests_NOT_BUILT urpg_compat_tests_NOT_BUILT)
endif()
