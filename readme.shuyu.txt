https://interactive.freertos.org/hc/en-us/community/posts/115001124852-FreeRTOS-ports-to-Xtensa-on-github

FreeRTOS ports to Xtensa on github
Avatar	
Rabih Chrabieh
August 17, 2017 18:14	NONE	Follow
You may find the contributed kernel ports and demo/test suites for all Cadence Tensilica Xtensa processors at

https://github.com/tensilica/freertos/

https://github.com/tensilica/freertos/releases/tag/V9.0.0-xtensa

To install, extract the files onto the directory where you have already extracted FreeRTOS.  You can find the makefile and readme in the <FreeRTOS>/Demo/Xtensa_XCC directory.  Build it using xt-make (included in the command line tools in the SDK available from Cadence).  The build can be targeted to run on the simulator included in the SDK, or on several Xilinx FPGA boards.  For more information about Xtensa processors, including a link to request a trial SDK, go to http://ip.cadence.com/ipportfolio/tensilica-ip or contact your local Cadence sales representative.

The ports were developed by Nestwave (contact [_at_] nestwave.com).

Facebook Twitter LinkedIn Google+
0 comments
Please sign in to leave a comment.
