# CPackConfig for Linux packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_PACKAGE_NAME "projecteur")
set(CPACK_STRIP_FILES ON)

set(CPACK_PACKAGE_CONTACT "Jahn Fuchs <github.jahnf@wolke7.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux/X11 application for the Logitech Spotlight device.")
set(CPACK_PACKAGE_DESCRIPTION "Linux/X11 application for the Logitech Spotlight device.
Homepage: https://github.com/jahnf/Projecteur")

set(CPACK_PACKAGE_VERSION 0.6-alpha.28)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 6)
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CPACK_INSTALL_CMAKE_PROJECTS "/media/data/development/internet/Projecteur/build;Projecteur;ALL;.")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_debian-unknown-x86_64")

set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_COMPRESSION_TYPE gzip)
set(CPACK_RPM_PACKAGE_VERSION "0.6.0")
set(CPACK_RPM_PACKAGE_RELEASE "alpha.28")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Linux/X11 application for the Logitech Spotlight device.
Homepage: https://github.com/jahnf/Projecteur")
set(CPACK_RPM_PACKAGE_AUTOPROV 1)
set(CPACK_RPM_PACKAGE_AUTOREQ 1)

# Other settings
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/jahnf/Projecteur")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# Set requires/depends
set(CPACK_RPM_PACKAGE_REQUIRES "qml-module-qtgraphicaleffects (>= 5.7), libqt5widgets5 (>= 5.7), passwd, udev")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qml-module-qtgraphicaleffects (>= 5.7), libqt5widgets5 (>= 5.7), passwd, udev") 

# Post and Pre-install actions if necessary
set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "/media/data/development/internet/Projecteur/build/pkg/scripts/preinst")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "/media/data/development/internet/Projecteur/build/pkg/scripts/postinst")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "")
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "")

foreach(script CPACK_RPM_PRE_INSTALL_SCRIPT_FILE
               CPACK_RPM_POST_INSTALL_SCRIPT_FILE
               CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
               CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE)
  if(NOT "${${script}}" STREQUAL "")
    list(APPEND PKG_DEBIAN_CTRL_EXTRA "${${script}}")
  endif()
endforeach()

list(LENGTH PKG_DEBIAN_CTRL_EXTRA NUM_PKG_CTRL_SCRIPTS)
set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
if(NUM_PKG_CTRL_SCRIPTS)
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PKG_DEBIAN_CTRL_EXTRA}")
endif()

