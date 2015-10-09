APP_STL := gnustl_static
APP_DEBUG := $(strip $(NDK_DEBUG))
ifeq ($(APP_DEBUG),0)
  APP_DEBUG:= false
endif
ifeq ($(APP_DEBUG),1)
  APP_DEBUG := true
endif
ifdef APP_DEBUG
APP_OPTIM := debug
else
APP_OPTIM := release
endif
