#ifndef __NOXIMROUTING_MOD_DOR_H__
#define __NOXIMROUTING_MOD_DOR_H__

#include "../Router.h"
#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"

using namespace std;

class Routing_MOD_DOR : RoutingAlgorithm {
public:
  vector<int> route(Router *router, const RouteData &routeData);

  static Routing_MOD_DOR *getInstance();

private:
  Routing_MOD_DOR(){};
  ~Routing_MOD_DOR(){};

  static Routing_MOD_DOR *routing_MOD_DOR;
  static RoutingAlgorithmsRegister routingAlgorithmsRegister;
  static RoutingAlgorithm *xy;
  static RoutingAlgorithm *yx;
};

#endif
