//
// Copyright 2010 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "clock_ctrl.hpp"
#include "ad9522_regs.hpp"
#include <uhd/utils/assert.hpp>
#include <boost/cstdint.hpp>
#include "usrp_e100_regs.hpp" //spi slave constants
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <utility>
#include <iostream>

using namespace uhd;

template <typename div_type, typename bypass_type> static void set_clock_divider(
    size_t divider, div_type &low, div_type &high, bypass_type &bypass
){
    high = divider/2 - 1;
    low = divider - high - 2;
    bypass = (divider == 1)? 1 : 0;
}

/***********************************************************************
 * Constants
 **********************************************************************/
static const bool enable_test_clock = false;
static const size_t ref_clock_doubler = 2; //enabled below
static const double ref_clock_rate = 10e6 * ref_clock_doubler;

static const size_t r_counter = 1;
static const size_t a_counter = 0;
static const size_t b_counter = 20 / ref_clock_doubler;
static const size_t prescaler = 8; //set below with enum, set to 8 when input is under 2400 MHz
static const size_t vco_divider = 5; //set below with enum

static const size_t n_counter = prescaler * b_counter + a_counter;
static const size_t vco_clock_rate = ref_clock_rate/r_counter * n_counter; //between 1400 and 1800 MHz
static const double master_clock_rate = vco_clock_rate/vco_divider;

static const size_t fpga_clock_divider = size_t(master_clock_rate/64e6);
static const size_t codec_clock_divider = size_t(master_clock_rate/64e6);

/***********************************************************************
 * Clock Control Implementation
 **********************************************************************/
class usrp_e100_clock_ctrl_impl : public usrp_e100_clock_ctrl{
public:
    usrp_e100_clock_ctrl_impl(usrp_e100_iface::sptr iface){
        _iface = iface;

        //init the clock gen registers
        //Note: out0 should already be clocking the FPGA or this isnt going to work
        _ad9522_regs.sdo_active = ad9522_regs_t::SDO_ACTIVE_SDO_SDIO;
        _ad9522_regs.enable_clock_doubler = 1; //enable ref clock doubler
        _ad9522_regs.enb_stat_eeprom_at_stat_pin = 0; //use status pin
        _ad9522_regs.status_pin_control = 0x1; //n divider
        _ad9522_regs.ld_pin_control = 0x00; //dld
        _ad9522_regs.refmon_pin_control = 0x12; //show ref2

        _ad9522_regs.enable_ref2 = 1;
        _ad9522_regs.enable_ref1 = 0;
        _ad9522_regs.select_ref = ad9522_regs_t::SELECT_REF_REF2;

        _ad9522_regs.set_r_counter(r_counter);
        _ad9522_regs.a_counter = a_counter;
        _ad9522_regs.set_b_counter(b_counter);
        _ad9522_regs.prescaler_p = ad9522_regs_t::PRESCALER_P_DIV8_9;

        _ad9522_regs.pll_power_down = ad9522_regs_t::PLL_POWER_DOWN_NORMAL;
        _ad9522_regs.cp_current = ad9522_regs_t::CP_CURRENT_1_2MA;

        _ad9522_regs.vco_calibration_now = 1; //calibrate it!
        _ad9522_regs.vco_divider = ad9522_regs_t::VCO_DIVIDER_DIV5;
        _ad9522_regs.select_vco_or_clock = ad9522_regs_t::SELECT_VCO_OR_CLOCK_VCO;

        //setup fpga master clock
        _ad9522_regs.out0_format = ad9522_regs_t::OUT0_FORMAT_LVDS;
        set_clock_divider(fpga_clock_divider,
            _ad9522_regs.divider0_low_cycles,
            _ad9522_regs.divider0_high_cycles,
            _ad9522_regs.divider0_bypass
        );

        //setup codec clock
        _ad9522_regs.out3_format = ad9522_regs_t::OUT3_FORMAT_LVDS;
        set_clock_divider(codec_clock_divider,
            _ad9522_regs.divider1_low_cycles,
            _ad9522_regs.divider1_high_cycles,
            _ad9522_regs.divider1_bypass
        );

        //setup test clock (same divider as codec clock)
        _ad9522_regs.out4_format = ad9522_regs_t::OUT4_FORMAT_CMOS;
        _ad9522_regs.out4_cmos_configuration = (enable_test_clock)?
            ad9522_regs_t::OUT4_CMOS_CONFIGURATION_A_ON :
            ad9522_regs_t::OUT4_CMOS_CONFIGURATION_OFF;

        //setup a list of register ranges to write
        typedef std::pair<boost::uint16_t, boost::uint16_t> range_t;
        static const std::vector<range_t> ranges = boost::assign::list_of
            (range_t(0x000, 0x000)) (range_t(0x010, 0x01F))
            (range_t(0x0F0, 0x0FD)) (range_t(0x190, 0x19B))
            (range_t(0x1E0, 0x1E1)) (range_t(0x230, 0x230))
        ;

        //write initial register values and latch/update
        BOOST_FOREACH(const range_t &range, ranges){
            for(boost::uint16_t addr = range.first; addr <= range.second; addr++){
                this->send_reg(addr);
            }
        }
        this->latch_regs();
        //test read:
        //boost::uint32_t reg = _ad9522_regs.get_read_reg(0x01b);
        //boost::uint32_t result = _iface->transact_spi(
        //    UE_SPI_SS_AD9522,
        //    spi_config_t::EDGE_RISE,
        //    reg, 24, true /*no*/
        //);
        //std::cout << "result " << std::hex << result << std::endl;
        this->enable_rx_dboard_clock(false);
        this->enable_tx_dboard_clock(false);
    }

