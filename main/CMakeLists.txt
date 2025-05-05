set(COMPONENT_SRCS ${SRCS})
idf_component_register(SRCS "main.cpp" "input.cpp" "popup.cpp"
  INCLUDE_DIRS ""
  REQUIRES M5Cardputer fatfs)
