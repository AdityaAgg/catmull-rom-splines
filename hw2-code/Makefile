LIB_CODE_BASE=../openGLHelper-starterCode

hw2_CXX_SRC=hw2.cpp
hw2_HEADER=
hw2_OBJ=$(notdir $(patsubst %.cpp,%.o,$(hw2_CXX_SRC)))

LIB_CODE_CXX_SRC=$(wildcard $(LIB_CODE_BASE)/*.cpp)
LIB_CODE_HEADER=$(wildcard $(LIB_CODE_BASE)/*.h)
LIB_CODE_OBJ=$(notdir $(patsubst %.cpp,%.o,$(LIB_CODE_CXX_SRC)))

IMAGE_LIB_SRC=$(wildcard ../external/imageIO/*.cpp)
IMAGE_LIB_HEADER=$(wildcard ../external/imageIO/*.h)
IMAGE_LIB_OBJ=$(notdir $(patsubst %.cpp,%.o,$(IMAGE_LIB_SRC)))

HEADER=$(hw2_HEADER) $(LIB_CODE_HEADER) $(IMAGE_LIB_HEADER)
CXX_OBJ=$(hw2_OBJ) $(LIB_CODE_OBJ) $(IMAGE_LIB_OBJ)

CXX=g++
TARGET=hw2
CXXFLAGS=-DGLM_FORCE_RADIANS
OPT=-O3 -std=c++11

UNAME_S=$(shell uname -s)

ifeq ($(UNAME_S),Linux)
  PLATFORM=Linux
  INCLUDE=-I../external/glm/ -I$(LIB_CODE_BASE) -I../external/imageIO
  LIB=-lGLEW -lGL -lglut -ljpeg
  LDFLAGS=
else
  PLATFORM=Mac OS
  INCLUDE=-I../external/glm/ -I$(LIB_CODE_BASE) -I../external/imageIO -I../external/jpeg-9a-mac/include
  LIB=-framework OpenGL -framework GLUT ../external/jpeg-9a-mac/lib/libjpeg.a
  CXXFLAGS+= -Wno-deprecated-declarations
  LDFLAGS=-Wl,-w
endif

all: $(TARGET)

$(TARGET): $(CXX_OBJ)
	$(CXX) $(LDFLAGS) $^ $(OPT) $(LIB) -o $@

$(hw2_OBJ):%.o: %.cpp $(HEADER)
	$(CXX) -c $(CXXFLAGS) $(OPT) $(INCLUDE) $< -o $@

$(LIB_CODE_OBJ):%.o: $(LIB_CODE_BASE)/%.cpp $(LIB_CODE_HEADER)
	$(CXX) -c $(CXXFLAGS) $(OPT) $(INCLUDE) $< -o $@

$(IMAGE_LIB_OBJ):%.o: ../external/imageIO/%.cpp $(IMAGE_LIB_HEADER)
	$(CXX) -c $(CXXFLAGS) $(OPT) $(INCLUDE) $< -o $@

clean:
	rm -rf *.o $(TARGET)
