################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/lightmysql/enum.cpp \
../src/lightmysql/mydbglog.cpp \
../src/lightmysql/resourcepool.cpp 

CC_SRCS += \
../src/lightmysql/cfghelper.cc \
../src/lightmysql/connection.cc \
../src/lightmysql/query.cc \
../src/lightmysql/result.cc \
../src/lightmysql/row.cc \
../src/lightmysql/row_std.cc \
../src/lightmysql/transaction.cc 

OBJS += \
./src/lightmysql/cfghelper.o \
./src/lightmysql/connection.o \
./src/lightmysql/enum.o \
./src/lightmysql/mydbglog.o \
./src/lightmysql/query.o \
./src/lightmysql/resourcepool.o \
./src/lightmysql/result.o \
./src/lightmysql/row.o \
./src/lightmysql/row_std.o \
./src/lightmysql/transaction.o 

CC_DEPS += \
./src/lightmysql/cfghelper.d \
./src/lightmysql/connection.d \
./src/lightmysql/query.d \
./src/lightmysql/result.d \
./src/lightmysql/row.d \
./src/lightmysql/row_std.d \
./src/lightmysql/transaction.d 

CPP_DEPS += \
./src/lightmysql/enum.d \
./src/lightmysql/mydbglog.d \
./src/lightmysql/resourcepool.d 


# Each subdirectory must supply rules for building sources it contributes
src/lightmysql/%.o: ../src/lightmysql/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/lightmysql/%.o: ../src/lightmysql/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


