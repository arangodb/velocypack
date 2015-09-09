#ifndef JASON_TYPE_H
#define JASON_TYPE_H

namespace triagens {
  namespace basics {
    
    enum JasonType {
      None,           // not yet initialised
      Null,           // JSON null
      Bool,
      Double,
      String,
      Array,
      Object,
      External,
      ID,
      ArangoDB_id,
      UTCDate,
      Int,
      UInt,
      Binary
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
