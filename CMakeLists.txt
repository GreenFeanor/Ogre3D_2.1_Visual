project(OgreUpdate)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Debug)
add_definitions(-DNDEBUG)
#add_definitions(-D_DEBUG)

set(CMAKE_MODULE_PATH "/usr/lib/OGRE/cmake/")

# Il s'agit du tutoriel d'exemple, qui utilise quelques fichiers prédéfinis de Ogre. Il faut indiquer à cmake où se trouvent les includes en question
include_directories ("/usr/include/OGRE/")

# Répertoire des includes d'Ogre
find_package(OGRE REQUIRED)
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIRS})
# Link QT5widget
FIND_PACKAGE(Qt5Widgets)
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# link QWindow
find_package(Qt5Gui)
INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Gui_DEFINITIONS})

# Link des librairies
LINK_DIRECTORIES(${BoostLIBRARY_DIRS})

#Fichiers c(pp)/h(pp) créés !!
SET(SOURCES
    sources/QTOgreWindow.hpp
    sources/QTOgreWindow.cpp
    sources/main.cpp
    sources/SdkQtCameraMan.hpp)


#Executable de la compilation
add_executable (OgreUpdateexe ${SOURCES})

target_link_libraries(OgreUpdateexe OgreMain OgreHlmsPbs OgreHlmsUnlit pthread ${Boost_SYSTEM_LIBRARY} ${GL_LIBRARY} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES})

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/dist/bin)
