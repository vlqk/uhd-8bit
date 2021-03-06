========================================================================
UHD - Firmware and FPGA Image Application Notes
========================================================================

.. contents:: Table of Contents

------------------------------------------------------------------------
Images Overview
------------------------------------------------------------------------
Every USRP device must be loaded with special firmware and FPGA images.
The methods of loading images into the device varies among devices:

* **USRP1:** The host code will automatically load the firmware and FPGA at runtime.
* **USRP2:** The user must manually write the images onto the USRP2 SD card.

------------------------------------------------------------------------
Pre-built images
------------------------------------------------------------------------

Pre-built images are available for download.
See the UHD wiki for the download link.

The pre-built images come in platform-specific installer packages
and platform-independent archive files:

* **Linux:** DEB or RPM installer
* **Windows:** not available yet...
* **Macintosh:** not available yet...
* **Platform-independent:** ZIP or TAR.GZ archive

^^^^^^^^^^^^^^^^^^
Linux installers
^^^^^^^^^^^^^^^^^^
The Linux-based installers will install the images into /usr/share/uhd/images.
On a Linux system, the UHD will always search this path for image files.

Commands to install a linux rpm or deb package:

::

    sudo rpm -i <linux-images-installer>.rpm

    -- OR --

    sudo dpkg -i <linux-images-installer>.deb

^^^^^^^^^^^^^^^^^^^^^^
Archive install
^^^^^^^^^^^^^^^^^^^^^^
When installing images from an archive, there are two options:

**Option 1:**

Unpack the archive into the UHD installation prefix.
The UHD will always search <prefix>/share/uhd/images for image files.
Where <prefix> was set by the CMAKE_INSTALL_PREFIX at configure-time.

**Option 2:**

Unpack the archive anywhere and set the UHD_IMAGE_PATH environment variable.
The UHD_IMAGE_PATH may contain a list of directories to search for image files.

------------------------------------------------------------------------
Building images
------------------------------------------------------------------------

The UHD source repository comes with the source code necessary to build
both firmware and FPGA images for all supported devices.
The build commands for a particular image can be found in <uhd-repo-path>/images/Makefile.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Xilinx FPGA builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Xilinx ISE 12.x and up is required to build the Xilinx FPGA images.
The build requires that you have a unix-like environment with make.
Make sure that xtclsh from the Xilinx ISE bin directory is in your $PATH.

See <uhd-repo-path>/fpga/usrp2/top/*

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Microblaze firmware builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The Microblaze GCC compiler from the Xilinx EDK is required to build the Microblaze firmware images.
The build requires that you have a unix-like environment with autotools and make.
Make sure that mb-gcc from the Xilinx EDK/microblaze directory is in your $PATH.

See <uhd-repo-path>/firmware/microblaze

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Altera FPGA builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Quartus is required to build the Altera FPGA images.
Pre-built images can also be found in <uhd-repo-path>/fpga/usrp1/rbf

See <uhd-repo-path>/fpga/usrp1/toplevel/*

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
FX2 firmware builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The sdcc compiler is required to build the FX2 firmware images.
The build requires that you have a unix-like environment with autotools and make.

See <uhd-repo-path>/firmware/fx2
