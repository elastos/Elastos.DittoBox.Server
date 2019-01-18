if(__carrier_packages_included)
    return()
endif()
set(__carrier_packages_included)

## Package Outputs Distribution
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_GENERATOR "DEB")

set(CPACK_DEBIAN_PACKAGE_NAME "elaoc-agentd")
set(CPACK_DEBIAN_PACKAGE_SOURCE "elastos")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_VENDOR "elastos.org")
set(CPACK_PACKAGE_CONTACT "libin@elastos.org")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.elastos.org/")

set(CPACK_PACKAGE_DESCRIPTION "Elastos Carrier Agent for ownCloud Distribution Packages")
set(CPACK_PACKAGE_VERSION_MAJOR ${CARRIER_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CARRIER_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CARRIER_VERSION_PATCH})

set(CPACK_DEBIAN_PACKAGE_PREDEPENDS "adduser")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
    "lsb-base (>= 3.0), init-system-helpers (>= 1.18~),
    libc6 (>= 2.14), libsystemd0")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "Elastos Owncloud Agent (daemon)
    Elastos Owncloud Agent is a service forwarding agent for owncloud.
    .
    This package contains the elastos owncloud agent.")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/debian/postinst;"
    "${CMAKE_SOURCE_DIR}/debian/postrm;"
    "${CMAKE_SOURCE_DIR}/debian/preinst;"
    "${CMAKE_SOURCE_DIR}/debian/prerm")

string(CONCAT CPACK_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-"
    "${CPACK_PACKAGE_VERSION_MAJOR}."
    "${CPACK_PACKAGE_VERSION_MINOR}."
    "${CPACK_PACKAGE_VERSION_PATCH}")

## Package Source distribution.
set(CPACK_SOURCE_GENERATOR "TGZ")

string(CONCAT CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-"
    "${CPACK_PACKAGE_VERSION_MAJOR}."
    "${CPACK_PACKAGE_VERSION_MINOR}."
    "${CPACK_PACKAGE_VERSION_PATCH}")

string(CONCAT CPACK_SOURCE_IGNORE_FILES
    "/build;"
    "/.git;"
    "${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)
