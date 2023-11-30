#############################################################################
# MCU操作
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
#  GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#############################################################################

openocd_erase:
	@$(OPENOCD) $(OCDFLAGS) -c init -c halt -c "flash erase_sector wch_riscv 0 last" -c exit

openocd_prog: $(TARGET).hex
	@$(OPENOCD) $(OCDFLAGS) -c init -c halt -c "program \"$<\" 0x08000000" -c exit

openocd_verify: $(TARGET).hex
	@$(OPENOCD) $(OCDFLAGS) -c init -c halt -c "verify_image \"$<\"" -c exit

openocd_reset:
	@$(OPENOCD) $(OCDFLAGS) -c init -c reset -c exit

openocd_flash: openocd_erase openocd_prog openocd_verify openocd_reset


wchisp_erase:
	@$(WCHISP) erase

wchisp_verify: $(TARGET).hex
	@$(WCHISP) verify "$<"

wchisp_reset:
	@$(WCHISP) reset

wchisp_flash: $(TARGET).hex
	@$(WCHISP) flash "$<"


wlink_erase:
	@$(WLINK) $(WLINKFLAGS) erase

wlink_reset:
	@$(WLINK) $(WLINKFLAGS) reset

wlink_flash: $(TARGET).hex
	@$(WLINK) $(WLINKFLAGS) flash "$<"

erase: $(MCU_OP)_erase
verify: $(MCU_OP)_verify
reset: $(MCU_OP)_reset
flash: $(MCU_OP)_flash
