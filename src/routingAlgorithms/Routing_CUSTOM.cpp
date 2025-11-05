#include "Routing_CUSTOM.h"

RoutingAlgorithmsRegister
    Routing_CUSTOM::routingAlgorithmsRegister("CUSTOM", getInstance());

Routing_CUSTOM *Routing_CUSTOM::routing_CUSTOM = 0;
RoutingAlgorithm *Routing_CUSTOM::xy = 0;
RoutingAlgorithm *Routing_CUSTOM::yx = 0;

Routing_CUSTOM *Routing_CUSTOM::getInstance() {
  if (routing_CUSTOM == 0)
    routing_CUSTOM = new Routing_CUSTOM();

  return routing_CUSTOM;
}

vector<int> Routing_CUSTOM::route(Router *router, const RouteData &routeData) {
  if (get_routing_for_vc(routeData.vc_id) == RoutingType::XY) {
    if (!xy) {
      xy = RoutingAlgorithms::get("XY");

      assert(xy);
    }
    return xy->route(router, routeData);
  } else { // YX
    if (!yx) {
      yx = RoutingAlgorithms::get("YX");

      assert(yx);
    }
    return yx->route(router, routeData);
  }
}
