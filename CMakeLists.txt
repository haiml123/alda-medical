cmake_minimum_required(VERSION 3.16)
project(alda_medical)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

file(COPY ${CMAKE_SOURCE_DIR}/external/imgui/misc/fonts/
        DESTINATION ${CMAKE_BINARY_DIR}/fonts)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/models/mvp_base_model.h
        ${CMAKE_CURRENT_SOURCE_DIR}/models/base_model.h
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

        views/monitoring/monitoring_screen.h
        views/monitoring/monitoring_screen.cpp
        views/monitoring/monitoring_model.h
        views/monitoring/monitoring_model.cpp
        views/monitoring/monitoring_view.h
        views/monitoring/monitoring_view.cpp
        views/monitoring/monitoring_presenter.h
        views/monitoring/monitoring_presenter.cpp
        views/monitoring/monitoring_toolbar.h
        views/monitoring/monitoring_toolbar.cpp

        views/impedance_viewer/impedance_viewer_model.h
        views/impedance_viewer/impedance_viewer_model.cpp
        views/impedance_viewer/impedance_viewer_presenter.h
        views/impedance_viewer/impedance_viewer_presenter.cpp
        views/impedance_viewer/impedance_viewer_view.h
        views/impedance_viewer/impedance_viewer_view.cpp
        views/impedance_viewer/impedance_viewer_screen.h
        views/impedance_viewer/impedance_viewer_screen.cpp
        views/impedance_viewer/impedance_viewer_helper.h
        views/impedance_viewer/impedance_viewer_helper.cpp
        views/impedance_viewer/impedance_viewer_header.h
        views/impedance_viewer/impedance_viewer_header.cpp
)

set(UI
        UI/toolbar/toolbar.h
        UI/toolbar/toolbar.cpp
        UI/tabbar/tabbar.h
        UI/tabbar/tabbar.cpp
        UI/chart/chart.h
        UI/chart/chart.cpp
        UI/chart/chart_data.h
        UI/impedance_range/impedance_range.cpp
        UI/impedance_range/impedance_range.h
)

set(CORE
        ${CMAKE_CURRENT_SOURCE_DIR}/core/core.h
        ${CMAKE_CURRENT_SOURCE_DIR}/core/core.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/app_state_manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/core/app_state_manager.cpp
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
        eeg_theme.h
        ${CORE}
        ${UI}
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

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")
