LIBRARY := libmem_mgr.a
SOURCES := MemoryManager.cpp SemaphoreManager.cpp Semaphore.cpp Lock.cpp ReadWriteLock.cpp
OBJECTS := $(SOURCES:.cpp=.o)
HEADERS := $(SOURCES:.cpp=.hpp)
CFLAGS  := -Wall -pedantic
HDR_DIR := /usr/local/include/
LIB_DIR := /usr/local/lib/

all: $(LIBRARY)

%.o : %.cpp
	@echo Compiling $< ...;\
		$(CXX) -Wno-variadic-macros -c $(CFLAGS) $< -o $@

$(LIBRARY): $(OBJECTS)
	@echo creating lib ...;\
		ar -rs $(LIBRARY) $(OBJECTS)

#libtool -o $(LIBRARY) -static $(OBJECTS)

clean:
	@echo removing objects ...; \
		rm -f $(OBJECTS); \
		echo removing library ...; \
		rm -f $(LIBRARY);

install:
	@echo installing headers and libs ...;\
		cp $(HEADERS) $(HDR_DIR) &&\
		cp $(LIBRARY) $(LIB_DIR);

cleanall:
	@echo removing headers and libs ...;\
		cd $(LIB_DIR) &&\
		rm -f $(LIBRARY);\
		cd $(HDR_DIR) &&\
		rm -f $(HEADERS);

.PHONY : all clean cleanall install
