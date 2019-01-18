include(AddSubModule)

add_submodule(libsodium)
add_submodule(libconfig)
add_submodule(libcrystal
    DEPENDS libsodium)
