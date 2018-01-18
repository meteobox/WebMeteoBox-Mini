# WebMeteoBox-Mini

------ Build started: Project: WebMeteoBox-mini, Configuration: Debug AVR ------
Build started.
Project "WebMeteoBox-mini.cproj" (default targets):
Target "PreBuildEvent" skipped, due to false condition; ('$(PreBuildEvent)'!='') was evaluated as (''!='').
Target "CoreBuild" in file "C:\Program Files (x86)\Atmel\Studio\7.0\Vs\Compiler.targets" from project "C:\projects\MeteoBox-mini\WebMeteoBox3\WebMeteoBox-mini.cproj" (target "Build" depends on it):
	Task "RunCompilerTask"
		Shell Utils Path C:\Program Files (x86)\Atmel\Studio\7.0\shellUtils
		C:\Program Files (x86)\Atmel\Studio\7.0\shellUtils\make.exe all --jobs 8 --output-sync 
		Building file: ../Source/i2csoft.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/i2csoft.d" -MT"Source/i2csoft.d" -MT"Source/i2csoft.o"   -o "Source/i2csoft.o" "../Source/i2csoft.c" 
		Finished building: ../Source/i2csoft.c
		Building file: ../Source/main.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/main.d" -MT"Source/main.d" -MT"Source/main.o"   -o "Source/main.o" "../Source/main.c" 
		Finished building: ../Source/main.c
		Building file: ../Source/dhcp_client.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/dhcp_client.d" -MT"Source/dhcp_client.d" -MT"Source/dhcp_client.o"   -o "Source/dhcp_client.o" "../Source/dhcp_client.c" 
		Finished building: ../Source/dhcp_client.c
		Building file: ../Source/DHT.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/DHT.d" -MT"Source/DHT.d" -MT"Source/DHT.o"   -o "Source/DHT.o" "../Source/DHT.c" 
		Finished building: ../Source/DHT.c
		Building file: ../Source/dnslkup.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/dnslkup.d" -MT"Source/dnslkup.d" -MT"Source/dnslkup.o"   -o "Source/dnslkup.o" "../Source/dnslkup.c" 
		Finished building: ../Source/dnslkup.c
		Building file: ../Source/led_display.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/led_display.d" -MT"Source/led_display.d" -MT"Source/led_display.o"   -o "Source/led_display.o" "../Source/led_display.c" 
		Finished building: ../Source/led_display.c
		Building file: ../Source/enc28j60.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/enc28j60.d" -MT"Source/enc28j60.d" -MT"Source/enc28j60.o"   -o "Source/enc28j60.o" "../Source/enc28j60.c" 
		Finished building: ../Source/enc28j60.c
		Building file: ../Source/ip_arp_udp_tcp.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/ip_arp_udp_tcp.d" -MT"Source/ip_arp_udp_tcp.d" -MT"Source/ip_arp_udp_tcp.o"   -o "Source/ip_arp_udp_tcp.o" "../Source/ip_arp_udp_tcp.c" 
		Finished building: ../Source/ip_arp_udp_tcp.c
		Building file: ../Source/VirtualWire.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/VirtualWire.d" -MT"Source/VirtualWire.d" -MT"Source/VirtualWire.o"   -o "Source/VirtualWire.o" "../Source/VirtualWire.c" 
		Finished building: ../Source/VirtualWire.c
		Building file: ../Source/OWIcrc.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/OWIcrc.d" -MT"Source/OWIcrc.d" -MT"Source/OWIcrc.o"   -o "Source/OWIcrc.o" "../Source/OWIcrc.c" 
		Finished building: ../Source/OWIcrc.c
		Building file: ../Source/OWIHighLevelFunctions.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/OWIHighLevelFunctions.d" -MT"Source/OWIHighLevelFunctions.d" -MT"Source/OWIHighLevelFunctions.o"   -o "Source/OWIHighLevelFunctions.o" "../Source/OWIHighLevelFunctions.c" 
		Finished building: ../Source/OWIHighLevelFunctions.c
		Building file: ../Source/OWISWBitFunctions.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/OWISWBitFunctions.d" -MT"Source/OWISWBitFunctions.d" -MT"Source/OWISWBitFunctions.o"   -o "Source/OWISWBitFunctions.o" "../Source/OWISWBitFunctions.c" 
		Finished building: ../Source/OWISWBitFunctions.c
		Building file: ../Source/task_config.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/task_config.d" -MT"Source/task_config.d" -MT"Source/task_config.o"   -o "Source/task_config.o" "../Source/task_config.c" 
		Finished building: ../Source/task_config.c
		Building file: ../Source/term.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/term.d" -MT"Source/term.d" -MT"Source/term.o"   -o "Source/term.o" "../Source/term.c" 
		Finished building: ../Source/term.c
		Building file: ../Source/protocol.c
		Invoking: AVR/GNU C Compiler : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe"  -x c -funsigned-char -funsigned-bitfields -DDEBUG -DF_CPU=8000000 -D__AVR_LIBC_DEPRECATED_ENABLE__  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g3 -Wall -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -c -std=gnu99 -MD -MP -MF "Source/protocol.d" -MT"Source/protocol.d" -MT"Source/protocol.o"   -o "Source/protocol.o" "../Source/protocol.c" 
		Finished building: ../Source/protocol.c
		Building target: WebMeteoBox-mini.elf
		Invoking: AVR/GNU Linker : 5.4.0
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe" -o WebMeteoBox-mini.elf  Source/dhcp_client.o Source/DHT.o Source/dnslkup.o Source/i2csoft.o Source/led_display.o Source/main.o Source/enc28j60.o Source/ip_arp_udp_tcp.o Source/OWIcrc.o Source/OWIHighLevelFunctions.o Source/OWISWBitFunctions.o Source/protocol.o Source/task_config.o Source/term.o Source/VirtualWire.o   -Wl,-Map="WebMeteoBox-mini.map" -Wl,--start-group -Wl,-lprintf_flt  -Wl,--end-group -Wl,-L"C:\MeteoBox3\WebMeteoBox3\WebMeteoBox3"  -Wl,--gc-sections -mrelax -mmcu=atmega328p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega328p" -Wl,-u,vfprintf -lprintf_flt -lm  
		Finished building target: WebMeteoBox-mini.elf
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "WebMeteoBox-mini.elf" "WebMeteoBox-mini.hex"
		"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "WebMeteoBox-mini.elf"
		   text	   data	    bss	    dec	    hex	filename
		  29570	    216	   1615	  31401	   7aa9	WebMeteoBox-mini.elf
	Done executing task "RunCompilerTask".
	Task "RunOutputFileVerifyTask"
				Program Memory Usage 	:	29786 bytes   90,9 % Full
				Data Memory Usage 		:	1831 bytes   89,4 % Full
	Done executing task "RunOutputFileVerifyTask".
Done building target "CoreBuild" in project "WebMeteoBox-mini.cproj".
Target "PostBuildEvent" skipped, due to false condition; ('$(PostBuildEvent)' != '') was evaluated as ('' != '').
Target "Build" in file "C:\Program Files (x86)\Atmel\Studio\7.0\Vs\Avr.common.targets" from project "C:\projects\MeteoBox-mini\WebMeteoBox3\WebMeteoBox-mini.cproj" (entry point):
Done building target "Build" in project "WebMeteoBox-mini.cproj".
Done building project "WebMeteoBox-mini.cproj".

Build succeeded.
========== Build: 1 succeeded or up-to-date, 0 failed, 0 skipped ==========
