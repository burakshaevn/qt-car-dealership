# SQL layer: every SQL statement of the application lives in sql/ as a
# standalone .sql file and is compiled into the binary as a Qt resource.
# C++ code refers to queries only by name (see include/SqlQueries.h).

file(GLOB_RECURSE SYSTEM_DATA_SQL_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/sql
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/sql/*.sql
)

add_library(system_data STATIC
    include/SqlQueries.h
    include/SqlQueryStore.h
    include/SqlScript.h
    src/SqlQueryStore.cpp
    src/SqlScript.cpp
)

target_include_directories(system_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(system_data PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
)

# Resource paths: :/sql/queries/<group>/<name>.sql, :/sql/migrations/NNN_*.sql,
# :/sql/bootstrap/<name>.sql
set(SYSTEM_DATA_SQL_RESOURCES "")
foreach(sql_file IN LISTS SYSTEM_DATA_SQL_FILES)
    set(source ${CMAKE_CURRENT_SOURCE_DIR}/sql/${sql_file})
    set_source_files_properties(${source} PROPERTIES QT_RESOURCE_ALIAS ${sql_file})
    list(APPEND SYSTEM_DATA_SQL_RESOURCES ${source})
endforeach()

qt_add_resources(system_data "system_data_sql"
    PREFIX "/sql"
    FILES ${SYSTEM_DATA_SQL_RESOURCES}
)
