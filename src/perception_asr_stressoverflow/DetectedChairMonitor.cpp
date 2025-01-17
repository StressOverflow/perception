// Copyright 2023 (c) StressOverflow
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <tf2/transform_datatypes.h>
#include <tf2/LinearMath/Quaternion.h>

#include <memory>

#include "perception_asr_stressoverflow/DetectedChairMonitor.hpp"

#include "geometry_msgs/msg/transform_stamped.hpp"

#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

#include "rclcpp/rclcpp.hpp"

namespace perception_asr_stressoverflow
{

using namespace std::chrono_literals;

DetectedChairMonitor::DetectedChairMonitor()
: Node("chair_monitor"),
  tf_buffer_(),
  tf_listener_(tf_buffer_)
{
  marker_pub_ = create_publisher<visualization_msgs::msg::Marker>("chair_marker", 1);

  timer_ = create_wall_timer(
    100ms, std::bind(&DetectedChairMonitor::control_cycle, this));
}

void
DetectedChairMonitor::control_cycle()
{
  geometry_msgs::msg::TransformStamped robot2person;

  try {
    robot2person = tf_buffer_.lookupTransform(
      "base_link", "detected_chair", tf2::TimePointZero);
  } catch (tf2::TransformException & ex) {
    RCLCPP_WARN(get_logger(), "Chair transform not found: %s", ex.what());
    return;
  }

  double x = robot2person.transform.translation.x;
  double y = robot2person.transform.translation.y;
  double z = robot2person.transform.translation.z;
  double theta = atan2(y, x);

  RCLCPP_INFO(
    get_logger(), "Chair detected at (%lf m, %lf m, , %lf m) = %lf rads",
    x, y, z, theta);

  visualization_msgs::msg::Marker obstacle_arrow;
  obstacle_arrow.header.frame_id = "base_link";
  obstacle_arrow.header.stamp = now();
  obstacle_arrow.type = visualization_msgs::msg::Marker::ARROW;
  obstacle_arrow.action = visualization_msgs::msg::Marker::ADD;
  obstacle_arrow.lifetime = rclcpp::Duration(1s);

  geometry_msgs::msg::Point start;
  start.x = 0.0;
  start.y = 0.0;
  start.z = 0.0;
  geometry_msgs::msg::Point end;
  end.x = x;
  end.y = y;
  end.z = z;
  obstacle_arrow.points = {start, end};

  obstacle_arrow.color.r = 0.0;
  obstacle_arrow.color.g = 0.0;
  obstacle_arrow.color.b = 1.0;
  obstacle_arrow.color.a = 1.0;

  obstacle_arrow.scale.x = 0.02;
  obstacle_arrow.scale.y = 0.1;
  obstacle_arrow.scale.z = 0.1;

  marker_pub_->publish(obstacle_arrow);
}

}  // namespace perception_asr_stressoverflow
