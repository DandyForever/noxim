#include "Routing_YX.h"

RoutingAlgorithmsRegister Routing_YX::routingAlgorithmsRegister("YX",
                                                                getInstance());

Routing_YX *Routing_YX::routing_YX = 0;

Routing_YX *Routing_YX::getInstance() {
  if (routing_YX == 0)
    routing_YX = new Routing_YX();

  return routing_YX;
}

vector<int> Routing_YX::route(Router *router, const RouteData &routeData) {
  Coord current = id2Coord(routeData.current_id);
  Coord destination = id2Coord(routeData.dst_id);
  vector<int> directions;

  if (destination.y > current.y)
    directions.push_back(DIRECTION_SOUTH);
  else if (destination.y < current.y)
    directions.push_back(DIRECTION_NORTH);
  else if (destination.x > current.x)
    directions.push_back(DIRECTION_EAST);
  else
    directions.push_back(DIRECTION_WEST);

  return directions;
}
