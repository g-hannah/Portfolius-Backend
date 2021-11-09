CC := g++

SOURCE_FILES := \
	server.cpp \
	exchangeratesmanager.cpp \
	socketobj.cpp \
	listeningsocket.cpp \
	client.cpp

OBJECT_FILES := \
	server.o \
	exchangeratesmanager.o \
	socketobj.o \
	listeningsocket.o \
	client.o

COMPILE_FLAGS := -Wall -Werror -D_FORTIFY_SOURCE=2 -fstack-protector-all -Wl,-z,relro

.PHONY: clean

portfolius-server: $(OBJECT_FILES)
	$(CC) $(COMPILE_FLAGS) -O2 -o portfolius-server $(OBJECT_FILES)

$(OBJECT_FILES): $(SOURCE_FILES)
ifeq ($(DEBUG),1)
	@echo "Compiling DEBUG version"
	$(CC) -g -DDEBUG $(COMPILE_FLAGS) -O2 -c $(SOURCE_FILES)
else
	@echo "Compiling PRODUCTION version"
	$(CC) $(COMPILE_FLAGS) -O2 -c $(SOURCE_FILES)
endif

clean:
	rm *.o
