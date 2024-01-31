#include "Routing_DOR.h"

RoutingAlgorithmsRegister Routing_DOR::routingAlgorithmsRegister("DOR", getInstance());

Routing_DOR * Routing_DOR::routing_DOR = 0;

Routing_DOR * Routing_DOR::getInstance() {
	if ( routing_DOR == 0 )
		routing_DOR = new Routing_DOR();
    
	return routing_DOR;
}

vector<int> Routing_DOR::route(Router * router, const RouteData & routeData)
{
    Coord current = id2Coord(routeData.current_id);
    Coord destination = id2Coord(routeData.dst_id);
    vector <int> directions;

    if (routeData.vc_id == 0 || routeData.vc_id == 3) {
        if (destination.y > current.y)
            directions.push_back(DIRECTION_SOUTH);
        else if (destination.y < current.y)
            directions.push_back(DIRECTION_NORTH);
        else if (destination.x > current.x)
            directions.push_back(DIRECTION_EAST);
        else
            directions.push_back(DIRECTION_WEST);
    } else {
        if (destination.x > current.x)
            directions.push_back(DIRECTION_EAST);
        else if (destination.x < current.x)
            directions.push_back(DIRECTION_WEST);
        else if (destination.y > current.y)
            directions.push_back(DIRECTION_SOUTH);
        else
            directions.push_back(DIRECTION_NORTH);
    }

    return directions;

}
