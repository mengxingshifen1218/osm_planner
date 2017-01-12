//
// Created by michal on 31.12.2016.
//


#include <osm_planner/osm_parser.h>

OsmParser::OsmParser(std::string xml){

        ros::NodeHandle n;

        marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1000);
        path_pub = n.advertise<nav_msgs::Path>("path", 10);
        shortest_path_pub = n.advertise<nav_msgs::Path>("shortest_path", 10);

        TiXmlDocument doc(xml);
        TiXmlNode* osm;
        TiXmlNode* node;
        TiXmlNode* way;

        TiXmlHandle hRootNode(0);
        TiXmlHandle hRootWay(0);
        TiXmlHandle hRootTag(0);

        bool loadOkay = doc.LoadFile();
        if (loadOkay)
        {
            ROS_INFO("loaded map: %s", xml.c_str());
        }
        else
        {
            ROS_ERROR("Failed to load file %s", xml.c_str());
            throw std::runtime_error("Failed to load xml");
        }

        osm = doc.FirstChildElement();
        node = osm->FirstChild("node");
        way = osm->FirstChild("way");
        TiXmlElement* nodeElement = node->ToElement();
        TiXmlElement* wayElement = way->ToElement();

        hRootNode=TiXmlHandle(nodeElement);
        hRootWay=TiXmlHandle(wayElement);

        createWays(&hRootWay);
        createNodes(&hRootNode);
        createNetwork();
    }


    //publishing all osm nodes
    void OsmParser::publishMarker(){
        visualization_msgs::Marker marker, line_strip, line_list;
        marker.header.frame_id = line_strip.header.frame_id = line_list.header.frame_id = "/map";
        marker.header.stamp = line_strip.header.stamp = line_list.header.stamp = ros::Time::now();
        marker.ns = line_strip.ns = line_list.ns = "work_space";
        marker.action = line_strip.action = line_list.action = visualization_msgs::Marker::ADD;
        marker.pose.orientation.w = line_strip.pose.orientation.w = line_list.pose.orientation.w = 1.0;

        marker.id = 0;
        line_list.id = 2;

        marker.type = visualization_msgs::Marker::POINTS;
        line_list.type = visualization_msgs::Marker::ARROW;

        line_list.scale.x = 0.05;
        line_list.scale.y = 0.05;
        line_list.scale.z = 0.05;

        line_list.color.r = 1.0f;
        line_list.color.g = 0.0f;
        line_list.color.b = 0.0f;
        line_list.color.a = 1.0;

        line_list.lifetime = ros::Duration(10);

        marker.scale.x = 0.05;
        marker.scale.y = 0.05;
        marker.scale.z = 0.05;

        marker.color.r = 0.0f;
        marker.color.g = 0.0f;
        marker.color.b = 1.0f;
        marker.color.a = 1.0;

        marker.lifetime = ros::Duration(100);
        geometry_msgs::Point point;
        point.z = 0;

        float lat, lon, latZero, lonZero;

        latZero = nodes[0].latitude;
        lonZero = nodes[0].longitude;
        point.x = 0;
        point.y = 0;

        for (int i = 0; i < nodes.size(); i++) {

            lat = nodes[i].latitude;
            lon = nodes[i].longitude;

            point.x = (lonZero - lon) * 1000;
            point.y = (latZero - lat) * 1000;

            marker.points.push_back(point);
            //	ROS_INFO("lat %f lon %f",lat, lon);
        }

        marker_pub.publish(marker);


        //vykreslenie lokalizacie
        /*marker.points.clear();
        marker.scale.x = 0.1;
        marker.scale.y = 0.1;
        marker.scale.z = 0.1;

        marker.color.r = 1.0f;
        marker.color.g = 0.0f;
        marker.color.b = 0.0f;
        marker.color.a = 1.0;

        OSM_NODE testLocale;
        testLocale.latitude = 48.1464707;
        testLocale.longitude = 17.0526409;

        //setLocale(&testLocale);

        //ROS_INFO("new pos  lat %f  lon %f", testLocale.latitude,
        //		testLocale.longitude);
        point.x = (lonZero - lon) * 1000;
        point.y = (latZero - lat) * 1000;

        marker.points.push_back(point);
        marker_pub.publish(marker);
*/
    }

