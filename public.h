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

class smecyAttribute;
class smecyClause;

//global wrapper for smecy directive parsing
smecyAttribute *parseSmecyDirective(std::string);

#endif //PUBLIC_H
