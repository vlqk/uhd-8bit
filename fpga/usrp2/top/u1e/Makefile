#
# Copyright 2008 Ettus Research LLC
#

##################################################
# Project Setup
##################################################
TOP_MODULE = u1e
BUILD_DIR = $(abspath build$(ISE))

##################################################
# Include other makefiles
##################################################

include ../Makefile.common
include ../../fifo/Makefile.srcs
include ../../control_lib/Makefile.srcs
include ../../sdr_lib/Makefile.srcs
include ../../serdes/Makefile.srcs
include ../../simple_gemac/Makefile.srcs
include ../../timing/Makefile.srcs
include ../../opencores/Makefile.srcs
include ../../vrt/Makefile.srcs
include ../../udp/Makefile.srcs
include ../../coregen/Makefile.srcs
include ../../extram/Makefile.srcs
include ../../gpmc/Makefile.srcs

##################################################
# Project Properties
##################################################
export PROJECT_PROPERTIES := \
family "Spartan-3A DSP" \
device xc3sd1800a \
package cs484 \
speed -4 \
top_level_module_type "HDL" \
synthesis_tool "XST (VHDL/Verilog)" \
simulator "ISE Simulator (VHDL/Verilog)" \
"Preferred Language" "Verilog" \
"Enable Message Filtering" FALSE \
"Display Incremental Messages" FALSE 

##################################################
# Sources
##################################################
TOP_SRCS = \
u1e_core.v \
u1e.v \
u1e.ucf \
timing.ucf

SOURCES = $(abspath $(TOP_SRCS)) $(FIFO_SRCS) \
$(CONTROL_LIB_SRCS) $(SDR_LIB_SRCS) $(SERDES_SRCS) \
$(SIMPLE_GEMAC_SRCS) $(TIMING_SRCS) $(OPENCORES_SRCS) \
$(VRT_SRCS) $(UDP_SRCS) $(COREGEN_SRCS) $(EXTRAM_SRCS) \
$(GPMC_SRCS)

##################################################
# Process Properties
##################################################
SYNTHESIZE_PROPERTIES = \
"Number of Clock Buffers" 8 \
"Pack I/O Registers into IOBs" Yes \
"Optimization Effort" High \
"Optimize Instantiated Primitives" TRUE \
"Register Balancing" Yes \
"Use Clock Enable" Auto \
"Use Synchronous Reset" Auto \
"Use Synchronous Set" Auto

TRANSLATE_PROPERTIES = \
"Macro Search Path" "$(shell pwd)/../../coregen/"

MAP_PROPERTIES = \
"Allow Logic Optimization Across Hierarchy" TRUE \
"Map to Input Functions" 4 \
"Optimization Strategy (Cover Mode)" Speed \
"Pack I/O Registers/Latches into IOBs" "For Inputs and Outputs" \
"Perform Timing-Driven Packing and Placement" TRUE \
"Map Effort Level" High \
"Extra Effort" Normal \
"Combinatorial Logic Optimization" TRUE \
"Register Duplication" TRUE

PLACE_ROUTE_PROPERTIES = \
"Place & Route Effort Level (Overall)" High 

STATIC_TIMING_PROPERTIES = \
"Number of Paths in Error/Verbose Report" 10 \
"Report Type" "Error Report"

GEN_PROG_FILE_PROPERTIES = \
"Configuration Rate" 6 \
"Create Binary Configuration File" TRUE \
"Done (Output Events)" 5 \
"Enable Bitstream Compression" TRUE \
"Enable Outputs (Output Events)" 6 \
"Unused IOB Pins" "Pull Up"

SIM_MODEL_PROPERTIES = ""
