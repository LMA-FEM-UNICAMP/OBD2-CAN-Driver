
#include "obd2_can_driver.hpp"

int main(){

    Obd2CanDriver obd2_can_driver("can0", "vcan0");

    while(1){
        obd2_can_driver.read_obd2();
        obd2_can_driver.send_data();
    }

}