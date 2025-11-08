cmake_minimum_required(VERSION 3.16)
project(alda_medical)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/implot)

set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(IMPLOT_SOURCES
        ${IMPLOT_DIR}/implot.cpp
        ${IMPLOT_DIR}/implot_items.cpp
)

set(MODELS
        ${CMAKE_CURRENT_SOURCE_DIR}/models/channel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/models/channels_group.h
        ${CMAKE_CURRENT_SOURCE_DIR}/models/patient.h
)

set(SERVICES
        ${CMAKE_CURRENT_SOURCE_DIR}/services/secure_storage_service.h
        ${CMAKE_CURRENT_SOURCE_DIR}/services/channel_management_service.h
        ${CMAKE_CURRENT_SOURCE_DIR}/services/channel_management_service.cpp
)

set(VIEWS
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_model.h
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_view.h
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_view.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_presenter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/views/channels_selector_modal/channels_group_presenter.cpp
)

include_directories(
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${IMPLOT_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(alda_medical
        main.cpp
        eeg_core.h
        eeg_core.cpp
        eeg_theme.h
        eeg_toolbar.h
        eeg_toolbar.cpp
        eeg_chart.h
        eeg_chart.cpp
        ${MODELS}
        ${SERVICES}
        ${VIEWS}
        ${IMGUI_SOURCES}
        ${IMPLOT_SOURCES}
)

target_link_libraries(alda_medical
        OpenGL::GL
        glfw
)

if(APPLE)
    target_link_libraries(alda_medical
            "-framework Cocoa" "-framework IOKit" "-framework CoreVideo"
    )
endif()

if(MSVC)
    target_compile_options(alda_medical PRIVATE /W4)
else()
    target_compile_options(alda_medical PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS alda_medical RUNTIME DESTINATION bin)