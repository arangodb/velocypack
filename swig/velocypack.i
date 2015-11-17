%module velocypack
%{
#include <velocypack/vpack.h>
using namespace arangodb::velocypack;
%}

%include "exception.i"
%include "stdint.i"
//%include "std_iostream.i"
%include "std_string.i"
%include "std_vector.i"

// Ignore all classes unless mentioned explicitly:
%rename($ignore, %$isclass) ""; // Only ignore all classes

// Handle our exceptions:
%exception {
  try {
    $action
  } catch(Exception const& e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  } catch(std::bad_alloc const&) {
    SWIG_exception(SWIG_MemoryError, "Out of memory");
  } catch(...) {
    SWIG_exception(SWIG_RuntimeError,"Unknown exception");
  }
}

%rename(%s) Version;
%include "../include/velocypack/Version.h"

%ignore "checkValueLength";
%ignore "getVariableValueLength";
%ignore "readVariableValueLength";
%ignore "storeVariableValueLength";
%ignore "toUInt64";
%ignore "toInt64";
%include "../include/velocypack/velocypack-common.h"

%rename(%s) ValueType;
%rename(_None) ValueType::None;
%include "../include/velocypack/ValueType.h"

%rename(%s) Value;
%rename(%s) ValuePair;
%include "../include/velocypack/Value.h"

%rename(%s) AttributeTranslator;
%include "../include/velocypack/AttributeTranslator.h"

%rename(%s) Exception;
%include "../include/velocypack/Exception.h"

%rename(%s) Options;
%include "../include/velocypack/Options.h"

%include "../include/velocypack/Buffer.h"

%rename(%s) Builder;
%include "../include/velocypack/Builder.h"

%rename(%s) Slice;
%include "../include/velocypack/Slice.h"

%rename(%s) Dumper;
%include "../include/velocypack/Dumper.h"

%rename(%s) Parser;
%include "../include/velocypack/Parser.h"

//%include "../include/velocypack/Collection.h"
//%include "../include/velocypack/HexDump.h"
//%include "../include/velocypack/Iterator.h"
//%include "../include/velocypack/Sink.h"
