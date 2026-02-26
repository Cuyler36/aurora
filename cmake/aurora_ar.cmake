add_library(aurora_ar STATIC
        lib/dolphin/ar/ar.cpp
        lib/dolphin/ar/arq.cpp
        lib/dolphin/ar/ar.hpp
)
add_library(aurora::ar ALIAS aurora_ar)

target_include_directories(aurora_ar PUBLIC include)
target_link_libraries(aurora_ar PRIVATE aurora::os aurora::core)
