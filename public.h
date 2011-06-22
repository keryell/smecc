#ifndef PUBLIC_H
#define PUBLIC_H

#include "rose.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>

namespace smecy
{
	class Attribute;
	class Clause;
	class intExpr;
	
	//global wrapper for smecy directive parsing
	Attribute* parseDirective(std::string);
}

#endif //PUBLIC_H
