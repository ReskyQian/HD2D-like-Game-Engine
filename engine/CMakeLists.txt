set(ENGINE_ROOT_DIR "${HD2D_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rd_party")
set(ENGINE_ASSET_DIR "/asset")
set(SHADER_DIR "${ENGINE_ROOT_DIR}/shader")

set(DEPLOY_CONFIG_DIR "configs/deployment")
set(DEVELOP_CONFIG_DIR "configs/development")

add_subdirectory(3rd_party)
add_subdirectory(source/editor)