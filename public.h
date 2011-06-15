#ifndef PUBLIC_H
#define PUBLIC_H

#include "rose.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

class smecyAttribute;
class smecyClause;

//global wrapper for smecy directive parsing TODO change return to smecyAttribute
smecyAttribute *parseSmecyDirective(std::string);	//FIXME parser can't recover after a syntax error

#endif //PUBLIC_H
