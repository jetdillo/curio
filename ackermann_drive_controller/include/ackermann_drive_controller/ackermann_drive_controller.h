//
//  Software License Agreement (BSD-3-Clause)
//   
//  Copyright (c) 2019 Rhys Mainwaring
//  All rights reserved
//   
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1.  Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//  2.  Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//
//  3.  Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived
//      from this software without specific prior written permission.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
 
// Adapted from the original source code for diff_drive_controller
// and ackermann_steering_controller from the ros_controllers
// package: https://github.com/ros-controls/ros_controllers

/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, PAL Robotics, S.L.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the PAL Robotics nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/*
 * Author: Luca Marchionni
 * Author: Bence Magyar
 * Author: Enrique Fernández
 * Author: Paul Mathieu
 */

#ifndef ACKERMANN_DRIVE_CONTROLLER_ACKERMANN_DRIVE_CONTROLLER_H_
#define ACKERMANN_DRIVE_CONTROLLER_ACKERMANN_DRIVE_CONTROLLER_H_

#include "ackermann_drive_controller/AckermannDriveControllerConfig.h"
#include "ackermann_drive_controller/odometry.h"
#include "ackermann_drive_controller/speed_limiter.h"

#include <control_msgs/JointTrajectoryControllerState.h>
#include <controller_interface/controller.h>
#include <controller_interface/multi_interface_controller.h>
#include <dynamic_reconfigure/server.h>
#include <geometry_msgs/TwistStamped.h>
#include <hardware_interface/joint_command_interface.h>
#include <memory>
#include <nav_msgs/Odometry.h>
#include <pluginlib/class_list_macros.hpp>
#include <realtime_tools/realtime_buffer.h>
#include <realtime_tools/realtime_publisher.h>
#include <tf/tfMessage.h>

