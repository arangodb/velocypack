////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "JasonDump.h"
#include "JasonSlice.h"
#include "JasonType.h"

using JasonPrettyDumper = arangodb::jason::JasonStringPrettyDumper;
using JasonSlice        = arangodb::jason::JasonSlice;
using JT                = arangodb::jason::JasonType;
        
JT const JasonSlice::TypeMap[256] = {
  /* 0x00 */  JT::None,        /* 0x01 */  JT::Array,       /* 0x02 */  JT::Array,       /* 0x03 */  JT::Array,        
  /* 0x04 */  JT::Array,       /* 0x05 */  JT::Array,       /* 0x06 */  JT::Array,       /* 0x07 */  JT::Array,
  /* 0x08 */  JT::Array,       /* 0x09 */  JT::Array,       /* 0x0a */  JT::Object,      /* 0x0b */  JT::Object, 
  /* 0x0c */  JT::Object,      /* 0x0d */  JT::Object,      /* 0x0e */  JT::Object,      /* 0x0f */  JT::Object,
  /* 0x10 */  JT::Object,      /* 0x11 */  JT::Object,      /* 0x12 */  JT::Object,      /* 0x13 */  JT::None,     
  /* 0x14 */  JT::None,        /* 0x15 */  JT::None,        /* 0x16 */  JT::None,        /* 0x17 */  JT::None,     
  /* 0x18 */  JT::Null,        /* 0x19 */  JT::Bool,        /* 0x1a */  JT::Bool,        /* 0x1b */  JT::Double,         
  /* 0x1c */  JT::UTCDate,     /* 0x1d */  JT::External,    /* 0x1e */  JT::MinKey,      /* 0x1f */  JT::MaxKey,         
  /* 0x20 */  JT::Int,         /* 0x21 */  JT::Int,         /* 0x22 */  JT::Int,         /* 0x23 */  JT::Int,         
  /* 0x24 */  JT::Int,         /* 0x25 */  JT::Int,         /* 0x26 */  JT::Int,         /* 0x27 */  JT::Int,         
  /* 0x28 */  JT::UInt,        /* 0x29 */  JT::UInt,        /* 0x2a */  JT::UInt,        /* 0x2b */  JT::UInt,        
  /* 0x2c */  JT::UInt,        /* 0x2d */  JT::UInt,        /* 0x2e */  JT::UInt,        /* 0x2f */  JT::UInt,        
  /* 0x30 */  JT::SmallInt,    /* 0x31 */  JT::SmallInt,    /* 0x32 */  JT::SmallInt,    /* 0x33 */  JT::SmallInt,    
  /* 0x34 */  JT::SmallInt,    /* 0x35 */  JT::SmallInt,    /* 0x36 */  JT::SmallInt,    /* 0x37 */  JT::SmallInt,    
  /* 0x38 */  JT::SmallInt,    /* 0x39 */  JT::SmallInt,    /* 0x3a */  JT::SmallInt,    /* 0x3b */  JT::SmallInt,    
  /* 0x3c */  JT::SmallInt,    /* 0x3d */  JT::SmallInt,    /* 0x3e */  JT::SmallInt,    /* 0x3f */  JT::SmallInt,    
  /* 0x40 */  JT::String,      /* 0x41 */  JT::String,      /* 0x42 */  JT::String,      /* 0x43 */  JT::String,      
  /* 0x44 */  JT::String,      /* 0x45 */  JT::String,      /* 0x46 */  JT::String,      /* 0x47 */  JT::String,      
  /* 0x48 */  JT::String,      /* 0x49 */  JT::String,      /* 0x4a */  JT::String,      /* 0x4b */  JT::String,      
  /* 0x4c */  JT::String,      /* 0x4d */  JT::String,      /* 0x4e */  JT::String,      /* 0x4f */  JT::String,      
  /* 0x50 */  JT::String,      /* 0x51 */  JT::String,      /* 0x52 */  JT::String,      /* 0x53 */  JT::String,      
  /* 0x54 */  JT::String,      /* 0x55 */  JT::String,      /* 0x56 */  JT::String,      /* 0x57 */  JT::String,      
  /* 0x58 */  JT::String,      /* 0x59 */  JT::String,      /* 0x5a */  JT::String,      /* 0x5b */  JT::String,      
  /* 0x5c */  JT::String,      /* 0x5d */  JT::String,      /* 0x5e */  JT::String,      /* 0x5f */  JT::String,      
  /* 0x60 */  JT::String,      /* 0x61 */  JT::String,      /* 0x62 */  JT::String,      /* 0x63 */  JT::String,      
  /* 0x64 */  JT::String,      /* 0x65 */  JT::String,      /* 0x66 */  JT::String,      /* 0x67 */  JT::String,      
  /* 0x68 */  JT::String,      /* 0x69 */  JT::String,      /* 0x6a */  JT::String,      /* 0x6b */  JT::String,      
  /* 0x6c */  JT::String,      /* 0x6d */  JT::String,      /* 0x6e */  JT::String,      /* 0x6f */  JT::String,      
  /* 0x70 */  JT::String,      /* 0x71 */  JT::String,      /* 0x72 */  JT::String,      /* 0x73 */  JT::String,      
  /* 0x74 */  JT::String,      /* 0x75 */  JT::String,      /* 0x76 */  JT::String,      /* 0x77 */  JT::String,      
  /* 0x78 */  JT::String,      /* 0x79 */  JT::String,      /* 0x7a */  JT::String,      /* 0x7b */  JT::String,      
  /* 0x7c */  JT::String,      /* 0x7d */  JT::String,      /* 0x7e */  JT::String,      /* 0x7f */  JT::String,      
  /* 0x80 */  JT::String,      /* 0x81 */  JT::String,      /* 0x82 */  JT::String,      /* 0x83 */  JT::String,      
  /* 0x84 */  JT::String,      /* 0x85 */  JT::String,      /* 0x86 */  JT::String,      /* 0x87 */  JT::String,      
  /* 0x88 */  JT::String,      /* 0x89 */  JT::String,      /* 0x8a */  JT::String,      /* 0x8b */  JT::String,      
  /* 0x8c */  JT::String,      /* 0x8d */  JT::String,      /* 0x8e */  JT::String,      /* 0x8f */  JT::String,      
  /* 0x90 */  JT::String,      /* 0x91 */  JT::String,      /* 0x92 */  JT::String,      /* 0x93 */  JT::String,      
  /* 0x94 */  JT::String,      /* 0x95 */  JT::String,      /* 0x96 */  JT::String,      /* 0x97 */  JT::String,      
  /* 0x98 */  JT::String,      /* 0x99 */  JT::String,      /* 0x9a */  JT::String,      /* 0x9b */  JT::String,      
  /* 0x9c */  JT::String,      /* 0x9d */  JT::String,      /* 0x9e */  JT::String,      /* 0x9f */  JT::String,      
  /* 0xa0 */  JT::String,      /* 0xa1 */  JT::String,      /* 0xa2 */  JT::String,      /* 0xa3 */  JT::String,      
  /* 0xa4 */  JT::String,      /* 0xa5 */  JT::String,      /* 0xa6 */  JT::String,      /* 0xa7 */  JT::String,      
  /* 0xa8 */  JT::String,      /* 0xa9 */  JT::String,      /* 0xaa */  JT::String,      /* 0xab */  JT::String,      
  /* 0xac */  JT::String,      /* 0xad */  JT::String,      /* 0xae */  JT::String,      /* 0xaf */  JT::String,      
  /* 0xb0 */  JT::String,      /* 0xb1 */  JT::String,      /* 0xb2 */  JT::String,      /* 0xb3 */  JT::String,      
  /* 0xb4 */  JT::String,      /* 0xb5 */  JT::String,      /* 0xb6 */  JT::String,      /* 0xb7 */  JT::String,      
  /* 0xb8 */  JT::String,      /* 0xb9 */  JT::String,      /* 0xba */  JT::String,      /* 0xbb */  JT::String,      
  /* 0xbc */  JT::String,      /* 0xbd */  JT::String,      /* 0xbe */  JT::String,      /* 0xbf */  JT::String,      
  /* 0xc0 */  JT::Binary,      /* 0xc1 */  JT::Binary,      /* 0xc2 */  JT::Binary,      /* 0xc3 */  JT::Binary,      
  /* 0xc4 */  JT::Binary,      /* 0xc5 */  JT::Binary,      /* 0xc6 */  JT::Binary,      /* 0xc7 */  JT::Binary,      
  /* 0xc8 */  JT::BCD,         /* 0xc9 */  JT::BCD,         /* 0xca */  JT::BCD,         /* 0xcb */  JT::BCD,         
  /* 0xcc */  JT::BCD,         /* 0xcd */  JT::BCD,         /* 0xce */  JT::BCD,         /* 0xcf */  JT::BCD,         
  /* 0xd0 */  JT::BCD,         /* 0xd1 */  JT::BCD,         /* 0xd2 */  JT::BCD,         /* 0xd3 */  JT::BCD,         
  /* 0xd4 */  JT::BCD,         /* 0xd5 */  JT::BCD,         /* 0xd6 */  JT::BCD,         /* 0xd7 */  JT::BCD,         
  /* 0xd8 */  JT::None,        /* 0xd9 */  JT::None,        /* 0xda */  JT::None,        /* 0xdb */  JT::None,        
  /* 0xdc */  JT::None,        /* 0xdd */  JT::None,        /* 0xde */  JT::None,        /* 0xdf */  JT::None,        
  /* 0xe0 */  JT::None,        /* 0xe1 */  JT::None,        /* 0xe2 */  JT::None,        /* 0xe3 */  JT::None,        
  /* 0xe4 */  JT::None,        /* 0xe5 */  JT::None,        /* 0xe6 */  JT::None,        /* 0xe7 */  JT::None,        
  /* 0xe8 */  JT::None,        /* 0xe9 */  JT::None,        /* 0xea */  JT::None,        /* 0xeb */  JT::None,        
  /* 0xec */  JT::None,        /* 0xed */  JT::None,        /* 0xee */  JT::None,        /* 0xef */  JT::None,        
  /* 0xf0 */  JT::Custom,      /* 0xf1 */  JT::Custom,      /* 0xf2 */  JT::Custom,      /* 0xf3 */  JT::Custom,        
  /* 0xf4 */  JT::Custom,      /* 0xf5 */  JT::Custom,      /* 0xf6 */  JT::Custom,      /* 0xf7 */  JT::Custom,        
  /* 0xf8 */  JT::Custom,      /* 0xf9 */  JT::Custom,      /* 0xfa */  JT::Custom,      /* 0xfb */  JT::Custom,        
  /* 0xfc */  JT::Custom,      /* 0xfd */  JT::Custom,      /* 0xfe */  JT::Custom,      /* 0xff */  JT::Custom
}; 
       
