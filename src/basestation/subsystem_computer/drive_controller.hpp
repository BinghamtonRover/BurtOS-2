#ifndef DRIVE_CONTROLLER
#define DRIVE_CONTROLLER

class DriveController {
    private:
    public:
        void halt();
        void setForwardVelocity(int mps); // In meters per second
        constexpr wheel_size = 15; // In cm 

}


#endif