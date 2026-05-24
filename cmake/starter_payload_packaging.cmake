# Package only the curated starter payload files that are approved in BND-009.
# Do not broaden this to content/assets/gameplay/** without a separate asset review.

install(
    DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}/content/asset_indexes"
        "${CMAKE_CURRENT_SOURCE_DIR}/content/part_catalogs"
    DESTINATION "${URPG_INSTALL_DATA_ROOT}/content"
    COMPONENT RuntimeData
    FILES_MATCHING
        PATTERN "*.json"
        PATTERN "*.md"
)

set(URPG_GAME_MAKER_STARTER_ASSET_FILES
    content/assets/gameplay/cutesckr/farm_1/tile_000.png
    content/assets/gameplay/cutesckr/farm_1/tile_001.png
    content/assets/gameplay/cutesckr/forest_wilderness_1/tile_000.png
    content/assets/gameplay/cutesckr/forest_wilderness_1/tile_001.png
    content/assets/gameplay/cutesckr/forest_wilderness_1/tile_002.png
    content/assets/gameplay/cutesckr/forest_wilderness_1/tile_003.png
    content/assets/gameplay/cutesckr/medieval_castle_1/tile_000.png
    content/assets/gameplay/cutesckr/medieval_castle_1/tile_001.png
    content/assets/gameplay/cutesckr/medieval_fantasy_town_1/tile_000.png
    content/assets/gameplay/cutesckr/medieval_fantasy_town_1/tile_001.png
    content/assets/gameplay/cutesckr/medieval_fantasy_town_1/tile_002.png
    content/assets/gameplay/cutesckr/medieval_fantasy_town_1/tile_003.png
    content/assets/gameplay/human_rpg_portraits/00000_10_human_barbarian_0_0.png
    content/assets/gameplay/human_rpg_portraits/00001_10_human_barbarian_0_1.png
)

foreach(_urpg_game_maker_starter_asset IN LISTS URPG_GAME_MAKER_STARTER_ASSET_FILES)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_urpg_game_maker_starter_asset}")
        message(FATAL_ERROR "Missing curated starter payload asset: ${_urpg_game_maker_starter_asset}")
    endif()

    get_filename_component(_urpg_game_maker_starter_asset_dir "${_urpg_game_maker_starter_asset}" DIRECTORY)
    string(REGEX REPLACE "^content/" "" _urpg_game_maker_starter_asset_content_dir
        "${_urpg_game_maker_starter_asset_dir}"
    )
    install(
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/${_urpg_game_maker_starter_asset}"
        DESTINATION "${URPG_INSTALL_DATA_ROOT}/content/${_urpg_game_maker_starter_asset_content_dir}"
        COMPONENT RuntimeData
    )
endforeach()
