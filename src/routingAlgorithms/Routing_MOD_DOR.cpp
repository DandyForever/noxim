#include "Routing_MOD_DOR.h"

RoutingAlgorithmsRegister
    Routing_MOD_DOR::routingAlgorithmsRegister("MOD_DOR", getInstance());

Routing_MOD_DOR *Routing_MOD_DOR::routing_MOD_DOR = 0;
RoutingAlgorithm *Routing_MOD_DOR::xy = 0;
RoutingAlgorithm *Routing_MOD_DOR::yx = 0;

Routing_MOD_DOR *Routing_MOD_DOR::getInstance() {
  if (routing_MOD_DOR == 0)
    routing_MOD_DOR = new Routing_MOD_DOR();

  return routing_MOD_DOR;
}

vector<int> Routing_MOD_DOR::route(Router *router, const RouteData &routeData) {
  if (routeData.vc_id % 2 == 1) {
    if (!xy) {
      xy = RoutingAlgorithms::get("XY");

      assert(xy);
    }

    return xy->route(router, routeData);
  } else {
    if (!yx) {
      yx = RoutingAlgorithms::get("DOR");

      assert(yx);
    }

    return yx->route(router, routeData);
  }
}
