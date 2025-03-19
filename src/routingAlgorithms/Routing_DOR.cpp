#include "Routing_DOR.h"

RoutingAlgorithmsRegister Routing_DOR::routingAlgorithmsRegister("DOR",
                                                                 getInstance());

Routing_DOR *Routing_DOR::routing_DOR = 0;
RoutingAlgorithm *Routing_DOR::xy = 0;
RoutingAlgorithm *Routing_DOR::yx = 0;

Routing_DOR *Routing_DOR::getInstance() {
  if (routing_DOR == 0)
    routing_DOR = new Routing_DOR();

  return routing_DOR;
}

vector<int> Routing_DOR::route(Router *router, const RouteData &routeData) {
  if (routeData.vc_id % 2 == 0) {
    if (!yx) {
      yx = RoutingAlgorithms::get("YX");

      assert(yx);
    }

    return yx->route(router, routeData);
  } else {
    if (!xy) {
      xy = RoutingAlgorithms::get("XY");

      assert(xy);
    }

    return xy->route(router, routeData);
  }
}
