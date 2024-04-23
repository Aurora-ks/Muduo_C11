set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(. MYSQL_LIB)
add_library(mysql SHARED ${MYSQL_LIB})
target_link_libraries(mysql PRIVATE mysqlcppconn)