unsigned int const JasonSlice::WidthMap[0x13] =
  { 0,   // 0x00, None
    1,   // 0x01, empty array
    1,   // 0x02, array without index table
    2,   // 0x03, array without index table
    4,   // 0x04, array without index table
    8,   // 0x05, array without index table
    1,   // 0x06, array with index table
    2,   // 0x07, array with index table
    4,   // 0x08, array with index table
    8,   // 0x09, array with index table
    1,   // 0x0a, empty object
    1,   // 0x0b, object with sorted index table
    2,   // 0x0c, object with sorted index table
    4,   // 0x0d, object with sorted index table
    8,   // 0x0e, object with sorted index table
    1,   // 0x0f, object with unsorted index table
    2,   // 0x10, object with unsorted index table
    4,   // 0x11, object with unsorted index table
    8    // 0x12, object with unsorted index table
  };

unsigned int const JasonSlice::FirstSubMap[0x13] =
  { 0,   // 0x00, None
    1,   // 0x01, empty array
    2,   // 0x02, array without index table
    3,   // 0x03, array without index table
    5,   // 0x04, array without index table
    9,   // 0x05, array without index table
    3,   // 0x06, array with index table
    5,   // 0x07, array with index table
    8,   // 0x08, array with index table
    8,   // 0x09, array with index table
    1,   // 0x0a, empty object
    3,   // 0x0b, object with sorted index table
    5,   // 0x0c, object with sorted index table
    8,   // 0x0d, object with sorted index table
    8,   // 0x0e, object with sorted index table
    3,   // 0x0f, object with unsorted index table
    5,   // 0x10, object with unsorted index table
    8,   // 0x11, object with unsorted index table
    8    // 0x12, object with unsorted index table
  };

std::string JasonSlice::toString () const {
  return JasonPrettyDumper::Dump(this);
}
        
std::ostream& operator<< (std::ostream& stream, JasonSlice const* slice) {
  stream << "[JasonSlice " << JasonTypeName(slice->type()) << ", byteSize: " << slice->byteSize() << "]";
  return stream;
}

std::ostream& operator<< (std::ostream& stream, JasonSlice const& slice) {
  stream << "[JasonSlice " << JasonTypeName(slice.type()) << ", byteSize: " << slice.byteSize() << "]";
  return stream;
}

