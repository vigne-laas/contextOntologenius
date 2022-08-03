cmake_minimum_required(VERSION 3.0.2)
project(context-package)

add_compile_options(-std=c++14)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  ontologenius
  pluginlib
)

find_package(pluginlib REQUIRED)


################################################
## Declare ROS messages, services and actions ##
################################################

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   std_msgs  # Or other packages containing msgs
# )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if your package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
 INCLUDE_DIRS include
#  LIBRARIES ontologenius_interface
 CATKIN_DEPENDS ontologenius
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${ontologenius_INCLUDE_DIRS}
)

catkin_install_python(PROGRAMS scripts/scenario.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)


##############################
## PLUGINS
##############################
add_library(ontologenius_context_reasoner_plugin
  src/ReasonerContext.cpp
)
target_link_libraries(ontologenius_context_reasoner_plugin
  ${catkin_LIBRARIES}
  ${ontologenius_LIBRARIES}
)


