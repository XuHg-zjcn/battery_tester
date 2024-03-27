##########################################################################
# 编译和链接程序
# Copyright (C) 2022-2023  Xu Ruijun
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
##########################################################################

sinclude $(TOOL_DIR)/inc.mk
sinclude $(TOOL_DIR)/src.mk


CSRC = $(shell find $(SDIR) -name '*.c' | awk -F '_' '{if($$(NF)!="template.c")print $$0}')
CXXSRC = $(shell find $(SDIR) -name '*.cpp')
ASRC = $(shell find $(SDIR) -name '*.S')

OBJF = $(CSRC:%.c=obj/%.o) \
       $(CXXSRC:%.cpp=obj/%.o) \
       $(ASRC:%.S=obj/%.o)


obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(INCS) -c -o "$@" "$<"

obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCS) -c -o "$@" "$<"

obj/%.o: %.S
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(INCS) -c -o "$@" "$<"

$(TARGET).elf: $(OBJF)
	$(CXX) $(LDFLAGS) -o "$@" $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex "$<" "$@"
