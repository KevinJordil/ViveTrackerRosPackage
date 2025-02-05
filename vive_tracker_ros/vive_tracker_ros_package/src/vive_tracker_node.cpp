// System
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
// ROS
#include "ros/ros.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/PoseStamped.h"
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
// VIVE
#include "survive.h"

int counter = 0;
bool wait_set00, wait_set300200, set_next_turn;
double offset_x, offset_y, alpha, cos_alpha, sin_alpha, offset_orientation;
//geometry_msgs::Pose tracker_pose;
geometry_msgs::PoseStamped tracker_pose;
//geometry_msgs::PoseStamped lighthouse_pose;
//geometry_msgs::TransformStamped transformStamped_world_lh;
geometry_msgs::TransformStamped transformStamped_lh_tracker;
ros::Publisher pose_publisher;
ros::Publisher lighthouse_initial_pose_publisher;

void tracker_pose_process(SurviveObject *so, survive_long_timecode timecode, const SurvivePose *pose)
{
  survive_default_pose_process(so, timecode, pose);

  tracker_pose.pose.position.x = pose->Pos[0];
  tracker_pose.pose.position.y = pose->Pos[1];

  if(!wait_set00 && counter == 200){
    if(!set_next_turn){
      std::string inputString;
      std::cout << "Place at x:0 and y:0 and press enter";
      std::getline(std::cin, inputString);
      set_next_turn = true;
      ros::spinOnce();
      counter = 0;
    } else {
      offset_x = 100 * pose->Pos[0];
      offset_y = 100 * pose->Pos[1];
      offset_orientation = pose->Rot[2];

      printf("cal pose: [x:%.3f,y:%.3f]\n", (100 * pose->Pos[0]), (100 * pose->Pos[1]));
      printf("offset: [x:%.3f,y:%.3f]\n", offset_x, offset_y);
      wait_set00 = true;
      set_next_turn = false;
      counter = 0;
    }
  } else if (!wait_set300200 && counter == 200) {
    if(!set_next_turn){
      std::string inputString;
      std::cout << "Place at x:300 and y:200 and press enter";
      std::getline(std::cin, inputString);
      set_next_turn = true;
      ros::spinOnce();
      counter = 0;
    } else {
      alpha = atan( (100 * pose->Pos[1] - offset_y) / (100 * pose->Pos[0] - offset_x)) - 0.9907935;
      printf("cal pose: [x:%.3f,y:%.3f]\n", (100 * pose->Pos[0]), (100 * pose->Pos[1]));
      printf("alpha: %.3f\n", alpha);
      cos_alpha = cos(alpha);
      sin_alpha = sin(alpha);


      wait_set300200 = true;
      set_next_turn = false;
      counter = 0;
    }
  } else {
    double x, y, o_d;

    x = abs(cos_alpha * ((100 * pose->Pos[1]) - offset_y) - sin_alpha * ((100 * pose->Pos[0]) - offset_x));
    y = abs(sin_alpha * ((100 * pose->Pos[1]) - offset_y) + cos_alpha * ((100 * pose->Pos[0]) - offset_x));
    //o_d = 180 * (((pose->Rot[2]) + 1) - alpha / M_PI);
    o_d =  fmod(pose->Rot[2] * 180, 360);
    o_d = o_d < 0 ? o_d + 360 : o_d;

    printf("[x:%.1f,y:%.1f] Old: [x:%.1f,y:%.1f] o[z:%.2f]\n", x, y, (100 * pose->Pos[0]), (100 * pose->Pos[1]), o_d);
  
  }

  counter++;

  //printf("Pose: [x:%.3f,y:%.3f]\n", (100 * pose->Pos[0]), (100 * pose->Pos[1]));


  tracker_pose.pose.position.x = pose->Pos[0];
  tracker_pose.pose.position.y = pose->Pos[1];
  tracker_pose.pose.position.z = pose->Pos[2];
  tracker_pose.pose.orientation.x = pose->Rot[0];
  tracker_pose.pose.orientation.y = pose->Rot[1];
  tracker_pose.pose.orientation.z = pose->Rot[2];
  tracker_pose.pose.orientation.w = pose->Rot[3];
  //printf("Pose: [%s][x:% 02.3f,y:% 02.3f,z:% 02.3f] [% 02.2f,% 02.2f,% 02.2f,% 02.2f]\n", so->codename, pose->Pos[0], pose->Pos[1], pose->Pos[2], pose->Rot[0], pose->Rot[1], pose->Rot[2], pose->Rot[3]);
}

