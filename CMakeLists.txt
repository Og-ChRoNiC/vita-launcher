cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{DOLCESDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{DOLCESDK}/share/dolce.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define DOLCESDK to point to your SDK path!")
  endif()
endif()

project(launcher)
include("${DOLCESDK}/share/dolce.cmake" REQUIRED)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
  COMMAND dolce-make-bootparam app_memsize 0x8000 attribute 0x08 ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin
)

set(VITA_APP_NAME "Launcher")
set(VITA_TITLEID  "SMLA00001")
set(VITA_VERSION  "01.90")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-psabi -Wunused-variable -Wwrite-strings -fpermissive -std=c++17 -Wimplicit-fallthrough")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive")

add_definitions(-DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0)

include_directories(
  src
  sqlite-3.6.23.1
)

add_executable(${PROJECT_NAME}
  src/launcher.cpp
  src/gui.cpp
  src/textures.cpp
  src/fs.cpp
  src/sfo.cpp
  src/game.cpp
  src/main.cpp
  src/inifile.c
  src/vita_sqlite.c
  src/db.cpp
  src/config.cpp
  src/eboot.cpp
  src/iso.cpp
  src/cso.cpp
  src/style.cpp
  src/ime_dialog.cpp
  sqlite-3.6.23.1/sqlite3.c
)

target_link_libraries(${PROJECT_NAME}
  vita2d_sys
  png
  imgui_vita2d
  stdc++
  c
  z
  SceCommonDialog_stub
  SceLibKernel_stub
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceTouch_stub
  m
  SceAppMgr_stub
  SceAppUtil_stub
  SceAppMgrUser_stub
  SceSharedFb_stub
  SceDbg_stub
  SceGxmInternalForVsh_stub
  SceDmacmgr_stub
	SceFios2_stub
  SceDisplayUser_stub
)

set(DOLCE_MKSFOEX_FLAGS "${DOLCE_MKSFOEX_FLAGS} -d ATTRIBUTE=17338760 -s CATEGORY=gdc")
set(DOLCE_ELF_CREATE_FLAGS "${DOLCE_ELF_CREATE_FLAGS} -h 544288")

dolce_create_self(eboot.bin ${PROJECT_NAME} UNSAFE BOOT_PARAM ${CMAKE_CURRENT_BINARY_DIR}/boot_param.bin)

dolce_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  CONTENT_ID_LABEL "VITLAUNCHR000000"
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE noicon.png noicon.png
  FILE favorite.png favorite.png
  FILE square.png square.png
  FILE triangle.png triangle.png
  FILE circle.png circle.png
  FILE cross.png cross.png
  FILE default_style.ini default_style.ini
)
