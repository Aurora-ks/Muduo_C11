set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(. NET_LIB)
add_library(net SHARED ${NET_LIB})