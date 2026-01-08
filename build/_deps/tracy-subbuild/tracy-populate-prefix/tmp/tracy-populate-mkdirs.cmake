# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-src"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-build"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/tmp"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/src/tracy-populate-stamp"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/src"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/src/tracy-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/src/tracy-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/tracy-subbuild/tracy-populate-prefix/src/tracy-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
