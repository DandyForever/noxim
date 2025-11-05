#ifndef __NOXIMROUTING_CUSTOM_H__
#define __NOXIMROUTING_CUSTOM_H__

#include "../Router.h"
#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"

using namespace std;

class Routing_CUSTOM : RoutingAlgorithm {
public:
  vector<int> route(Router *router, const RouteData &routeData);

  static Routing_CUSTOM *getInstance();

private:
  Routing_CUSTOM(){};
  ~Routing_CUSTOM(){};

  static Routing_CUSTOM *routing_CUSTOM;
  static RoutingAlgorithmsRegister routingAlgorithmsRegister;
  static RoutingAlgorithm *xy;
  static RoutingAlgorithm *yx;
};

#endif
