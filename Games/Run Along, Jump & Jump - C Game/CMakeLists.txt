cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
include(/home/jack/pico/pico-sdk/pico_sdk_init.cmake)

# cmake -DPICO_BOARD=pico .. && make
SET(RAJAJ_SINGLE ON)
# cmake -DPICO_BOARD=pico_w .. && make
# SET(RAJAJ_SINGLE OFF)

if (RAJAJ_SINGLE)
	project(rajaj-single)
else()
	project(rajaj-server)
	project(rajaj-client)
endif()

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (RAJAJ_SINGLE)
	add_executable(rajaj-single
		main-single.c
		RAJAJ.c
		ZIP96Pico.c
		ws2812.pio)
	
	pico_generate_pio_header(rajaj-single ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

	target_include_directories(rajaj-single PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/..)
	
	target_link_libraries(rajaj-single
		pico_stdlib
		hardware_gpio
		hardware_pio
		hardware_pwm)
	
	pico_add_extra_outputs(rajaj-single)
else()
	# RAJAJ Server
	add_executable(rajaj-server
		main-server.c
		RAJAJ.c
		ZIP96Pico.c
		ws2812.pio
		PicoWNetworking.c)
	
	pico_generate_pio_header(rajaj-server ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

	target_include_directories(rajaj-server PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/..)
	
	target_link_libraries(rajaj-server
		pico_stdlib
		pico_multicore
		hardware_gpio
		hardware_pio
		hardware_pwm
		pico_cyw43_arch_lwip_threadsafe_background)
	
	pico_add_extra_outputs(rajaj-server)

	# RAJAJ Client
	add_executable(rajaj-client
		main-client.c
		RAJAJ.c
		ZIP96Pico.c
		ws2812.pio
		PicoWNetworking.c)
	
	pico_generate_pio_header(rajaj-client ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

	target_include_directories(rajaj-client PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/..)
	
	target_link_libraries(rajaj-client
		pico_stdlib
		pico_multicore
		hardware_gpio
		hardware_pio
		hardware_pwm
		pico_cyw43_arch_lwip_threadsafe_background)
	
	pico_add_extra_outputs(rajaj-client)
endif()
