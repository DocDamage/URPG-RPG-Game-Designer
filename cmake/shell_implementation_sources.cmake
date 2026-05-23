set(URPG_SHELL_IMPLEMENTATION_CORE_SOURCES
    engine/core/map/grid_part_catalog_loader.cpp
    editor/project/main_menu_panel.cpp
)

set(URPG_SHELL_IMPLEMENTATION_TEST_SOURCES
    tests/unit/test_main_menu_panel.cpp
)

if(TARGET urpg_runtime_core_objects)
    target_sources(urpg_runtime_core_objects PRIVATE
        engine/core/map/grid_part_catalog_loader.cpp
    )
endif()

if(TARGET urpg_editor_panels_objects)
    target_sources(urpg_editor_panels_objects PRIVATE
        editor/project/main_menu_panel.cpp
    )
endif()

if(TARGET urpg_tests)
    target_sources(urpg_tests PRIVATE
        tests/unit/test_main_menu_panel.cpp
    )
endif()