/*
void lighthouse_pose_process(SurviveContext *tctx, uint8_t lighthouse, SurvivePose *pose, SurvivePose *obj_pose)
{
  survive_default_lighthouse_pose_process(tctx, lighthouse, pose, obj_pose);
  lighthouse_pose.pose.position.x = pose->Pos[0];
  lighthouse_pose.pose.position.y = pose->Pos[1];
  lighthouse_pose.pose.position.z = pose->Pos[2];
  lighthouse_pose.pose.orientation.x = pose->Rot[0];
  lighthouse_pose.pose.orientation.y = pose->Rot[1];
  lighthouse_pose.pose.orientation.z = pose->Rot[2];
  lighthouse_pose.pose.orientation.w = pose->Rot[3];
  //printf("LH Pose: [%d][% 02.2f,% 02.2f,% 02.2f] [% 02.2f,% 02.2f,% 02.2f,% 02.2f]\n", lighthouse, pose->Pos[0], pose->Pos[1], pose->Pos[2], pose->Rot[0], pose->Rot[1], pose->Rot[2], pose->Rot[3]);
}
*/
void tracker_pose_tf()
{
  static tf2_ros::TransformBroadcaster br;

  transformStamped_lh_tracker.transform.translation.x = tracker_pose.pose.position.x;
  transformStamped_lh_tracker.transform.translation.y = tracker_pose.pose.position.y;
  transformStamped_lh_tracker.transform.translation.z = tracker_pose.pose.position.z;
  transformStamped_lh_tracker.transform.rotation.x = -tracker_pose.pose.orientation.z;
  transformStamped_lh_tracker.transform.rotation.y = tracker_pose.pose.orientation.y;
  transformStamped_lh_tracker.transform.rotation.z = -tracker_pose.pose.orientation.x;
  transformStamped_lh_tracker.transform.rotation.w = tracker_pose.pose.orientation.w;

/*
  transformStamped_world_lh.transform.translation.x = lighthouse_pose.pose.position.x;
  transformStamped_world_lh.transform.translation.y = lighthouse_pose.pose.position.y;
  transformStamped_world_lh.transform.translation.z = lighthouse_pose.pose.position.z;
  transformStamped_world_lh.transform.rotation.x = lighthouse_pose.pose.orientation.x;
  transformStamped_world_lh.transform.rotation.y = lighthouse_pose.pose.orientation.y;
  transformStamped_world_lh.transform.rotation.z = lighthouse_pose.pose.orientation.z;
  transformStamped_world_lh.transform.rotation.w = lighthouse_pose.pose.orientation.w;
*/
  br.sendTransform(transformStamped_lh_tracker);
  //br.sendTransform(transformStamped_world_lh);
}

void puslish_pose_and_tf()
{
  ros::Time t_now = ros::Time::now();
  tracker_pose.header.stamp = t_now;
  //lighthouse_pose.header.stamp = t_now;
  transformStamped_lh_tracker.header.stamp = t_now;
  //transformStamped_world_lh.header.stamp = t_now;
  pose_publisher.publish(tracker_pose);
  //lighthouse_initial_pose_publisher.publish(lighthouse_pose);
  tracker_pose_tf();
}

int main(int argc, char **argv)
{
  std::cout << "ROS Vive Tracker\n";
  struct SurviveContext *ctx;
  std::string config_file;
  std::string nspace(getenv("ROS_NAMESPACE"));

  if (nspace.empty())
  {
    throw std::runtime_error("Could not find namespace. Check if ROS_NAMESPACE is defined");
    return 1;
  }

  // Initialize ROS
  std::cout << "ROS init\n";
  ros::init(argc, argv, "vive_tracker", ros::init_options::AnonymousName);
  ros::NodeHandle n("~");
  n.param<std::string>("config_file", config_file, "config.conf");
  n.deleteParam("config_file");
  pose_publisher = n.advertise<geometry_msgs::PoseStamped>("tracker_pose", 100);
  wait_set00 = false;
  wait_set300200 = false;
  set_next_turn = false;
  //lighthouse_initial_pose_publisher = n.advertise<geometry_msgs::PoseStamped>("lighthouse_pose", 10);

  // Configuring frames
  tracker_pose.header.frame_id = nspace + "_tracker_frame";
  //lighthouse_pose.header.frame_id = nspace + "_lighthouse_frame";
  transformStamped_lh_tracker.header.frame_id = nspace + "_world_frame";
  transformStamped_lh_tracker.child_frame_id = nspace + "_tracker_frame";
  //transformStamped_world_lh.header.frame_id = nspace + "_world_frame";
  //transformStamped_world_lh.child_frame_id = nspace + "_lighthouse_frame";

  // Initialize Tracker
  char* av[] = {"pgm_name","--configfile", const_cast<char*>(config_file.c_str()), const_cast<char*>("--no-calibrate"), NULL};
  int ac = sizeof(av) / sizeof(char *) - 1;
  std::cout << "Before init\n";
  ctx = survive_init(ac, av);
  if ((!ctx))
  {
    throw std::runtime_error("Could not initialize! Exiting.");
    return 1;
  }
  //survive_install_raw_pose_fn(ctx, testprog_raw_pose_process);
  std::cout << "Before pose\n";
  survive_install_pose_fn(ctx, tracker_pose_process);
  //survive_install_lighthouse_pose_fn(ctx, lighthouse_pose_process);
  std::cout << "Before startup\n";
  
  survive_startup(ctx);
  if (ctx->state == SURVIVE_STOPPED)
  {
    throw std::runtime_error("Could not start tracker! Exiting.");
    return 1;
  }

  // Loop until stopped
  
  while (ros::ok() & (survive_poll(ctx) == 0))
  {
    puslish_pose_and_tf();
    ros::spinOnce();
  }
  return 0;
}