    ~usrp_e100_clock_ctrl_impl(void){
        this->enable_rx_dboard_clock(false);
        this->enable_tx_dboard_clock(false);
    }

    double get_fpga_clock_rate(void){
        return master_clock_rate/fpga_clock_divider;
    }

    /***********************************************************************
     * RX Dboard Clock Control (output 9, divider 3)
     **********************************************************************/
    void enable_rx_dboard_clock(bool enb){
        _ad9522_regs.out9_format = ad9522_regs_t::OUT9_FORMAT_CMOS;
        _ad9522_regs.out9_cmos_configuration = (enb)?
            ad9522_regs_t::OUT9_CMOS_CONFIGURATION_B_ON :
            ad9522_regs_t::OUT9_CMOS_CONFIGURATION_OFF;
        this->send_reg(0x0F9);
        this->latch_regs();
    }

    std::vector<double> get_rx_dboard_clock_rates(void){
        std::vector<double> rates;
        for(size_t div = 1; div <= 16+16; div++)
            rates.push_back(master_clock_rate/div);
        return rates;
    }

    void set_rx_dboard_clock_rate(double rate){
        assert_has(get_rx_dboard_clock_rates(), rate, "rx dboard clock rate");
        size_t divider = size_t(master_clock_rate/rate);
        //set the divider registers
        set_clock_divider(divider,
            _ad9522_regs.divider3_low_cycles,
            _ad9522_regs.divider3_high_cycles,
            _ad9522_regs.divider3_bypass
        );
        this->send_reg(0x199);
        this->send_reg(0x19a);
        this->latch_regs();
    }

    /***********************************************************************
     * TX Dboard Clock Control (output 6, divider 2)
     **********************************************************************/
    void enable_tx_dboard_clock(bool enb){
        _ad9522_regs.out6_format = ad9522_regs_t::OUT6_FORMAT_CMOS;
        _ad9522_regs.out6_cmos_configuration = (enb)?
            ad9522_regs_t::OUT6_CMOS_CONFIGURATION_B_ON :
            ad9522_regs_t::OUT6_CMOS_CONFIGURATION_OFF;
        this->send_reg(0x0F6);
        this->latch_regs();
    }

    std::vector<double> get_tx_dboard_clock_rates(void){
        return get_rx_dboard_clock_rates(); //same master clock, same dividers...
    }

    void set_tx_dboard_clock_rate(double rate){
        assert_has(get_tx_dboard_clock_rates(), rate, "tx dboard clock rate");
        size_t divider = size_t(master_clock_rate/rate);
        //set the divider registers
        set_clock_divider(divider,
            _ad9522_regs.divider2_low_cycles,
            _ad9522_regs.divider2_high_cycles,
            _ad9522_regs.divider2_bypass
        );
        this->send_reg(0x196);
        this->send_reg(0x197);
        this->latch_regs();
    }
    
    /***********************************************************************
     * Clock reference control
     **********************************************************************/
    void use_internal_ref(void) {
        _ad9522_regs.enable_ref2 = 1;
        _ad9522_regs.enable_ref1 = 0;
        _ad9522_regs.select_ref = ad9522_regs_t::SELECT_REF_REF2;
        _ad9522_regs.enb_auto_ref_switchover = ad9522_regs_t::ENB_AUTO_REF_SWITCHOVER_MANUAL;
        this->send_reg(0x01C);
    }
    
    void use_external_ref(void) {
        _ad9522_regs.enable_ref2 = 0;
        _ad9522_regs.enable_ref1 = 1;
        _ad9522_regs.select_ref = ad9522_regs_t::SELECT_REF_REF1;
        _ad9522_regs.enb_auto_ref_switchover = ad9522_regs_t::ENB_AUTO_REF_SWITCHOVER_MANUAL;
        this->send_reg(0x01C);
    }
    
    void use_auto_ref(void) {
        _ad9522_regs.enable_ref2 = 1;
        _ad9522_regs.enable_ref1 = 1;
        _ad9522_regs.select_ref = ad9522_regs_t::SELECT_REF_REF1;
        _ad9522_regs.enb_auto_ref_switchover = ad9522_regs_t::ENB_AUTO_REF_SWITCHOVER_AUTO;
    }

private:
    usrp_e100_iface::sptr _iface;
    ad9522_regs_t _ad9522_regs;

    void latch_regs(void){
        _ad9522_regs.io_update = 1;
        this->send_reg(0x232);
    }

    void send_reg(boost::uint16_t addr){
        boost::uint32_t reg = _ad9522_regs.get_write_reg(addr);
        //std::cout << "clock control write reg: " << std::hex << reg << std::endl;
        _iface->transact_spi(
            UE_SPI_SS_AD9522,
            spi_config_t::EDGE_RISE,
            reg, 24, false /*no rb*/
        );
    }
};

/***********************************************************************
 * Clock Control Make
 **********************************************************************/
usrp_e100_clock_ctrl::sptr usrp_e100_clock_ctrl::make(usrp_e100_iface::sptr iface){
    return sptr(new usrp_e100_clock_ctrl_impl(iface));
}
