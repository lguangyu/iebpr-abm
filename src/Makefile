# run make NO_PYTHON_INTERFACE=1 to build without python interface
NO_PYTHON_INTERFACE ?=
# manual build make file, intended for test purposes
# to install as a python module, pip is recommended
PYTHON_PREFIX ?= /usr
PYTHON_VERSION ?= 3.10
PYTHON_INCLUCE ?= $(PYTHON_PREFIX)/include/python$(PYTHON_VERSION)
PYTHON_LIB ?= $(PYTHON_PREFIX)/lib
NUMPY_PREFIX ?= $(PYTHON_LIB)/python$(PYTHON_VERSION)/site-packages/numpy/core
NUMPY_INCLUDE ?= $(NUMPY_PREFIX)/include
NUMPY_LIB ?= $(NUMPY_PREFIX)/lib

CFLAGS := -g -O0 -Wall -Wextra -Wno-sign-compare -Wno-missing-braces $(CFLAGS)
LDFLAGS := $(LDFLAGS)
LIBS := $(LIBS)

ifeq ($(NO_PYTHON_INTERFACE),)
CFLAGS += -I$(PYTHON_INCLUCE) -I$(NUMPY_INCLUDE)
LDFLAGS += -L$(PYTHON_LIB) -L$(NUMPY_LIB)
LIBS += -lpython$(PYTHON_VERSION) 
else
CFLAGS += -DNO_PYTHON_INTERFACE
endif

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp, %.o, $(SRC))
DEP := $(patsubst %.cpp, %.d, $(SRC))
TARGET := _iebpr.so

.PHONY: build
build: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -shared $^ -o $@ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) -std=c++11 -fPIC -MMD -MP -c $< -o $@ $(CFLAGS) -I../include

.PHONY: clean
clean:
	-$(RM) $(TARGET)
	-$(RM) $(OBJ)
	-$(RM) $(DEP)

-include $(DEP)
