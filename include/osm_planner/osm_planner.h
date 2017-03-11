#include <ros/ros.h>
#include <osm_planner/dijkstra.h>
#include <osm_planner/osm_parser.h>
#include <osm_planner/newTarget.h>
#include <osm_planner/cancelledPoint.h>
#include <std_msgs/Int32.h>
#include <std_srvs/Empty.h>
#include <geometry_msgs/Point.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <sensor_msgs/NavSatFix.h>

namespace osm_planner {

    class Planner{
    public:

        typedef struct point{
            int id;
            Parser::OSM_NODE geoPoint;
            geometry_msgs::PoseStamped cartesianPoint;
        }POINT;

        Planner();
        void initialize();

        int makePlan(double target_latitude, double target_longitude);

    protected:

        //Before start make plan, this function must be call
        void initializePos(double lat, double lon);

        //make plan from source to target
        int planning(int sourceID, int targetID);

        //deleted selected point id on the path
        int cancelPoint(int pointID);

        //update pose
        bool updatePose(); //from tf
        void setPositionFromGPS(double lat, double lon);        //from gps
        void setPositionFromOdom(geometry_msgs::Point point);  //from odom

    private:

        Parser osm;
        Dijkstra dijkstra;

        bool initialized_ros;
        bool initialized_position;

        POINT source;
        POINT target;

        //global ros parameters
        bool use_tf;
        std::string map_frame, base_link_frame;
        double interpolation_max_distance;

        /*Publisher*/
        ros::Publisher shortest_path_pub;

        //msgs for shortest path
        nav_msgs::Path path;

        /* Services */
        ros::ServiceServer init_service;
        ros::ServiceServer cancel_point_service;

        //callbacks
        bool initCallback(osm_planner::newTarget::Request &req, osm_planner::newTarget::Response &res);
        bool cancelPointCallback(osm_planner::cancelledPoint::Request &req, osm_planner::cancelledPoint::Response &res);

        double checkDistance(int node_id, double lat, double lon);
        double checkDistance(int node_id, geometry_msgs::Pose pose);
    };
}