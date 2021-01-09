TARGET   = gnoraster


INCDIR   =inc
SRCDIR   =src
OBJDIR   =obj
BINDIR   =bin




#detect the OS (linux or OSX, dont care about windows)
UNAME := $(shell uname)

# compiling flags  
CFLAGS   = -I$(INCDIR)  

CC       = g++ 
LINKER   = g++

# CC       = gcc 
# LINKER   = gcc

# linking flags 

# LINUX
ifeq ($(UNAME), Linux)
	# LFLAGS   = -Wall -lglut -lGL -lGLU -lX11 -lXi  -lm -lz 
	LFLAGS   = -Wall -lm -lz -lpng16

endif

# OSX
ifeq ($(UNAME), Darwin)
	# LFLAGS   = -Wall -lm -lz -I/usr/X11/lib -lpng16
	LFLAGS   = -Wall -lm -lz -I/usr/X11/lib 

endif



SOURCES  := $(wildcard $(SRCDIR)/*.cpp)

OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
rm       = rm -f


$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"


.PHONY: cleanimg
cleanimg:
	@$(rm) *.bmp
	@echo "image cleanup done"


.PHONY: clean
clean:
	@$(rm) $(TARGET)
	@$(rm) $(OBJECTS)
	@echo "cleanup done"


.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@$(rm) $(OBJDIR)/*.o
	@echo "Executable removed!"
	 
