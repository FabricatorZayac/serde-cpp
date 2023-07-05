##
# C++ Header-only library Makefile
#
# @file
# @version 0.1

CXX          := clang++
CXX_STANDARD := c++20
RM           := rm -f
RMDIR        := rm -rf
MKDIR        := mkdir
DEBUGGER     := lldb

INCLUDE      := include

TESTDIR      := test
TESTSRC      := $(TESTDIR)/src
TESTOBJ      := $(TESTDIR)/obj
TESTBIN      := $(TESTDIR)/bin

TESTSRCS     := $(shell find $(TESTSRC) -name "*.cpp")
TESTOBJS     := $(patsubst $(TESTSRC)/%.cpp, $(TESTOBJ)/%.o, $(TESTSRCS))
TESTS        := $(patsubst $(TESTOBJ)/%.o, $(TESTBIN)/%, $(TESTOBJS))

LDFLAGS      :=
CFLAGS       := -I$(INCLUDE) -std=$(CXX_STANDARD) -Wall -Wextra
DEBUGFLAGS   := -O0 -ggdb

define execute
$(1)

endef

.PHONY: clean debug lldb test all
.SECONDARY: $(TESTOBJS)

all:

test: $(TESTS)
	$(foreach x, $(TESTS), $(call execute, ./$(x)))

debug: CFLAGS := $(CFLAGS) $(DEBUGFLAGS)
debug: $(TESTS)

$(TESTBIN)/%: $(TESTOBJ)/%.o | $(TESTBIN)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TESTOBJ)/%.o: $(TESTSRC)/%.cpp $(TESTOBJ)
	$(CXX) $(CFLAGS) -c $< -o $@

$(TESTOBJ) $(TESTBIN):
	$(MKDIR) $@

clean:
	$(RMDIR) $(TESTDIR)/bin $(TESTDIR)/obj

# end