namespace ackermann_drive_controller
{
    class AckermannDriveController :
        public controller_interface::MultiInterfaceController<
            hardware_interface::PositionJointInterface,
            hardware_interface::VelocityJointInterface>
    {
    public:
        AckermannDriveController();

        bool init(hardware_interface::RobotHW* robot_hw,
            ros::NodeHandle& root_nh,
            ros::NodeHandle &controller_nh);

        void update(const ros::Time& time, const ros::Duration& period);

        void starting(const ros::Time& time);

        void stopping(const ros::Time& /*time*/);

    private:
        std::string name_;

        /// Odometry related:
        ros::Duration publish_period_;
        ros::Time last_state_publish_time_;
        bool open_loop_;

        /// Hardware handles:
        // std::vector<hardware_interface::JointHandle> left_wheel_joints_;
        // std::vector<hardware_interface::JointHandle> right_wheel_joints_;
        std::vector<hardware_interface::JointHandle> wheel_joints_;
        std::vector<hardware_interface::JointHandle> steer_joints_;

        // Previous time
        ros::Time time_previous_;

        /// Previous velocities from the encoders:
        // std::vector<double> vel_left_previous_;
        // std::vector<double> vel_right_previous_;
        std::vector<double> vel_previous_;
        std::vector<double> ang_previous_;

        /// Previous velocities from the encoders:
        // double vel_left_desired_previous_;
        // double vel_right_desired_previous_;
        double vel_desired_previous_;
        double ang_desired_previous_;

        /// Velocity command related:
        struct Commands
        {
            double lin;
            double ang;
            ros::Time stamp;

            Commands() : lin(0.0), ang(0.0), stamp(0.0) {}
        };
        realtime_tools::RealtimeBuffer<Commands> command_;
        Commands command_struct_;
        ros::Subscriber sub_command_;

        /// Publish executed commands
        std::shared_ptr<realtime_tools::RealtimePublisher<geometry_msgs::TwistStamped> > cmd_vel_pub_;

        /// Odometry related:
        std::shared_ptr<realtime_tools::RealtimePublisher<nav_msgs::Odometry> > odom_pub_;
        std::shared_ptr<realtime_tools::RealtimePublisher<tf::tfMessage> > tf_odom_pub_;
        Odometry odometry_;

        /// Controller state publisher
        std::shared_ptr<realtime_tools::RealtimePublisher<control_msgs::JointTrajectoryControllerState> > controller_state_pub_;

        /// Wheel separation, wrt the midpoint of the wheel width:
        // double wheel_separation_;

        /// Wheel radius (assuming it's the same for the left and right wheels):
        double wheel_radius_;
        double mid_wheel_lat_separation_;
        double front_wheel_lat_separation_; 
        double front_wheel_lon_separation_;
        double back_wheel_lat_separation_;
        double back_wheel_lon_separation_;

        struct Pos
        {
            Pos(double x, double y) : x(x), y(y) {}
            double x = 0.0;
            double y = 0.0;
        };
        // Joint geometric positions
        std::vector<Pos> wheel_positions_, steer_positions_;

        // Workspace for velocity and angle calculations
        std::vector<double> wheel_vel_, steer_ang_;

        /// Wheel separation and radius calibration multipliers:
        // double wheel_separation_multiplier_;
        // double left_wheel_radius_multiplier_;
        // double right_wheel_radius_multiplier_;

        /// Timeout to consider cmd_vel commands old:
        double cmd_vel_timeout_;

        /// Whether to allow multiple publishers on cmd_vel topic or not:
        bool allow_multiple_cmd_vel_publishers_;

        /// Frame to use for the robot base:
        std::string base_frame_id_;

        /// Frame to use for odometry and odom tf:
        std::string odom_frame_id_;

        /// Whether to publish odometry to tf or not:
        bool enable_odom_tf_;

        /// Number of wheel joints:
        size_t wheel_joints_size_;

        /// Number of steer joints:
        size_t steer_joints_size_;
        
        /// Speed limiters:
        Commands last1_cmd_;
        Commands last0_cmd_;
        SpeedLimiter limiter_lin_;
        SpeedLimiter limiter_ang_;

        /// Publish limited velocity:
        bool publish_cmd_;

        /// Publish wheel data:
        bool publish_wheel_joint_controller_state_;    

        // @TODO: enable dynamic reconfig
        // A struct to hold dynamic parameters
        // set from dynamic_reconfigure server
        /*
        struct DynamicParams
        {
            bool update;

            double left_wheel_radius_multiplier;
            double right_wheel_radius_multiplier;
            double wheel_separation_multiplier;

            bool publish_cmd;

            double publish_rate;
            bool enable_odom_tf;

            DynamicParams() :
                left_wheel_radius_multiplier(1.0),
                right_wheel_radius_multiplier(1.0),
                wheel_separation_multiplier(1.0),
                publish_cmd(false),
                publish_rate(50),
                enable_odom_tf(true)
            {}

            friend std::ostream& operator<<(std::ostream& os, const DynamicParams& params)
            {
                os << "DynamicParams:\n"
                //
                << "\tOdometry parameters:\n"
                << "\t\tleft wheel radius multiplier: "   << params.left_wheel_radius_multiplier  << "\n"
                << "\t\tright wheel radius multiplier: "  << params.right_wheel_radius_multiplier << "\n"
                << "\t\twheel separation multiplier: "    << params.wheel_separation_multiplier   << "\n"
                //
                << "\tPublication parameters:\n"
                << "\t\tPublish executed velocity command: " << (params.publish_cmd?"enabled":"disabled") << "\n"
                << "\t\tPublication rate: " << params.publish_rate                 << "\n"
                << "\t\tPublish frame odom on tf: " << (params.enable_odom_tf?"enabled":"disabled");

                return os;
            }
        };

        realtime_tools::RealtimeBuffer<DynamicParams> dynamic_params_;

        /// Dynamic Reconfigure server
        typedef dynamic_reconfigure::Server<AckermannDriveControllerConfig> ReconfigureServer;
        
        std::shared_ptr<ReconfigureServer> dyn_reconf_server_;
        */

    private:

        void brake();

        void cmdVelCallback(const geometry_msgs::Twist& command);

        // @TOOD: probably not required unless we extend to more than 6 wheels...
        /*
        bool getWheelNames(ros::NodeHandle& controller_nh,
            const std::string& wheel_param,
            std::vector<std::string>& wheel_names);
        */

        // @TODO: enable setting params from URDF
        /*
        bool setOdomParamsFromUrdf(ros::NodeHandle& root_nh,
            const std::string& left_wheel_name,
            const std::string& right_wheel_name,
            bool lookup_wheel_separation,
            bool lookup_wheel_radius);
        */

        void setOdomPubFields(ros::NodeHandle& root_nh, ros::NodeHandle& controller_nh);

        void reconfCallback(AckermannDriveControllerConfig& config, uint32_t /*level*/);

        // @TODO: enable dynamic reconfig
        /*
        void updateDynamicParams();
        */

        // @TODO: enable publishing wheel and steer joint info.    
        /*
        void publishWheelData(const ros::Time& time,
            const ros::Duration& period,
            Commands& curr_cmd,
            double wheel_separation,
            double left_wheel_radius,
            double right_wheel_radius);
        */
    };

    PLUGINLIB_EXPORT_CLASS(ackermann_drive_controller::AckermannDriveController, controller_interface::ControllerBase);

} // namespace ackermann_drive_controller

#endif // ACKERMANN_DRIVE_CONTROLLER_ACKERMANN_DRIVE_CONTROLLER_H_
