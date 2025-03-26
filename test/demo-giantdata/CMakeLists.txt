add_library(giantdata SHARED waveforms.cpp waveform_generator_input.cpp)
target_link_libraries(giantdata PRIVATE meld::core)

add_executable(unfold_transform_fold unfold_transform_fold.cpp)
target_link_libraries(unfold_transform_fold PRIVATE giantdata meld::core)
