PROJECT_NAME := car
EXTRA_COMPONENT_DIRS := managed_components
include $(IDF_PATH)/make/project.mk

ota_flash: all
	./tools/ota.py $(APP_BIN)
