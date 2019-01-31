﻿/*******************************************************************************
* Copyright 2018 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/
#include "open_manipulator_motion/open_manipulator_motion.h"

using namespace open_manipulator_motion;
using namespace std;

OpenManipulatorMotion::OpenManipulatorMotion()
  :node_handle_("")
{
  initPublisher();
  initSubscriber();
  initClient();
}

OpenManipulatorMotion::~OpenManipulatorMotion()
{
  if(ros::isStarted()) {
    ros::shutdown(); // explicitly needed since we use ros::start();
    ros::waitForShutdown();
  }
  wait();
}

void OpenManipulatorMotion::initPublisher()
{
  open_manipulator_motion_state_pub_ = node_handle_.advertise<open_manipulator_motion::MotionState>("motion_state", 10);
}

void OpenManipulatorMotion::initSubscriber()
{
  open_manipulator_kinematics_pose_sub_ = node_handle_.subscribe("kinematics_pose", 10, &OpenManipulatorMotion::kinematicsPoseCallback, this);
  open_manipulator_gui_button_sub_ = node_handle_.subscribe("button_clicked", 10, &OpenManipulatorMotion::motionStatesCallback, this);
  open_manipulator_ar_marker_sub_ = node_handle_.subscribe("/ar_pose_marker", 10, &OpenManipulatorMotion::markerPosCallback, this);
}

void OpenManipulatorMotion::initClient()
{
  goal_joint_space_path_to_kinematics_pose_client_ = node_handle_.serviceClient<open_manipulator_msgs::SetKinematicsPose>("goal_joint_space_path_to_kinematics_pose");
}

void OpenManipulatorMotion::kinematicsPoseCallback(const open_manipulator_msgs::KinematicsPose::ConstPtr &msg)
{
  Eigen::Quaterniond temp_orientation(msg->pose.orientation.w, msg->pose.orientation.x, msg->pose.orientation.y, msg->pose.orientation.z);
  kinematics_orientation_rpy_ = robotis_manipulator_math::convertQuaternion2RPYVector(temp_orientation);

  kinematics_pose_.pose = msg->pose;
}

void OpenManipulatorMotion::motionStatesCallback(const std_msgs::Bool::ConstPtr &msg)
{
  motion_flag = msg->data;
}

void OpenManipulatorMotion::markerPosCallback(const ar_track_alvar_msgs::AlvarMarkers::ConstPtr &msg)
{
  Eigen::Vector3d origin_orientation;
  Eigen::Vector3d rotation_orientation;
  Eigen::Matrix3d rotation_matrix;
  std::vector<double> object_position;
  std::vector<double> kinematics_pose;

  rotation_matrix = robotis_manipulator_math::matrix3(-1,0,0, 0,1,0, 0,0,1);

  if(!(msg->markers.empty()))
  {
    Eigen::Quaterniond temp_orientation(msg->markers.at(0).pose.pose.orientation.w, msg->markers.at(0).pose.pose.orientation.x, msg->markers.at(0).pose.pose.orientation.y, msg->markers.at(0).pose.pose.orientation.z);

    rotation_orientation = rotation_matrix * robotis_manipulator_math::convertQuaternion2RPYVector(temp_orientation);
    origin_orientation = robotis_manipulator_math::convertQuaternion2RPYVector(temp_orientation);

    cout << "r_r: " << kinematics_orientation_rpy_.coeff(0,0) << endl;
    cout << "p_r: " << kinematics_orientation_rpy_.coeff(1,0) << endl;
    cout << "y_r: " << kinematics_orientation_rpy_.coeff(2,0) << endl << endl;

    cout << "r_m: " << rotation_orientation.coeff(0,0) << endl;
    cout << "p_m: " << rotation_orientation.coeff(1,0) << endl;
    cout << "y_m: " << rotation_orientation.coeff(2,0) << endl << endl;



    //quaternionToEulerN();
  }

//  if(!(msg->markers.empty()))
//  {
//    object_position.push_back(msg->markers.at(0).pose.pose.position.x);
//    object_position.push_back(msg->markers.at(0).pose.pose.position.y);
//    object_position.push_back(msg->markers.at(0).pose.pose.position.z);

//    Eigen::Quaterniond temp_orientation(msg->markers.at(0).pose.pose.orientation.w, msg->markers.at(0).pose.pose.orientation.x, msg->markers.at(0).pose.pose.orientation.y, msg->markers.at(0).pose.pose.orientation.z);
//    rotation_orientation = robotis_manipulator_math::matrix3(0,-1,0, -1,0,0, 0,0,1)*robotis_manipulator_math::convertQuaternion2RPYVector(temp_orientation);
//    object_orientation = robotis_manipulator_math::convertRPY2Quaternion(rotation_orientation.coeffRef(0,0), rotation_orientation.coeffRef(1,0), rotation_orientation.coeffRef(2,0));

//    cout << "R: " << rotation_orientation.coeffRef(0,0) << endl;
//    cout << "P: " << rotation_orientation.coeffRef(1,0) << endl;
//    cout << "Y: " << rotation_orientation.coeffRef(2,0) << endl << endl;

//    cout << "r: " << kinematics_orientation_rpy_.coeffRef(0,0) << endl;
//    cout << "p: " << kinematics_orientation_rpy_.coeffRef(1,0) << endl;
//    cout << "y: " << kinematics_orientation_rpy_.coeffRef(2,0) << endl << endl;


//    if(motion_flag)
//    {
//      kinematics_pose.push_back(object_position.at(0));
//      kinematics_pose.push_back(object_position.at(1));
//      kinematics_pose.push_back(object_position.at(2));
//      kinematics_pose.push_back(object_orientation.w());
//      kinematics_pose.push_back(object_orientation.x());
//      kinematics_pose.push_back(object_orientation.y());
//      kinematics_pose.push_back(object_orientation.z());

//      if(!setJointSpacePathToKinematicsPose(kinematics_pose, 2.0))
//      {
//        cout << "Fail Service!" << endl;
//        return;
//      }

//      motion_flag = 0;
//    }
//  }
//  else
//  {
//    if(motion_flag)
//      ROS_INFO("No markers!");

//    motion_flag = 0;
//  }

}

bool OpenManipulatorMotion::setJointSpacePathToKinematicsPose(std::vector<double> kinematics_pose, double path_time)
{
  open_manipulator_msgs::SetKinematicsPose srv;

  srv.request.end_effector_name = "gripper";

  srv.request.kinematics_pose.pose.position.x = kinematics_pose.at(0);
  srv.request.kinematics_pose.pose.position.y = kinematics_pose.at(1);
  srv.request.kinematics_pose.pose.position.z = kinematics_pose.at(2);

  srv.request.kinematics_pose.pose.orientation.w = kinematics_pose.at(3);
  srv.request.kinematics_pose.pose.orientation.x = kinematics_pose.at(4);
  srv.request.kinematics_pose.pose.orientation.y = kinematics_pose.at(5);
  srv.request.kinematics_pose.pose.orientation.z = kinematics_pose.at(6);

  srv.request.path_time = path_time;

  cout << "call" << endl;

  if(goal_joint_space_path_to_kinematics_pose_client_.call(srv))
  {
    return srv.response.is_planned;
  }
  return false;
}

void OM_MOTION::quaternionToEulerV(const Eigen::Quaterniond quaterniond)
{
  double x,y,z,w;

  w = quaterniond.w();
  x = quaterniond.x();
  y = quaterniond.y();
  z = quaterniond.z();

  double sqx = x*x;
  double sqy = y*y;
  double sqz = z*z;
  double sqw = w*w;

  double eulerX = (double)(atan2(2.0*(y*z + x*w),(-sqx-sqy+sqz+sqw))*PI);
  double eulerY = (double)(asin(-2.0*(x*z-y*w))*PI);
  double eulerZ = (double)(atan2(2.0*(x*y+z*w),(sqx-sqy-sqz+sqw))*PI);

  cout << "EULERX: " << eulerX << endl;
  cout << "EULERY: " << eulerY << endl;
  cout << "EULERZ: " << eulerZ << endl << endl;
}


void OM_MOTION::quaternionToEulerN()
{
  double x,y,z,w;

  w = kinematics_pose_.pose.orientation.w;
  x = kinematics_pose_.pose.orientation.x;
  y = kinematics_pose_.pose.orientation.y;
  z = kinematics_pose_.pose.orientation.z;

  double sqx = x*x;
  double sqy = y*y;
  double sqz = z*z;
  double sqw = w*w;

  double eulerX = (double)(atan2(2.0*(y*z + x*w),(-sqx-sqy+sqz+sqw))*PI);
  double eulerY = (double)(asin(-2.0*(x*z-y*w))*PI);
  double eulerZ = (double)(atan2(2.0*(x*y+z*w),(sqx-sqy-sqz+sqw))*PI);

  cout << "eulerx: " << eulerX << endl;
  cout << "eulery: " << eulerY << endl;
  cout << "eulerz: " << eulerZ << endl << endl;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "open_manipulator_motion");

  OpenManipulatorMotion openmanipulatormotion;

  ROS_INFO("OpenManipulator Motion Node");

  ros::Rate loop_rate(100);
  while (ros::ok())
  {
    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}
