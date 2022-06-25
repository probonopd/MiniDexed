#
# Synth_Dexed.mk
#

CMSIS_CORE_INCLUDE_DIR = $(CMSIS_DIR)/Core/Include
CMSIS_DSP_INCLUDE_DIR = $(CMSIS_DIR)/DSP/Include
CMSIS_DSP_PRIVATE_INCLUDE_DIR = $(CMSIS_DIR)/DSP/PrivateInclude
CMSIS_DSP_COMPUTELIB_INCLUDE_DIR = $(CMSIS_DIR)/DSP/ComputeLibrary/Include
CMSIS_DSP_SOURCE_DIR = $(CMSIS_DIR)/DSP/Source
CMSIS_DSP_COMPUTELIB_SRC_DIR = $(CMSIS_DIR)/DSP/ComputeLibrary/Source

U8G2_OBJS += \
       $(U8G2_DIR)/cppsrc/U8g2lib.o \
       $(U8G2_DIR)/csrc/mui.o \
       $(U8G2_DIR)/csrc/mui_u8g2.o \
       $(U8G2_DIR)/csrc/u8g2_bitmap.o \
       $(U8G2_DIR)/csrc/u8g2_box.o \
       $(U8G2_DIR)/csrc/u8g2_buffer.o \
       $(U8G2_DIR)/csrc/u8g2_button.o \
       $(U8G2_DIR)/csrc/u8g2_circle.o \
       $(U8G2_DIR)/csrc/u8g2_cleardisplay.o \
       $(U8G2_DIR)/csrc/u8g2_d_memory.o \
       $(U8G2_DIR)/csrc/u8g2_d_setup.o \
       $(U8G2_DIR)/csrc/u8g2_font.o \
       $(U8G2_DIR)/csrc/u8g2_fonts.o \
       $(U8G2_DIR)/csrc/u8g2_hvline.o \
       $(U8G2_DIR)/csrc/u8g2_input_value.o \
       $(U8G2_DIR)/csrc/u8g2_intersection.o \
       $(U8G2_DIR)/csrc/u8g2_kerning.o \
       $(U8G2_DIR)/csrc/u8g2_line.o \
       $(U8G2_DIR)/csrc/u8g2_ll_hvline.o \
       $(U8G2_DIR)/csrc/u8g2_message.o \
       $(U8G2_DIR)/csrc/u8g2_polygon.o \
       $(U8G2_DIR)/csrc/u8g2_selection_list.o \
       $(U8G2_DIR)/csrc/u8g2_setup.o \
       $(U8G2_DIR)/csrc/u8log.o \
       $(U8G2_DIR)/csrc/u8log_u8g2.o \
       $(U8G2_DIR)/csrc/u8log_u8x8.o \
       $(U8G2_DIR)/csrc/u8x8_8x8.o \
       $(U8G2_DIR)/csrc/u8x8_byte.o \
       $(U8G2_DIR)/csrc/u8x8_cad.o \
       $(U8G2_DIR)/csrc/u8x8_capture.o \
       $(U8G2_DIR)/csrc/u8x8_d_a2printer.o \
       $(U8G2_DIR)/csrc/u8x8_d_gu800.o \
       $(U8G2_DIR)/csrc/u8x8_d_hd44102.o \
       $(U8G2_DIR)/csrc/u8x8_d_il3820_296x128.o \
       $(U8G2_DIR)/csrc/u8x8_d_ist3020.o \
       $(U8G2_DIR)/csrc/u8x8_d_ist7920.o \
       $(U8G2_DIR)/csrc/u8x8_d_ks0108.o \
       $(U8G2_DIR)/csrc/u8x8_d_lc7981.o \
       $(U8G2_DIR)/csrc/u8x8_d_ld7032_60x32.o \
       $(U8G2_DIR)/csrc/u8x8_d_ls013b7dh03.o \
       $(U8G2_DIR)/csrc/u8x8_d_max7219.o \
       $(U8G2_DIR)/csrc/u8x8_d_pcd8544_84x48.o \
       $(U8G2_DIR)/csrc/u8x8_d_pcf8812.o \
       $(U8G2_DIR)/csrc/u8x8_d_pcf8814_hx1230.o \
       $(U8G2_DIR)/csrc/u8x8_d_s1d15721.o \
       $(U8G2_DIR)/csrc/u8x8_d_s1d15e06.o \
       $(U8G2_DIR)/csrc/u8x8_d_sbn1661.o \
       $(U8G2_DIR)/csrc/u8x8_d_sed1330.o \
       $(U8G2_DIR)/csrc/u8x8_d_sh1106_64x32.o \
       $(U8G2_DIR)/csrc/u8x8_d_sh1106_72x40.o \
       $(U8G2_DIR)/csrc/u8x8_d_sh1107.o \
       $(U8G2_DIR)/csrc/u8x8_d_sh1108.o \
       $(U8G2_DIR)/csrc/u8x8_d_sh1122.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1305.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_128x32.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_128x64_noname.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_2040x16.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_48x64.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_64x32.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_64x48.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_72x40.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_96x16.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1306_96x40.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1309.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1316.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1317.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1318.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1320.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1322.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1325.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1326.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1327.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1329.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1606_172x72.o \
       $(U8G2_DIR)/csrc/u8x8_d_ssd1607_200x200.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7511.o \
       $(U8G2_DIR)/csrc/u8x8_d_st75160.o \
       $(U8G2_DIR)/csrc/u8x8_d_st75256.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7528.o \
       $(U8G2_DIR)/csrc/u8x8_d_st75320.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7565.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7567.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7571.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7586s_erc240160.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7586s_s028hn118a.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7586s_ymc240160.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7588.o \
       $(U8G2_DIR)/csrc/u8x8_d_st7920.o \
       $(U8G2_DIR)/csrc/u8x8_d_stdio.o \
       $(U8G2_DIR)/csrc/u8x8_d_t6963.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1601.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1604.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1608.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1609.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1610.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1611.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1617.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1638.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1701_dogs102.o \
       $(U8G2_DIR)/csrc/u8x8_d_uc1701_mini12864.o \
       $(U8G2_DIR)/csrc/u8x8_debounce.o \
       $(U8G2_DIR)/csrc/u8x8_display.o \
       $(U8G2_DIR)/csrc/u8x8_fonts.o \
       $(U8G2_DIR)/csrc/u8x8_gpio.o \
       $(U8G2_DIR)/csrc/u8x8_input_value.o \
       $(U8G2_DIR)/csrc/u8x8_message.o \
       $(U8G2_DIR)/csrc/u8x8_selection_list.o \
       $(U8G2_DIR)/csrc/u8x8_setup.o \
       $(U8G2_DIR)/csrc/u8x8_string.o \
       $(U8G2_DIR)/csrc/u8x8_u16toa.o \
       $(U8G2_DIR)/csrc/u8x8_u8toa.o

