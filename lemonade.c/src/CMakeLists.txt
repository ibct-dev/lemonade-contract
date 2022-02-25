project(lemonade.c)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( lemonade.c lemonade.c lemonade.c.cpp token.cpp )
target_include_directories( lemonade.c PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( lemonade.c ${CMAKE_SOURCE_DIR}/../ricardian )