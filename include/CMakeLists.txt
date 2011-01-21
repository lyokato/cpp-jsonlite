INCLUDE_DIRECTORIES(${jsonlite_SOURCE_DIR}/include)

#FILE(GLOB src_files ${jsonlite_SOURCE_DIR}/src/*)
#ADD_LIBRARY(jsonlite SHARED ${src_files})
#ADD_LIBRARY(jsonlite_static STATIC ${src_files})
# SET_TARGET_PROPERTIES(jsonlite_static PROPERTIES OUTPUT_NAME jsonlite)

FILE(GLOB header_files ${jsonlite_SOURCE_DIR}/include/*)
INSTALL(FILES ${header_files} DESTINATION include)
#INSTALL(TARGETS jsonlite DESTINATION lib)
