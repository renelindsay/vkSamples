cmake_minimum_required(VERSION 3.4.1)

project(cpdf)

include_directories ( ./                  )
include_directories ( thirdparty/         )

aux_source_directory( ./          SRC_LIST)
aux_source_directory( thirdparty/ SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
