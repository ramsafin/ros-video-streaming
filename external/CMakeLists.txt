include(FetchContent)

FetchContent_Declare(
  plog
  GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
  GIT_TAG        1.1.10
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/plog
)

FetchContent_MakeAvailable(plog)
