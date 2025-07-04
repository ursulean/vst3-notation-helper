cmake_minimum_required(VERSION 3.14.0)

option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

project(NotationChordHelper
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.2.0
    DESCRIPTION "NotationChordHelper VST 3 Plug-in"
)

add_subdirectory(external)

smtg_enable_vst3_sdk()

smtg_add_vst3plugin(NotationChordHelper
    source/version.h
    source/cids.h
    source/key_signature.h
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/notation_view.h
    source/notation_view.cpp
    source/notation_editor.h
    source/notation_editor.cpp
    source/entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_sources(NotationChordHelper
        PRIVATE
        resource/editor.uidesc
    )
    target_link_libraries(NotationChordHelper
        PRIVATE
        vstgui_support
    )
    smtg_target_add_plugin_resources(NotationChordHelper
        RESOURCES
        "resource/editor.uidesc"
    )
endif(SMTG_ENABLE_VSTGUI_SUPPORT)
# -------------------

smtg_target_add_plugin_snapshots(NotationChordHelper
    RESOURCES
    resource/DEA1730E1F515AF1B8D0AA160EA0F195_snapshot.png
    resource/DEA1730E1F515AF1B8D0AA160EA0F195_snapshot_2.0x.png
)

target_link_libraries(NotationChordHelper
    PRIVATE
    sdk
)

smtg_target_configure_version_file(NotationChordHelper)

if(SMTG_MAC)
    smtg_target_set_bundle(NotationChordHelper
        BUNDLE_IDENTIFIER com.ursulean.nchelper
        COMPANY_NAME "Paul Ursulean"
    )
    smtg_target_set_debug_executable(NotationChordHelper
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(NotationChordHelper PRIVATE
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT NotationChordHelper)

        smtg_target_set_debug_executable(NotationChordHelper
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
