#ifndef __NOXIMROUTING_DOR_H__
#define __NOXIMROUTING_DOR_H__

#include "../Router.h"
#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"

using namespace std;

class Routing_DOR : RoutingAlgorithm {
public:
  vector<int> route(Router *router, const RouteData &routeData);

  static Routing_DOR *getInstance();

private:
  Routing_DOR(){};
  ~Routing_DOR(){};

  static Routing_DOR *routing_DOR;
  static RoutingAlgorithmsRegister routingAlgorithmsRegister;
};

#endif
