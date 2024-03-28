macro (vcpkg_manifest_version OUT)
    file(READ ${CMAKE_SOURCE_DIR}/vcpkg.json VCPKG_JSON_CONTENT)
    string(JSON ${OUT} GET ${VCPKG_JSON_CONTENT} "version")
endmacro()