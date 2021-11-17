CC := g++

SOURCE_FILES := \
	main.cpp \
	server.cpp \
	applicationsettings.cpp \
	exchangeratesmanager.cpp \
	socketobj.cpp \
	listeningsocket.cpp \
	client.cpp \
	filehandler.cpp \
	rate.cpp

OBJECT_FILES := \
	main.o \
	server.o \
	applicationsettings.o \
	exchangeratesmanager.o \
	socketobj.o \
	listeningsocket.o \
	client.o \
	filehandler.o \
	rate.o

LIBRARIES := ./lib/libssl.a ./lib/libcrypto.a -lpthread -ldl

INCLUDES := -I./include -I./lib/openssl-1.1.1k/include

COMPILE_FLAGS := -Wall -Werror -D_FORTIFY_SOURCE=2 -fstack-protector-all -Wl,-z,relro,-z,noexecstack

.PHONY: clean

portfolius-server: $(OBJECT_FILES)
	$(CC) $(COMPILE_FLAGS) $(INCLUDES) -O2 -o portfolius-server $(OBJECT_FILES) $(LIBRARIES)

$(OBJECT_FILES): $(SOURCE_FILES)
ifeq ($(DEBUG),1)
	@echo "Compiling DEBUG version"
	$(CC) -g -DDEBUG $(COMPILE_FLAGS) $(INCLUDES) -O2 -c $(SOURCE_FILES)
else
	@echo "Compiling PRODUCTION version"
	$(CC) $(COMPILE_FLAGS) $(INCLUDES) -O2 -c $(SOURCE_FILES)
endif

clean:
	rm *.o
