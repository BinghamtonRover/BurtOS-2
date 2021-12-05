#include <iostream>  
using namespace std;

class DriveController {
    private:
    public:
        float wheel_size = 15; // In cm 

        // accelerations for each motor
        float left_motor_1 = 0;
        float left_motor_2 = 0;
        float left_motor_3 = 0;

        float right_motor_1 = 0;
        float right_motor_2 = 0;
        float right_motor_3 = 0;

        float current_angle = 0;
        float current_acceleration = 0;

        float current_velocity = 0;
        float target_velocity = 0;

        // max acc = max acc for a specific motor 
        float MAX_ACCELERATION = 15;
        // max velocity for rover 
        float MAX_VELOCITY = 2000;

        void halt() {
            left_motor_1 = 0;
            left_motor_2 = 0;
            left_motor_3 = 0;

            right_motor_1 = 0;
            right_motor_2 = 0;
            right_motor_3 = 0;
        }

        // Uses globals current_angle, current_acceleration to get motor accs
        void update_motor_acceleration() {

            float right_scaling = (90 + current_angle) / 180;
            int sign_acceleration = (current_acceleration > 0) - (current_acceleration < 0);

            current_acceleration = target_velocity - current_velocity;
            if (abs(max(right_scaling, 1 - right_scaling) * current_acceleration) > MAX_ACCELERATION) {
                current_acceleration = sign_acceleration * MAX_ACCELERATION / max(right_scaling, 1 - right_scaling);
            }

            if(current_acceleration != 0) {
                current_velocity += current_acceleration;
                // ideally the motor accelerations produce the target acceleration in current_acceleration 
                left_motor_1 = sign_acceleration * min(abs((1 - right_scaling) * current_acceleration), MAX_ACCELERATION);
                left_motor_2 = sign_acceleration * min(abs((1 - right_scaling) * current_acceleration), MAX_ACCELERATION);
                left_motor_3 = sign_acceleration * min(abs((1 - right_scaling) * current_acceleration), MAX_ACCELERATION);
                
                right_motor_1 = sign_acceleration * min(abs(right_scaling * current_acceleration), MAX_ACCELERATION);
                right_motor_2 = sign_acceleration * min(abs(right_scaling * current_acceleration), MAX_ACCELERATION);
                right_motor_3 = sign_acceleration * min(abs(right_scaling * current_acceleration), MAX_ACCELERATION);
            }

            // change acceleration



            // if angle > 0 and it's at max acc, set curr acceleration/scaling to max acceleration
        }

        void setForwardVelocity(float mps) {
            // convert meters/second into revolutions per second
            target_velocity = mps / (2 * 3.14159 * wheel_size / 100);
        } 
        
        
        // -90 = sharp left, 0 = straight, 90 = sharp right
        void setSteeringAngle(int8_t angle) { 
            current_angle = angle;
        }

};


int main() {
    DriveController d = DriveController();
    d.setForwardVelocity(10);
    d.setSteeringAngle(45);
    // d.update_motor_acceleration();
    cout << "Wheel size: " << d.wheel_size << endl;
    cout << "Steering angle: " << d.current_angle << endl;
    cout << "Left motor acc: " << d.left_motor_1 << " " << d.left_motor_2 << " " << d.left_motor_3 << endl;
    cout << "Right motor acc: " << d.right_motor_1 << " " << d.right_motor_2 << " " << d.right_motor_3 << endl;

    DriveController negative_acc = DriveController();
    
    negative_acc.setForwardVelocity(-100);
    negative_acc.setSteeringAngle(45);
    cout << "Wheel size: " << negative_acc.wheel_size << endl;
    cout << "Steering angle: " << negative_acc.current_angle << endl;
    cout << "Left motor acc: " << negative_acc.left_motor_1 << " " << negative_acc.left_motor_2 << " " << negative_acc.left_motor_3 << endl;
    cout << "Right motor acc: " << negative_acc.right_motor_1 << " " << negative_acc.right_motor_2 << " " << negative_acc.right_motor_3 << endl;

    for(int i=0;i<10;i++) {
        d.update_motor_acceleration();
        cout << "Left motor acc: " << d.left_motor_1 << " " << d.left_motor_2 << " " << d.left_motor_3 << endl;
        cout << "Right motor acc: " << d.right_motor_1 << " " << d.right_motor_2 << " " << d.right_motor_3 << endl;
    }
   return 0;
}