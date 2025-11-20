#include "Routing_CUSTOM.h"

RoutingAlgorithmsRegister
    Routing_CUSTOM::routingAlgorithmsRegister("CUSTOM", getInstance());

Routing_CUSTOM *Routing_CUSTOM::routing_CUSTOM = 0;
RoutingAlgorithm *Routing_CUSTOM::xy = 0;
RoutingAlgorithm *Routing_CUSTOM::yx = 0;
RoutingAlgorithm *Routing_CUSTOM::nf = 0;
RoutingAlgorithm *Routing_CUSTOM::nl = 0;
RoutingAlgorithm *Routing_CUSTOM::oe = 0;
RoutingAlgorithm *Routing_CUSTOM::wf = 0;

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
  } else if (get_routing_for_vc(routeData.vc_id) == RoutingType::YX) {
    if (!yx) {
      yx = RoutingAlgorithms::get("YX");

      assert(yx);
    }
    return yx->route(router, routeData);
  } else if (get_routing_for_vc(routeData.vc_id) == RoutingType::NF) {
    if (!nf) {
      nf = RoutingAlgorithms::get("NEGATIVE_FIRST");

      assert(nf);
    }
    return nf->route(router, routeData);
  } else if (get_routing_for_vc(routeData.vc_id) == RoutingType::NL) {
    if (!nl) {
      nl = RoutingAlgorithms::get("NORTH_LAST");

      assert(nl);
    }
    return nl->route(router, routeData);
  } else if (get_routing_for_vc(routeData.vc_id) == RoutingType::OE) {
    if (!oe) {
      oe = RoutingAlgorithms::get("ODD_EVEN");

      assert(oe);
    }
    return oe->route(router, routeData);
  } else if (get_routing_for_vc(routeData.vc_id) == RoutingType::WF) {
    if (!wf) {
      wf = RoutingAlgorithms::get("WEST_FIRST");

      assert(wf);
    }
    return wf->route(router, routeData);
  }
  return {};
}
