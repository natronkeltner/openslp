##---------------------------------------------------------------------------
##  Copyright (C) 2000 Caldera Systems, Inc
##  All rights reserved.
##
##  Redistribution and use in source and binary forms, with or without
##  modification, are permitted provided that the following conditions are
##  met:
##
##       Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##
##       Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in
##       the documentation and/or other materials provided with the
##       distribution.
##
##       Neither the name of Caldera Systems nor the names of its
##       contributors may be used to endorse or promote products derived
##       from this software without specific prior written permission.
##
##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
##  `AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
##  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
##  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CALDERA
##  SYSTEMS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
##  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
##  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,
##  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
##  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
##  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
##  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##---------------------------------------------------------------------------

#if you're building on Irix, replace .la with .a below
noinst_LTLIBRARIES = libslpattr.la

if ENABLE_PREDICATES
PREDICATE_SRCS = libslpattr.c
else
PREDICATE_SRCS = libslpattr_tiny.c
endif

#if you're building on Irix, replace _la_ with _a_ below
EXTRA_libslpattr_la_SOURCES = libslpattr.c libslpattr_tiny.c
libslpattr_la_SOURCES = $(PREDICATE_SRCS) libslpattr_internal.h

noinst_HEADERS = libslpattr.h

