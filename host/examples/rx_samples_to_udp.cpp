//
// Copyright 2010-2011 Ettus Research LLC
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

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/single_usrp.hpp>
#include <uhd/transport/udp_simple.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <complex>

namespace po = boost::program_options;

int UHD_SAFE_MAIN(int argc, char *argv[]){
    uhd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;
    size_t total_num_samps;
    double rate, freq, gain;
    std::string addr, port;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single uhd device address args")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to receive")
        ("rate", po::value<double>(&rate)->default_value(100e6/16), "rate of incoming samples")
        ("freq", po::value<double>(&freq)->default_value(0), "rf center frequency in Hz")
        ("gain", po::value<double>(&gain)->default_value(0), "gain for the RF chain")
        ("port", po::value<std::string>(&port)->default_value("7124"), "server udp port")
        ("addr", po::value<std::string>(&addr)->default_value("192.168.1.10"), "resolvable server address")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("UHD RX to UDP %s") % desc << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::single_usrp::sptr sdev = uhd::usrp::single_usrp::make(args);
    uhd::device::sptr dev = sdev->get_device();
    std::cout << boost::format("Using Device: %s") % sdev->get_pp_string() << std::endl;

    //set the rx sample rate
    std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate/1e6) << std::endl;
    sdev->set_rx_rate(rate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (sdev->get_rx_rate()/1e6) << std::endl << std::endl;

    //set the rx center frequency
    std::cout << boost::format("Setting RX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    sdev->set_rx_freq(freq);
    std::cout << boost::format("Actual RX Freq: %f Mhz...") % (sdev->get_rx_freq()/1e6) << std::endl << std::endl;

    //set the rx rf gain
    std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
    sdev->set_rx_gain(gain);
    std::cout << boost::format("Actual RX Gain: %f dB...") % sdev->get_rx_gain() << std::endl << std::endl;

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time
    std::cout << "LO Locked = " << sdev->get_rx_lo_locked() << std::endl;

    //setup streaming
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps = total_num_samps;
    stream_cmd.stream_now = true;
    sdev->issue_stream_cmd(stream_cmd);

    //loop until total number of samples reached
    size_t num_acc_samps = 0; //number of accumulated samples
    uhd::rx_metadata_t md;
    std::vector<std::complex<float> > buff(dev->get_max_recv_samps_per_packet());
    uhd::transport::udp_simple::sptr udp_xport = uhd::transport::udp_simple::make_connected(addr, port);

    while(num_acc_samps < total_num_samps){
        size_t num_rx_samps = dev->recv(
            &buff.front(), buff.size(), md,
            uhd::io_type_t::COMPLEX_FLOAT32,
            uhd::device::RECV_MODE_ONE_PACKET
        );

        //handle the error codes
        switch(md.error_code){
        case uhd::rx_metadata_t::ERROR_CODE_NONE:
            break;

        case uhd::rx_metadata_t::ERROR_CODE_TIMEOUT:
            if (num_acc_samps == 0) continue;
            std::cout << boost::format(
                "Got timeout before all samples received, possible packet loss, exiting loop..."
            ) << std::endl;
            goto done_loop;

        default:
            std::cout << boost::format(
                "Got error code 0x%x, exiting loop..."
            ) % md.error_code << std::endl;
            goto done_loop;
        }

        //send complex single precision floating point samples over udp
        udp_xport->send(boost::asio::buffer(buff, num_rx_samps));

        num_acc_samps += num_rx_samps;
    } done_loop:

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return 0;
}
