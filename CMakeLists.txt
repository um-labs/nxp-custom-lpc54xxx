set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.29)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(OM13092 C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#Uncomment for hardware floating point
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

file(GLOB DRIVER_INC_DIRS "Drivers/*/Inc" "CMSIS/Include" "CMSIS/*/Include")

include_directories(
        ${DRIVER_INC_DIRS}
        Core/Inc
        Middlewares/FreeRTOS-Kernel/include
        Middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM4F
        Middlewares/tensorflow
        Middlewares/gemmlowp
        Middlewares/flatbuffers/include
        Middlewares/kissfft
        Middlewares/ruy
)

add_definitions(
        -DDEBUG
        -DCPU_LPC54608
        -DCPU_LPC54608J512ET180
        -DLV_LVGL_H_INCLUDE_SIMPLE
        -DCMSIS_NN
        -DKERNELS_OPTIMIZED_FOR_SPEED
        -DCPU_M4=1
        -DCMSIS_DEVICE_ARM_CORTEX_M_XX_HEADER_FILE="LPC54608.h"
)

file(GLOB_RECURSE SOURCES
        "Core/*.*"
        "Drivers/*.*"
        "Middlewares/FreeRTOS-Kernel/*.*"
        "Middlewares/lvgl/*.*"
        "CMSIS/NN/Source/*.*"
)

set(TFLM
        "Middlewares/tensorflow/tensorflow/lite/micro/cortex_m_generic/debug_log.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/fake_micro_context.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/flatbuffer_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/memory_helpers.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_allocation_info.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_context.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_interpreter.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_interpreter_context.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_interpreter_graph.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_log.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_op_resolver.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_profiler.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_resource_variable.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/cortex_m_generic/micro_time.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/micro_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/mock_micro_graph.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/recording_micro_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/system_setup.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/test_helper_custom_ops.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/test_helpers.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/kernel_util.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/tensor_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/reference/comparisons.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/runtime_shape.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/common.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/quantization_util.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/portable_tensor_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/kernels/internal/tensor_ctypes.cc"
        "Middlewares/tensorflow/tensorflow/lite/core/c/common.cc"
        "Middlewares/tensorflow/tensorflow/lite/core/api/tensor_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/core/api/flatbuffer_conversions.cc"
        "Middlewares/tensorflow/tensorflow/compiler/mlir/lite/core/api/error_reporter.cc"
        "Middlewares/tensorflow/tensorflow/compiler/mlir/lite/schema/schema_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/activations.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/activations_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/add.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/add_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/add_n.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/arg_min_max.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/assign_variable.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/batch_matmul.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/batch_to_space_nd.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/broadcast_args.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/broadcast_to.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/call_once.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cast.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/ceil.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/circular_buffer.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/circular_buffer_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/comparisons.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/concatenation.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/conv_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cumsum.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/depth_to_space.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/depthwise_conv_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/dequantize.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/dequantize_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/detection_postprocess.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/div.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/elementwise.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/elu.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/embedding_lookup.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/ethosu.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/exp.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/expand_dims.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/fill.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/floor.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/floor_div.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/floor_mod.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/fully_connected_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/gather.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/gather_nd.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/hard_swish.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/hard_swish_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/if.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/kernel_runner.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/kernel_util.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/l2norm.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/l2_pool_2d.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/leaky_relu.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/leaky_relu_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/logical.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/logical_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/logistic.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/logistic_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/log_softmax.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/lstm_eval.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/lstm_eval_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/maximum_minimum.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/micro_tensor_utils.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/mirror_pad.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/mul.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/mul_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/neg.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/pack.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/pad.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/pooling_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/prelu.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/prelu_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/quantize.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/quantize_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/read_variable.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/reduce.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/reduce_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/reshape.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/reshape_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/resize_bilinear.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/round.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/select.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/shape.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/slice.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/softmax_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/space_to_batch_nd.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/space_to_depth.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/split.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/split_v.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/squared_difference.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/squeeze.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/strided_slice.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/strided_slice_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/sub.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/sub_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/svdf_common.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/tanh.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/transpose.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/transpose_conv.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/unidirectional_sequence_lstm.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/unpack.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/var_handle.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/while.cc"
        "Middlewares/tensorflow/tensorflow/lite/micro/kernels/zeros_like.cc"
        "Middlewares/tensorflow/signal/src/kiss_fft_wrappers/kiss_fft_float.cc"
        "Middlewares/tensorflow/signal/src/kiss_fft_wrappers/kiss_fft_int16.cc"
        "Middlewares/tensorflow/signal/src/kiss_fft_wrappers/kiss_fft_int32.cc"
        "Middlewares/tensorflow/signal/micro/kernels/delay.cc"
        "Middlewares/tensorflow/signal/micro/kernels/energy.cc"
        "Middlewares/tensorflow/signal/micro/kernels/fft_auto_scale_kernel.cc"
        "Middlewares/tensorflow/signal/micro/kernels/fft_auto_scale_common.cc"
        "Middlewares/tensorflow/signal/micro/kernels/filter_bank.cc"
        "Middlewares/tensorflow/signal/micro/kernels/filter_bank_log.cc"
        "Middlewares/tensorflow/signal/micro/kernels/filter_bank_square_root.cc"
        "Middlewares/tensorflow/signal/micro/kernels/filter_bank_square_root_common.cc"
        "Middlewares/tensorflow/signal/micro/kernels/filter_bank_spectral_subtraction.cc"
        "Middlewares/tensorflow/signal/micro/kernels/framer.cc"
        "Middlewares/tensorflow/signal/micro/kernels/irfft.cc"
        "Middlewares/tensorflow/signal/micro/kernels/rfft.cc"
        "Middlewares/tensorflow/signal/micro/kernels/stacker.cc"
        "Middlewares/tensorflow/signal/micro/kernels/overlap_add.cc"
        "Middlewares/tensorflow/signal/micro/kernels/pcan.cc"
        "Middlewares/tensorflow/signal/micro/kernels/window.cc"
        "Middlewares/tensorflow/signal/src/circular_buffer.cc"
        "Middlewares/tensorflow/signal/src/energy.cc"
        "Middlewares/tensorflow/signal/src/fft_auto_scale.cc"
        "Middlewares/tensorflow/signal/src/filter_bank.cc"
        "Middlewares/tensorflow/signal/src/filter_bank_log.cc"
        "Middlewares/tensorflow/signal/src/filter_bank_square_root.cc"
        "Middlewares/tensorflow/signal/src/filter_bank_spectral_subtraction.cc"
        "Middlewares/tensorflow/signal/src/irfft_float.cc"
        "Middlewares/tensorflow/signal/src/irfft_int16.cc"
        "Middlewares/tensorflow/signal/src/irfft_int32.cc"
        "Middlewares/tensorflow/signal/src/log.cc"
        "Middlewares/tensorflow/signal/src/max_abs.cc"
        "Middlewares/tensorflow/signal/src/msb_32.cc"
        "Middlewares/tensorflow/signal/src/msb_64.cc"
        "Middlewares/tensorflow/signal/src/overlap_add.cc"
        "Middlewares/tensorflow/signal/src/pcan_argc_fixed.cc"
        "Middlewares/tensorflow/signal/src/rfft_float.cc"
        "Middlewares/tensorflow/signal/src/rfft_int16.cc"
        "Middlewares/tensorflow/signal/src/rfft_int32.cc"
        "Middlewares/tensorflow/signal/src/square_root_32.cc"
        "Middlewares/tensorflow/signal/src/square_root_64.cc"
        "Middlewares/tensorflow/signal/src/window.cc"
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/LPC54608J512_flash.ld)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_PARALLEL_LEVEL 20)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})
add_link_options(-Wl,--no-warn-rwx-segments)

add_subdirectory(Middlewares/lvgl)

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${TFLM} ${LINKER_SCRIPT})

target_link_libraries(${PROJECT_NAME}.elf
        ${CMAKE_SOURCE_DIR}/Drivers/libs/libpower_hardabi.a
)
target_link_libraries(${PROJECT_NAME}.elf lvgl)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