//publishing all paths
    void OsmParser::publishPath(){

        OSM_NODE testNode;
        nav_msgs::Path path;
        path.header.frame_id = "/map";
        geometry_msgs::PoseStamped pose;
        pose.pose.position.x = 0;
        pose.pose.position.y = 0;
        pose.pose.position.z = 0;
        float latZero, lonZero;

        latZero = nodes[0].latitude;
        lonZero = nodes[0].longitude;

        for (int i = 0; i < ways.size(); i++){
            path.poses.clear();

            for (int j = 0; j < ways[i].nodesId.size(); j++){

                pose.pose.position.x = (lonZero - nodes[ways[i].nodesId[j]].longitude) * 1000;
                pose.pose.position.y = (latZero - nodes[ways[i].nodesId[j]].latitude) * 1000;
                path.poses.push_back(pose);

            }
            usleep(100000);

            path_pub.publish(path);
        }

    }

//publishing defined path
void OsmParser::publishPath(std::vector<int> nodesInPath) {

    sh_path.poses.clear();
    sh_path.header.frame_id = "/map";

    geometry_msgs::PoseStamped pose;

    pose.pose.position.x = 0;
    pose.pose.position.y = 0;
    pose.pose.position.z = 0;
    float latZero, lonZero;

    latZero = nodes[0].latitude;
    lonZero = nodes[0].longitude;


    for (int i = 0; i < nodesInPath.size(); i++) {

        pose.pose.position.x = (lonZero - nodes[nodesInPath[i]].longitude) * 1000;
        pose.pose.position.y = (latZero - nodes[nodesInPath[i]].latitude) * 1000;
        sh_path.poses.push_back(pose);
    }

    usleep(300000);
    shortest_path_pub.publish(sh_path);

}

//getter for dijkstra algorithm
std::vector< std::vector<double> > OsmParser::getGraphOfVertex(){
    return networkArray;
}

    /*void setLocale(OSM_NODE *position){

        OSM_NODE test = *position;
        double distance = getDistance(*position, nodes[0]);
        double minDistance = distance;
        memcpy(position, &nodes[0], sizeof(OSM_NODE));

        for (int i = 0; i < nodes.size(); i++){
            distance = getDistance(*position, nodes[i]);
            if (minDistance > distance){
                minDistance = distance;
                memcpy(&test, &nodes[i], sizeof(OSM_NODE));
            }

        }
        memcpy(position, &test, sizeof(OSM_NODE));
    }
*/

//protected function

//distance between two osm nodes
double OsmParser::getDistance(OSM_NODE node1, OSM_NODE node2){
    return sqrt(pow(node1.latitude - node2.latitude, 2.0) + pow(node1.longitude - node2.longitude, 2.0)) * 1000;
}

//angle between two osm nodes
double OsmParser::getBearing(OSM_NODE node1, OSM_NODE node2){
    return atan((node1.longitude - node2.longitude)/(node1.latitude - node2.latitude));
}

//private functions

