
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
    char string_buf[128];

    // Set file permissions
    umask(0);

    // Open syslog for logging
    openlog("obd2_can_daemon", LOG_PID, LOG_DAEMON);

    /* Program */

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <string1> <string2>\n";
        return 1;
    }

    syslog(LOG_INFO, "Starting obd2_can_driver...");

    snprintf(string_buf, sizeof(string_buf), "Using %s as OBD2 CAN interface", argv[1]);
    syslog(LOG_INFO, "%s", string_buf);

    snprintf(string_buf, sizeof(string_buf), "Using %s as OBU CAN interface", argv[2]);
    syslog(LOG_INFO, "%s", string_buf);

    Obd2CanDriver obd2_can_driver(argv[1], argv[2]);

    std::thread obd2_requester_trd(&Obd2CanDriver::obd2_requester, &obd2_can_driver);

    while (1)
    {
        bool success = obd2_can_driver.read_obd2();
        if (success)
        {
            obd2_can_driver.send_data_to_can_out();
        }
    
        snprintf(string_buf, sizeof(string_buf), "Reading CAN bus...");
        syslog(LOG_INFO, "%s", string_buf);
    }

    obd2_requester_trd.join();

    closelog();

    return EXIT_SUCCESS;
}