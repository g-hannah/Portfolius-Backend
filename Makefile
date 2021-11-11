CC := g++

SOURCE_FILES := \
	main.cpp \
	server.cpp \
	exchangeratesmanager.cpp \
	socketobj.cpp \
	listeningsocket.cpp \
	client.cpp

OBJECT_FILES := \
	main.o \
	server.o \
	exchangeratesmanager.o \
	socketobj.o \
	listeningsocket.o \
	client.o

LIBARIES := -lssl -lcrypto

COMPILE_FLAGS := -Wall -Werror -D_FORTIFY_SOURCE=2 -fstack-protector-all -Wl,-z,relro

.PHONY: clean

portfolius-server: $(OBJECT_FILES)
	$(CC) $(COMPILE_FLAGS) -O2 -o portfolius-server $(OBJECT_FILES) $(LIBRARIES)

$(OBJECT_FILES): $(SOURCE_FILES)
ifeq ($(DEBUG),1)
	@echo "Compiling DEBUG version"
	$(CC) -g -DDEBUG $(COMPILE_FLAGS) -O2 -c $(SOURCE_FILES) $(LIBRARIES)
else
	@echo "Compiling PRODUCTION version"
	$(CC) $(COMPILE_FLAGS) -O2 -c $(SOURCE_FILES) $(LIBRARIES)
endif

clean:
	rm *.o
