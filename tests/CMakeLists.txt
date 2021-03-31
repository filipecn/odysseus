set(SOURCES
        main.cpp
        memory_tests.cpp
        )

add_executable(odysseus_tests ${SOURCES})
target_include_directories(odysseus_tests PUBLIC
        ${CATCH2_INCLUDES}
        ${PONOS_INCLUDES}
        "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_options(odysseus_tests INTERFACE --coverage)

if (UNIX AND NOT APPLE)
    target_link_libraries(odysseus_tests
            odysseus
            ${PONOS_LIBRARIES}
            gcov
            )

    add_custom_target(test_odysseus
            COMMAND lcov --directory . --zerocounters
            COMMAND odysseus_tests DEPENDS odysseus_tests
            COMMAND lcov --directory . --capture -output-file cov.info
            COMMAND lcov --remove cov.info '/usr/*' --output-file cov.info
            COMMAND lcov --remove cov.info '*/ext/*' --output-file cov.info
            COMMAND lcov --remove cov.info '*/tests/*' --output-file cov.info
            COMMAND lcov --remove cov.info '*/ponos/*' --output-file cov.info
            COMMAND genhtml cov.info --output-directory out
            )
else (UNIX AND NOT APPLE)

    target_link_libraries(odysseus_tests ponos)

endif (UNIX AND NOT APPLE)