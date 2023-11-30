##########################################################################
# 根目录下的Makefile
# Copyright (C) 2021-2022  Xu Ruijun
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

TOP_DIR = .

sinclude $(TOP_DIR)/tools/conf.mk

all: $(TARGET).hex flash

sinclude $(TOP_DIR)/tools/build.mk
sinclude $(TOP_DIR)/tools/mcu.mk


# clean output dir 'obj/*', keep symbol link in top dir
clean:
	@rm -f $(TARGET)*
	@for file in $(OBJODIR)/*; \
	do \
		if [ ! -L $$file ]; then \
			rm -rf $$file; \
		fi \
	done
