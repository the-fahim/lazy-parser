# tests/CMakeLists.txt — LazyParser test suite

# ── test_parser ───────────────────────────────────────────────────────────────
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE lazyparser_lib)
add_test(NAME test_parser COMMAND test_parser)

# ── test_grammar ──────────────────────────────────────────────────────────────
add_executable(test_grammar test_grammar.cpp)
target_link_libraries(test_grammar PRIVATE lazyparser_lib)
add_test(NAME test_grammar COMMAND test_grammar)

# ── test_utils ────────────────────────────────────────────────────────────────
add_executable(test_utils test_utils.cpp)
target_link_libraries(test_utils PRIVATE lazyparser_lib)
add_test(NAME test_utils COMMAND test_utils)