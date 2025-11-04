
#include <iostream>

#include <string>
#include <cstring>
#include <vector>

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define N_ATTEMPTS 5

typedef struct can_frame can_frame_t;

class Obd2CanDriver
{
public:

    Obd2CanDriver(std::string, std::string);
    ~Obd2CanDriver();

    bool read_obd2();

    bool send_data();

    can_frame_t obd2_request(uint8_t);

private:
    std::string can_in_;
    std::string can_out_;

    int socket_in_;
    struct sockaddr_can addr_in_;
    struct ifreq ifr_in_;

    int socket_out_;
    struct sockaddr_can addr_out_;
    struct ifreq ifr_out_;

    std::vector<uint8_t> pids_;

    bool is_new_data_;

    double longitudinal_speed_;
    double engine_rpm_;
    double throttle_position_;
};