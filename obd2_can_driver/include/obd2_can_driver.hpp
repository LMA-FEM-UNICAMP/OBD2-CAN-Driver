
#include <iostream>

#include <string>
#include <cstring>
#include <vector>
#include <cstdio>

#include <atomic>

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/un.h>

/* OBD2 PIDs */

#define GET_AVAILABLE_PIDS_PID 0x00
#define ENGINE_SPEED_PID 0x0C
#define VEHICLE_SPEED_PID 0x0D
#define THROTTLE_PEDAL_POSITION_PID 0x11

#define CANVSTATE_CANID 0x123
#define REQUEST_DELAY 20

static const char *LOG_SOCK_PATH = "/tmp/obd2_can_logging.sock";

typedef struct can_frame can_frame_t;

class Obd2CanDriver
{
public:
    Obd2CanDriver(std::string, std::string);
    ~Obd2CanDriver();

    bool read_obd2();
    bool read_obd2_j1939();

    bool send_data_to_can_out();
    bool send_data_to_can_out(can_frame_t);

    can_frame_t obd2_response();
    can_frame_t obd2_response_j1939();
    bool obd2_request(uint8_t);
    bool obd2_request_j1939();
    void obd2_requester();
    void configure_requester(bool);

    void obd2_logging(const char*);

    void add_pid(uint8_t);

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
    std::atomic<bool> requesting_;

    double longitudinal_speed_;
    double engine_rpm_;
    double throttle_position_;

    /* Logging */

    int sockfd_log_;
    struct sockaddr_un addr_log_;
    int client_log_;
};