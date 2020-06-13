# BSD 3-Clause License
#
# Copyright (c) 2020, Aleksei Dynda
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

default: all

.PHONY: all unittest check clean help

SINGLE_THREAD ?= n
USE_STL ?= y

CPPFLAGS += -I./include -I./src \

ifeq ($(SINGLE_THREAD),y)
    CPPFLAGS += -DSM_ENGINE_MULTITHREAD=0
else
    CPPFLAGS += -DSM_ENGINE_MULTITHREAD=1
endif

ifeq ($(USE_STL),y)
    CPPFLAGS += -DSM_ENGINE_USE_STL=1
else
    CPPFLAGS += -DSM_ENGINE_USE_STL=0
endif

OBJS=src/iengine.o src/engine.o \


all: $(OBJS)
	$(AR) rcs libsm_engine.a $(OBJS)

clean:
	@rm -rf $(OBJS) libsm_engine.a

help:
	@echo ""
	@echo "targets:"
	@echo "    all                                builds library"
	@echo "    unittest                           builds unit tests"
	@echo "    check                              builds and run unit tests"
	@echo "    examples                           builds examples"
	@echo "available options: "
	@echo "    SINGLE_THREAD    y/(n - default)   avoid using locks, assume that FSM is accessed in single thread"
	@echo "    USE_STL          (y - default)/n   use standard stl classes (stack, vector, list)."
	@echo "                                       The library allows to use internal minor implementation "
	@echo "                                       for these classes on platforms that do not support STL"

# ================================== Unit Tests ==============================

include Makefile.cpputest

# ================================== Examples ================================

include Makefile.examples
