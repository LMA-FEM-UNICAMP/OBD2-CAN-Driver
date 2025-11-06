
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
    /* Deamon */
    pid_t pid, sid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS); // Parent exits

    // Create a new session
    sid = setsid();
    if (sid < 0)
        exit(EXIT_FAILURE);

    // Change working directory
    if ((chdir("/")) < 0)
        exit(EXIT_FAILURE);

    // Set file permissions
    umask(0);

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Open syslog for logging
    openlog("obd2_can_deamon", LOG_PID, LOG_DAEMON);

    /* Program */

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <string1> <string2>\n";
        return 1;
    }

    std::cout << "Starting obd2_can_driver" << std::endl;

    syslog(LOG_INFO, "Starting obd2_can_driver...");

    std::cout << "Using " << argv[1] << " as OBD2 CAN interface" << std::endl;
    std::cout << "Using " << argv[2] << " as OBU CAN interface" << std::endl;

    Obd2CanDriver obd2_can_driver(argv[1], argv[2]);

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