OBJS += \
       $(SYNTH_DEXED_DIR)/PluginFx.o \
       $(SYNTH_DEXED_DIR)/dexed.o \
       $(SYNTH_DEXED_DIR)/dx7note.o \
       $(SYNTH_DEXED_DIR)/env.o \
       $(SYNTH_DEXED_DIR)/exp2.o \
       $(SYNTH_DEXED_DIR)/fm_core.o \
       $(SYNTH_DEXED_DIR)/fm_op_kernel.o \
       $(SYNTH_DEXED_DIR)/freqlut.o \
       $(SYNTH_DEXED_DIR)/lfo.o \
       $(SYNTH_DEXED_DIR)/pitchenv.o \
       $(SYNTH_DEXED_DIR)/porta.o \
       $(SYNTH_DEXED_DIR)/sin.o \
       $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/SupportFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/BasicMathFunctions/BasicMathFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/FastMathFunctions/FastMathFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/FilteringFunctions/FilteringFunctions.o \
       $(CMSIS_DSP_SOURCE_DIR)/CommonTables/CommonTables.o \
       $(CMSIS_DSP_COMPUTELIB_SRC_DIR)/arm_cl_tables.o \
       $(U8G2_OBJS)

INCLUDE += -I $(SYNTH_DEXED_DIR)
INCLUDE += -I $(CMSIS_CORE_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_PRIVATE_INCLUDE_DIR)
INCLUDE += -I $(CMSIS_DSP_COMPUTELIB_INCLUDE_DIR)
INCLUDE += -I $(U8G2_DIR)/csrc
INCLUDE += -I $(U8G2_DIR)/cppsrc

DEFINE += -DUSE_FX -DU8X8_USE_PINS

ifeq ($(strip $(AARCH)),64)
DEFINE += -DARM_MATH_NEON
DEFINE += -DHAVE_NEON
endif

EXTRACLEAN = $(SYNTH_DEXED_DIR)/*.[od] $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/SupportFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/BasicMathFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/FastMathFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/FilteringFunctions/*.[od] $(CMSIS_DSP_SOURCE_DIR)/CommonTables/*.[od] $(CMSIS_DSP_COMPUTELIB_SRC_DIR)/*.[od] $(U8G2_DIR)/cppsrc/*.[od] $(U8G2_DIR)/csrc/*.[od]