//parse all footways in osm maps
//todo parametrizovat funkciu na vyber roznych ciest napr. footway
    void OsmParser::createWays(TiXmlHandle* hRootWay){

        ways.clear();
        TiXmlElement* wayElement = hRootWay->Element();

        OSM_WAY wayTmp;
        TiXmlElement* tag;
        int counter = 1;
        //prejde vsetky elementy way
        for( wayElement; wayElement; wayElement = wayElement->NextSiblingElement("way")){

            tag = wayElement->FirstChildElement("tag");
            wayElement->Attribute("id", &wayTmp.id);

            //prejde vsetky elementy tag
            while (tag != NULL){

                std::string key(tag->Attribute("k"));
                std::string value(tag->Attribute("v"));

                if (key == "highway" && value == "footway"){

                    wayElement->Attribute("id", &wayTmp.id);
                    getNodesInWay(wayElement, &wayTmp); //finding all nodes located in selected way
                    ways.push_back(wayTmp);
                    break;
                }
                tag = tag->NextSiblingElement("tag");
            }
        }
    }

    //parse all osm nodes and select nodes located in ways (footways)
    void OsmParser::createNodes(TiXmlHandle *hRootNode){

        nodes.clear();
        nodes.resize(table.size());

        TiXmlElement *nodeElement = hRootNode->Element();

        int id;
        int i = 0;

        for( nodeElement; nodeElement; nodeElement = nodeElement->NextSiblingElement("node")){


            nodeElement->Attribute("id", &id);

            int ret;
            if (!translateID(id, &ret)){
                continue;
            }

            nodes[ret].id = ret;
            nodeElement->Attribute("lat", &nodes[ret].latitude);
            nodeElement->Attribute("lon", &nodes[ret].longitude);
        }

    }

    //creating graph for dijkstra algorithm
    void OsmParser::createNetwork(){

        networkArray.resize(nodes.size());
        //ROS_ERROR("size %d ", networkArray.size());
        for (int i = 0; i < networkArray.size(); i++){
            networkArray[i].resize(nodes.size());
            //  ROS_WARN("size %d ", networkArray[i].size());
        }

        double distance = 0;
        //prejde vsetky cesty
        for (int i = 0; i < ways.size(); i++){

            //prejde vsetky uzly na ceste
            for (int j = 0; j < ways[i].nodesId.size() - 1; j++){
                //vypocita vzdialenost medzi susednimi uzlami
                distance = getDistance(nodes[ways[i].nodesId[j]], nodes[ways[i].nodesId[j + 1]]);
                //
                if (networkArray[ways[i].nodesId[j]] [ways[i].nodesId[j + 1]] != 0)
                    ROS_ERROR("pozicia [%d, %d] je obsadena - toto by nemalo nastat",ways[i].nodesId[j], ways[i].nodesId[j + 1]);
                networkArray[ways[i].nodesId[j]] [ways[i].nodesId[j + 1]] = distance;
                networkArray[ways[i].nodesId[j + 1]] [ways[i].nodesId[j]] = distance;

            }

        }

    }

    //finding nodes located on way and fill way.nodesId
    void OsmParser::getNodesInWay(TiXmlElement* wayElement, OSM_WAY *way){

        TiXmlHandle hRootNode(0);
        TiXmlHandle hRootNd(0);
        TiXmlElement* nodeElement;
        int id;
        static int id_new = 0;

        way->nodesId.clear();

        nodeElement = wayElement->FirstChild("nd")->ToElement();
        hRootNode = TiXmlHandle(nodeElement);

        nodeElement = hRootNode.Element();
        TRANSLATE_TABLE tableTmp;

        for( nodeElement; nodeElement; nodeElement = nodeElement->NextSiblingElement("nd")) {

            nodeElement->Attribute("ref", &id);
            tableTmp.oldID = id;
            int ret;
            if (!translateID(tableTmp.oldID, &ret)){

                tableTmp.newID = id_new++;
                table.push_back(tableTmp);
                way->nodesId.push_back(tableTmp.newID);

            } else {
                way->nodesId.push_back(ret);
            }
        }

    }

//preklada stare osm node ID na nove osm node ID (cielom bolo vytvorit usporiadane indexovanie)
    bool OsmParser::translateID(int id, int *ret_value){

        for (int i = 0; i < table.size(); i++){
            if (table[i].oldID == id) {
                ret_value[0] = table[i].newID;
                return true;
            }
        }
        return false;
    }