cmake_minimum_required(VERSION 3.16)
project(hui LANGUAGES CXX)

# Установить стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Charts)

# Пути к твоим заголовкам
include_directories(include src)

# Исходники и заголовки
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/tableconfigdialog.cpp
    src/configmanager.cpp
    src/graphwidget.cpp
)

set(HEADERS
    include/mainwindow.h
    include/tableconfigdialog.h
    include/configmanager.h
    include/temperaturegause.h
    include/graphwidget.h
)

# Создать исполняемый файл
add_executable(hui ${SOURCES} ${HEADERS})

# Линковка с Qt6
target_link_libraries(hui
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Charts
)



# Включить автоматическую обработку MOC, UIC и RCC
set_target_properties(hui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

