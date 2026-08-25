PYTHON_INC := $(shell python3-config --includes)
PYTHON_LIB := $(shell python3-config --libs)
%.so: %.c
	gcc --shared -fPIC $(PYTHON_INC) $< $(PYTHON_LIB) -o $@

clean:
	rm -fr __pycache__ *.so
