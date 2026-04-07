cmake_minimum_required(VERSION 3.16)
project(document_ingestion LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# nlohmann/json
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3)
FetchContent_MakeAvailable(nlohmann_json)

# SQLite3 - try vcpkg/system first, else FetchContent
find_package(unofficial-sqlite3 CONFIG QUIET)
if(unofficial-sqlite3_FOUND)
  set(SQLite3_LIB unofficial-sqlite3::sqlite3)
else()
  find_package(SQLite3 QUIET)
  if(SQLite3_FOUND)
    set(SQLite3_LIB SQLite::SQLite3)
  else()
    FetchContent_Declare(sqlite_amalgamation
      URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_Populate(sqlite_amalgamation)
    file(GLOB SQLITE_SOURCES "${sqlite_amalgamation_SOURCE_DIR}/*/sqlite3.c"
                            "${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c")
    list(GET SQLITE_SOURCES 0 SQLITE_C_FILE)
    get_filename_component(SQLITE_DIR ${SQLITE_C_FILE} DIRECTORY)
    add_library(sqlite3_embedded STATIC ${SQLITE_C_FILE})
    target_include_directories(sqlite3_embedded PUBLIC ${SQLITE_DIR})
    set(SQLite3_LIB sqlite3_embedded)
  endif()
endif()

# pugixml
FetchContent_Declare(pugixml
  GIT_REPOSITORY https://github.com/zeux/pugixml.git
  GIT_TAG v1.14)
FetchContent_MakeAvailable(pugixml)

# cpp-httplib (header only)
FetchContent_Declare(cpp-httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.14.3)
FetchContent_Populate(cpp-httplib)

# Optional Poppler for PDF
option(ENABLE_PDF "Enable PDF via Poppler" OFF)
if(ENABLE_PDF)
  find_package(Poppler QUIET)
endif()

# miniz for DOCX/XLSX (ZIP)
set(MINIZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniz)
if(EXISTS ${MINIZ_DIR}/miniz.c)
  add_library(miniz STATIC
    ${MINIZ_DIR}/miniz.c
    ${MINIZ_DIR}/miniz_zip.c
    ${MINIZ_DIR}/miniz_tinfl.c
    ${MINIZ_DIR}/miniz_tdef.c
  )
  target_include_directories(miniz PUBLIC ${MINIZ_DIR})
  if(MSVC)
    target_compile_definitions(miniz PRIVATE _CRT_SECURE_NO_WARNINGS)
  endif()
else()
  message(FATAL_ERROR "miniz not found at ${MINIZ_DIR}")
endif()

set(DOC_INGESTION_CORE_SOURCES
  src/config.cpp
  src/store/avl_tree.cpp
  src/store/store.cpp
  src/store/models.cpp
  src/store/search_history.cpp
  src/scanner/scanner.cpp
  src/scanner/registry.cpp
  src/parsers/factory.cpp
  src/parsers/txt_parser.cpp
  src/parsers/pdf_parser.cpp
  src/parsers/docx_parser.cpp
  src/parsers/xlsx_parser.cpp
  src/rules/models.cpp
  src/rules/engine.cpp
  src/validation/normalizer.cpp
  src/validation/validator.cpp
  src/db/database.cpp
  src/db/rules_repo.cpp
  src/db/logs_repo.cpp
  src/services/ingestion_service.cpp
  src/services/report_service.cpp
  src/services/keyword_search_service.cpp
  src/logging/logger.cpp
)

add_library(doc_ingestion_core STATIC ${DOC_INGESTION_CORE_SOURCES})
target_include_directories(doc_ingestion_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
  ${cpp-httplib_SOURCE_DIR}
)
target_link_libraries(doc_ingestion_core PUBLIC
  nlohmann_json::nlohmann_json
  ${SQLite3_LIB}
  pugixml::pugixml
  miniz
)

if(ENABLE_PDF AND Poppler_FOUND)
  target_compile_definitions(doc_ingestion_core PUBLIC ENABLE_POPPLER=1)
  target_link_libraries(doc_ingestion_core PUBLIC Poppler::poppler)
endif()

add_executable(document_ingestion src/main.cpp)
target_link_libraries(document_ingestion PRIVATE doc_ingestion_core)

if(WIN32)
  option(BUILD_GUI "Build Win32 desktop GUI (document_ingestion_gui.exe)" ON)
  if(BUILD_GUI)
    add_executable(document_ingestion_gui WIN32 src/gui/win32_app.cpp src/gui/app.manifest)
    target_link_libraries(document_ingestion_gui PRIVATE doc_ingestion_core comctl32)
    if(MSVC)
      target_compile_options(document_ingestion_gui PRIVATE /utf-8)
    endif()
  endif()
endif()

# Google Test
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_avl_tree tests/test_avl_tree.cpp src/store/avl_tree.cpp)
target_include_directories(test_avl_tree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_avl_tree PRIVATE GTest::gtest_main)
add_test(NAME AVLTreeTests COMMAND test_avl_tree)

add_executable(test_rule_engine tests/test_rule_engine.cpp
  src/rules/engine.cpp src/rules/models.cpp src/validation/normalizer.cpp)
target_include_directories(test_rule_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_rule_engine PRIVATE GTest::gtest_main)
add_test(NAME RuleEngineTests COMMAND test_rule_engine)

add_executable(test_integration tests/test_integration.cpp
  src/store/avl_tree.cpp src/store/store.cpp src/store/models.cpp)
target_include_directories(test_integration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_integration PRIVATE GTest::gtest_main nlohmann_json::nlohmann_json)
add_test(NAME IntegrationTests COMMAND test_integration)

add_executable(test_pipeline tests/test_pipeline.cpp
  src/config.cpp
  src/store/avl_tree.cpp src/store/store.cpp src/store/models.cpp
  src/scanner/scanner.cpp src/scanner/registry.cpp
  src/parsers/factory.cpp src/parsers/txt_parser.cpp src/parsers/pdf_parser.cpp
  src/parsers/docx_parser.cpp src/parsers/xlsx_parser.cpp
  src/rules/models.cpp src/rules/engine.cpp
  src/validation/normalizer.cpp src/validation/validator.cpp
  src/db/database.cpp src/db/rules_repo.cpp src/db/logs_repo.cpp
  src/services/ingestion_service.cpp
)
target_include_directories(test_pipeline PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(test_pipeline PRIVATE
  GTest::gtest_main
  nlohmann_json::nlohmann_json
  ${SQLite3_LIB}
  pugixml::pugixml
  miniz
)
add_test(NAME PipelineTests COMMAND test_pipeline)
