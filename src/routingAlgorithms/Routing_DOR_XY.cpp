#include "Routing_DOR_XY.h"

RoutingAlgorithmsRegister
    Routing_DOR_XY::routingAlgorithmsRegister("DOR_XY", getInstance());

Routing_DOR_XY *Routing_DOR_XY::routing_DOR_XY = 0;
RoutingAlgorithm *Routing_DOR_XY::xy = 0;
RoutingAlgorithm *Routing_DOR_XY::yx = 0;

Routing_DOR_XY *Routing_DOR_XY::getInstance() {
  if (routing_DOR_XY == 0)
    routing_DOR_XY = new Routing_DOR_XY();

  return routing_DOR_XY;
}

vector<int> Routing_DOR_XY::route(Router *router, const RouteData &routeData) {
  if (routeData.vc_id % 2 == 0) {
    if (!xy) {
      xy = RoutingAlgorithms::get("XY");

      assert(xy);
    }

    return xy->route(router, routeData);
  } else {
    if (!yx) {
      yx = RoutingAlgorithms::get("YX");

      assert(yx);
    }

    return yx->route(router, routeData);
  }
}
