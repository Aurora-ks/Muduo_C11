set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(. http_LIB)
add_library(http SHARED ${http_LIB})