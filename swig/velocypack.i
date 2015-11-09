%module velocypack
%{
#include <velocypack/vpack.h>
using namespace arangodb::velocypack;
%}
%include "std_string.i"
%include "std_vector.i"
%include "../include/velocypack/vpack.h"
