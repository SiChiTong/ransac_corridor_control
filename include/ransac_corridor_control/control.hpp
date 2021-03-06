#ifndef RANSAC_CONTROL_H
#define RANSAC_CONTROL_H

#include <ros/ros.h>

#include "control_piv.hpp"
#include "ransac_corridor_control/CarCommandStamped.h"
#include "ransac_corridor_control/LineCoeffs3Stamped.h"


class Control{
private:
    static Control *instance;

    double linear_vel, angular_vel, dt;
    ros::Time start_time;

    /* parameters defined by the user */
    double kpt, kit, krt, kvt;
protected:
    Control( const float &_kpt
            ,const float &_kit
            ,const float &_krt
            ,const float &_kvt
            ,const float &_linear_vel
            ,const ros::Publisher &_cmd_vel_pub
           ):
             kpt(_kpt)
            ,kit(_kit)
            ,krt(_krt)
            ,kvt(_kvt)
            ,linear_vel(_linear_vel)
            ,cmd_vel_pub(_cmd_vel_pub)
    {
    angular_vel = 0.0;
    bool print = true;
    while(!ros::Time::now().toSec()){
       if(print){
          print = false;
          ROS_INFO("Control node --> waiting for time");
       }
    }
    ROS_INFO("Control node --> time received");
    start_time = ros::Time::now();
    dt = start_time.toSec();
    }
    Control (const Control& other) {}
    Control &operator= (const Control& other) {}
public:
    ros::Publisher cmd_vel_pub;

    static Control* unique_instance();
    static Control* unique_instance( const float &_kpt
                                    ,const float &_kit
                                    ,const float &_krt
                                    ,const float &_kvt
                                    ,const float &_linear_vel
                                    ,const ros::Publisher &_cmd_vel_pub
                                   );

    void ransac_callback(const ransac_corridor_control::LineCoeffs3Stamped &bisector_line_msg);
};
#endif /* RANSAC_CONTROL_H */
