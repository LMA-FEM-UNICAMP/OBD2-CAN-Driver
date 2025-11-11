
#include "obd2_can_driver.hpp"

#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <string1> <string2>\n";
        return 1;
    }

    std::cout << "Starting obd2_can_driver" << std::endl;

    std::cout << "Using " << argv[1] << " as OBD2 CAN interface" << std::endl;

    std::cout << "Using " << argv[2] << " as OBU CAN interface" << std::endl;

    Obd2CanDriver obd2_can_driver(argv[1], argv[2]);

    obd2_can_driver.add_pid(THROTTLE_PEDAL_POSITION_PID);
    obd2_can_driver.add_pid(ENGINE_SPEED_PID);

    std::thread obd2_requester_trd(&Obd2CanDriver::obd2_requester, &obd2_can_driver);

    while (1)
    {
        bool success = obd2_can_driver.read_obd2();
        if (success)
        {
            obd2_can_driver.send_data_to_can_out();
        }
    }

    obd2_requester_trd.join();

    closelog();

    return EXIT_SUCCESS;
}