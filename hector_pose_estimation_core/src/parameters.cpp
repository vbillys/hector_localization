//=================================================================================================
// Copyright (c) 2011, Johannes Meyer, TU Darmstadt
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Flight Systems and Automatic Control group,
//       TU Darmstadt, nor the names of its contributors may be used to
//       endorse or promote products derived from this software without
//       specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//=================================================================================================

#include <hector_pose_estimation/parameters.h>
#include <hector_pose_estimation/ros/parameters.h>

#include <boost/algorithm/string.hpp>

#include <hector_pose_estimation/matrix.h>

#include <iostream>

namespace hector_pose_estimation {

template <typename T>
struct ParameterRegistryROS::Handler
{
  bool operator()(const ParameterPtr& parameter, ros::NodeHandle& nh) {
    try {
      TypedParameter<T> p(*parameter);
      std::string param_key(boost::algorithm::to_lower_copy(parameter->key));
      if (!nh.getParam(param_key, p.value)) {
        nh.setParam(param_key, p.value);
        ROS_DEBUG_STREAM("Registered parameter " << param_key << " with new value " << p.value);
      } else {
        ROS_DEBUG_STREAM("Found parameter " << param_key << " with value " << p.value);
      }
      return true;
    } catch(std::bad_cast&) {
      return false;
    }
  }
};

template <>
struct ParameterRegistryROS::Handler<ColumnVector>
{
  bool operator()(const ParameterPtr& parameter, ros::NodeHandle& nh) {
    try {
      TypedParameter<ColumnVector> p(*parameter);
      std::string param_key(boost::algorithm::to_lower_copy(parameter->key));
      XmlRpc::XmlRpcValue vector;
      if (!nh.getParam(param_key, vector)) {
        /// nh.setParam(param_key, p.value);
        ROS_DEBUG_STREAM("Not registered vector parameter " << param_key << ". Using defaults.");
      } else {
        if (vector.getType() != XmlRpc::XmlRpcValue::TypeArray) {
          ROS_WARN_STREAM("Found parameter " << param_key << ", but it's not an array!");
          return false;
        }
        p.value.resize(vector.size());
        for(int i = 0; i < vector.size(); ++i) p.value[i] = vector[i];
        ROS_DEBUG_STREAM("Found parameter " << param_key << " with value " << p.value);
      }
      return true;
    } catch(std::bad_cast&) {
      return false;
    }
  }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vector) {
  os << "[";
  for(typename std::vector<T>::const_iterator it = vector.begin(); it != vector.end(); ++it) {
    if (it != vector.begin()) os << ", ";
    os << *it;
  }
  os << "]";
  return os;
}

template <typename T>
struct ParameterRegistryROS::Handler< std::vector<T> >
{
  bool operator()(const ParameterPtr& parameter, ros::NodeHandle& nh) {
    try {
      TypedParameter< std::vector<T> > p(*parameter);
      std::string param_key(boost::algorithm::to_lower_copy(parameter->key));
      XmlRpc::XmlRpcValue vector;
      if (!nh.getParam(param_key, vector)) {
        /// nh.setParam(param_key, p.value);
        ROS_DEBUG_STREAM("Not registered vector parameter " << param_key << ". Using defaults.");
      } else {
        if (vector.getType() != XmlRpc::XmlRpcValue::TypeArray) {
          ROS_WARN_STREAM("Found parameter " << param_key << ", but it's not an array!");
          return false;
        }
        p.value.resize(vector.size());
        for(int i = 0; i < vector.size(); ++i) p.value[i] = vector[i];
        ROS_DEBUG_STREAM("Found parameter " << param_key << " with value " << p.value);
      }
      return true;
    } catch(std::bad_cast&) {
      return false;
    }
  }
};

ParameterRegistryROS::ParameterRegistryROS(ros::NodeHandle nh)
  : nh_(nh)
{}

void ParameterRegistryROS::operator ()(ParameterPtr parameter) {
  if (Handler<std::string>()(parameter, nh_) ||
      Handler<double>()(parameter, nh_) ||
      Handler<std::vector<double> >()(parameter, nh_) ||
      Handler<int>()(parameter, nh_) ||
      Handler<bool>()(parameter, nh_) ||
      Handler<ColumnVector>()(parameter, nh_)
     ) {
    return;
  }
  ROS_ERROR("Could not register parameter %s due to unknown type %s!", parameter->key.c_str(), parameter->type());
}

void ParameterList::initialize(ParameterRegisterFunc func) const {
  for(const_iterator it = begin(); it != end(); ++it) func(*it);
}

} // namespace hector_pose_estimation
