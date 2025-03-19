#ifndef __NOXIMROUTING_DOR_XY_H__
#define __NOXIMROUTING_DOR_XY_H__

#include "../Router.h"
#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"

using namespace std;

class Routing_DOR_XY : RoutingAlgorithm {
public:
  vector<int> route(Router *router, const RouteData &routeData);

  static Routing_DOR_XY *getInstance();

private:
  Routing_DOR_XY(){};
  ~Routing_DOR_XY(){};

  static Routing_DOR_XY *routing_DOR_XY;
  static RoutingAlgorithmsRegister routingAlgorithmsRegister;
  static RoutingAlgorithm *xy;
  static RoutingAlgorithm *yx;
};

#endif
