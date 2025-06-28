# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-src"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-build"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/tmp"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/src"
  "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/willow/Documents/GitHub/SpaceGame/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
