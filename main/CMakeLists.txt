# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES "esp32-camera")
set(COMPONENT_PRIV_REQUIRES)

file(GLOB SRCS *.c)
set(COMPONENT_SRCS ${SRCS})
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
