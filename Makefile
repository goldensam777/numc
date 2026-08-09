#
#
#
# Makefile de numc, bibliotheque de calcul scientifique en C
#
# Produit une librairie statique (libnumc.a) et un binaire de tests.
#
# Cibles :
#   all      - compile la librairie statique
#   tests    - compile les tests cmocka et les execute
#   valgrind - execute les tests sous valgrind
#   clean    - supprime les artefacts
#
#


# Includes the project configurations
include project.conf


#
# Validating project variables defined in project.conf
#
ifndef PROJECT_NAME
$(error Missing PROJECT_NAME. Put variables at project.conf file)
endif


# Gets the Operating system name
OS := $(shell uname -s)

# Default shell
SHELL := bash


# Color definition for print purpose (codes ANSI, fonctionne sur Linux et macOS)
BROWN := $(shell printf '\033[0;33m')
BLUE := $(shell printf '\033[1;34m')
END_COLOR := $(shell printf '\033[0m')


# Source code directory structure
BINDIR := bin
SRCDIR := src
LOGDIR := log
LIBDIR := lib
TESTDIR := test


# Source code file extension
SRCEXT := c


# Defines the C Compiler
CC := gcc


# Defines the language standards for GCC.
# -std=gnu11 : C11 (stdbool, _Thread_local, ...) avec extensions GNU.
# Voir README.md pour les conventions du projet.
STD := -std=gnu11

# Protection for stack-smashing attack
STACK := -fstack-protector-all -Wstack-protector

# Specifies to GCC the required warnings
WARNS := -Wall -Wextra -pedantic

# Include path of the public headers
INCLUDE := -I$(SRCDIR)

# Flags for compiling
CFLAGS := -O3 $(STD) $(STACK) $(WARNS) $(INCLUDE)

# Debug options
DEBUG := -g3 -DDEBUG=1

# Dependency libraries
LIBS := -lm

# Test libraries
TEST_LIBS := -lcmocka


# Artifacts
LIBRARY := lib$(PROJECT_NAME).a
TEST_BINARY := $(PROJECT_NAME)_tests


# Source and object files
SRCS := $(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%.$(SRCEXT),$(LIBDIR)/%.o,$(SRCS))

# Public headers of the library (excludes the umbrella header itself)
PUBLIC_HEADERS := $(filter-out $(SRCDIR)/numc.h,$(wildcard $(SRCDIR)/*.h))


#
# COMPILATION RULES
#

default: all

# Help message
help:
	@echo "numc - bibliotheque de calcul scientifique en C"
	@echo
	@echo "Cibles:"
	@echo "    all      - compile la librairie statique"
	@echo "    tests    - compile les tests (cmocka) et les execute"
	@echo "    valgrind - execute les tests sous valgrind"
	@echo "    clean    - supprime les artefacts"
	@echo "    help     - affiche ce message"


# Object files depend on numc.h and all public headers: any change to a
# header triggers the recompilation (and the regeneration of numc.h).
$(OBJECTS): $(SRCDIR)/numc.h $(wildcard $(SRCDIR)/*.h)


# Force target: the rule below always runs, so numc.h is regenerated both
# when a header is added and when one is removed.
.PHONY: FORCE
FORCE:

# Rule for the umbrella header numc.h
#
# numc.h is generated from every public header in $(SRCDIR) (except
# itself). Adding or removing a header updates numc.h automatically.
# The generated file is only rewritten when its content actually changes
# (cmp), so dependents are not spuriously rebuilt.
$(SRCDIR)/numc.h: $(PUBLIC_HEADERS) FORCE
	@echo -en "$(BROWN)GEN $(END_COLOR)numc.h (umbrella header)\n";
	@printf '%s\n' \
		'/*' \
		' * numc.h' \
		' *' \
		' * GENERATED FILE - DO NOT EDIT.' \
		' * Regenerate via : make' \
		' *' \
		' * Umbrella header of numc:' \
		' * includes every public header of the library.' \
		' *' \
		' * Conventions (see README.md):' \
		' *   - public symbols are prefixed by numc_' \
		' *   - functions return numc_status_t, results go into' \
		' *     output parameters' \
		' *   - last status readable via numc_error()' \
		' */' \
		'' \
		'#ifndef NUMC_H' \
		'#define NUMC_H' \
		'' > $@.tmp
	@for h in $(sort $(PUBLIC_HEADERS)); do \
		printf '#include "%s"\n' "$$(basename $$h)"; \
	done >> $@.tmp
	@printf '%s\n' '' '#endif /* NUMC_H */' >> $@.tmp
	@cmp -s $@ $@.tmp && rm -f $@.tmp || mv $@.tmp $@


# Rule for object binaries compilation
$(LIBDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo -en "$(BROWN)CC $(END_COLOR)";
	$(CC) -c $< -o $@ $(DEBUG) $(CFLAGS) $(LIBS)


# Rule for link and generate the static library
all: $(SRCDIR)/numc.h $(BINDIR)/$(LIBRARY)

$(BINDIR)/$(LIBRARY): $(OBJECTS)
	@echo -en "$(BROWN)AR $(END_COLOR)";
	ar rcs $@ $(OBJECTS)
	@echo -en "\n--\nStatic library placed at" \
			  "$(BROWN)$(BINDIR)/$(LIBRARY)$(END_COLOR)\n";


# Compile tests and run the test binary
tests: $(SRCDIR)/numc.h $(BINDIR)/$(LIBRARY)
	@echo -en "$(BROWN)CC $(END_COLOR)";
	$(CC) $(wildcard $(TESTDIR)/*.$(SRCEXT)) -o $(BINDIR)/$(TEST_BINARY) \
		$(DEBUG) $(CFLAGS) -L$(BINDIR) -l$(PROJECT_NAME) $(TEST_LIBS) $(LIBS)
	@echo -en "$(BROWN) Running tests: $(END_COLOR)";
	./$(BINDIR)/$(TEST_BINARY)


# Rule for run valgrind tool over the tests
valgrind: tests
	valgrind \
		--track-origins=yes \
		--leak-check=full \
		--leak-resolution=high \
		--log-file=$(LOGDIR)/$@.log \
		$(BINDIR)/$(TEST_BINARY)
	@echo -en "\nCheck the log file: $(LOGDIR)/$@.log\n"


# Rule for cleaning the project
clean:
	@rm -rvf $(BINDIR)/* $(LIBDIR)/* $(LOGDIR)/*;
