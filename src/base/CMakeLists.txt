set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(. BASE_LIB)
add_library(base STATIC ${BASE_LIB})