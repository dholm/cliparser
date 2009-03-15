# Top-level Makefile for CLI Parser
# $Id: Makefile 54 2009-03-13 08:06:44Z henry $

# Copyright (c) 2008, Henry Kwok
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the project nor the names of its contributors 
#       may be used to endorse or promote products derived from this software 
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY HENRY KWOK ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL HENRY KWOK BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

all: unix

dbg: unix_dbg

tests: unix_tests

help:
	@echo "The following targets are supported:"
	@echo
	@echo "unix         - Linux / MAC OS X / UNIX"
	@echo "unix_dbg     - Linux / MAC OS X / UNIX with debugging"
	@echo "unix_tests   - Linux / MAC OS X / UNIX with test suite"
	@echo
	@echo "To make a target, do 'make [target]' or 'make -s [target]'"
	@echo "to reduce the amount of make displays."

# Individual targets
unix:
	make -f Makefile.unix all

unix_dbg:
	make -f Makefile.unix DEBUG=TRUE all

unix_tests: unix
	./scripts/run_tests.py ./build/unix/bin unix

dox:
	doxygen doxygen.cfg

# Clean targets
unix_clean:
	rm -fr build/unix
	rm -fr build/unix_dbg

clean: unix_clean
