INC += 	-Iapp/inc \
		-Iapp/test/inc

SRCS += app/src/main.c	\
		app/test/src/test_gpio.c \
		app/test/src/test_timer.c \
		app/test/src/test_console.c \
		app/test/src/test_can.c	\
		app/src/ring_buffer.c	\
		app/src/wheel_controller.c