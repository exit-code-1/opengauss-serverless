#
# Copyright (c) 2020 Huawei Technologies Co.,Ltd.
#
# openGauss is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# ---------------------------------------------------------------------------------------
#
# Makefile
#     Makefile for keymgr
#
# IDENTIFICATION
#        src/gausskernel/security/keymgr/Makefile
#
# ---------------------------------------------------------------------------------------

subdir=src/gausskernel/security/keymgr
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

#/src/include/event/suse

ifneq "$(MAKECMDGOALS)" "clean"
  ifneq "$(MAKECMDGOALS)" "distclean"
    ifneq "$(shell which g++ |grep hutaf_llt |wc -l)" "1"
      -include $(DEPEND)
    endif
  endif
endif

FIELS = $(shell find $(top_builddir)/$(subdir) -name '*.cpp' | sort)
OBJS = $(FIELS:.cpp=.o)

include $(top_srcdir)/src/gausskernel/common.mk
