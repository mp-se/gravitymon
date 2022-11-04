set JAVA_HOME=c:\dev\jdk-17.0.1
%JAVA_HOME%\bin\java -jar EspStackTraceDecoder.jar c:\Users\magnu\.platformio\packages\toolchain-xtensa\bin\xtensa-lx106-elf-addr2line.exe c:\dev\gravitymon\.pio\build\gravity-release\firmware.elf exception_dump.txt

rem %JAVA_HOME%\bin\java -jar EspStackTraceDecoder.jar c:\Users\magnu\.platformio\packages\toolchain-xtensa\bin\xtensa-lx106-elf-addr2line.exe c:\dev\gravitymon\.pio\build\gravity-debug\firmware.elf exception_dump.txt