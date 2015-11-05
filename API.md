API documentation for the VPack libraries
=========================================

## Buffer

## Builder

This class organizes the buildup of a VPack object. It manages
the memory allocation and allows convenience methods to build
the object up recursively.

Use as follows, to build a VPack like on the right hand side:

```cpp
Builder b;
b.add(Value(5, ValueType::Object));    b = {
b.add("a", Value(1.0));                      "a": 1.0,
b.add("b", Value());                         "b": null,
b.add("c", Value(false));                    "c": false,
b.add("d", Value("xyz"));                    "d": "xyz",
b.add("e", Value(3, ValueType::Array));      "e": [
b.add(Value(2.3));                                   2.3,
b.add(Value("abc"));                                 "abc",
b.add(Value(true));                                  true
b.close();                                        ],
b.add("f", Value(2, ValueType::Object));     "f": {
b.add("hans", Value("Wurst"));                       "hans": "wurst",
b.add("hallo", Value(3.141));                        "hallo": 3.141
b.close();                                        }
```

Or, if you like fancy syntactic sugar:

```cpp
Builder b;
b(Value(5, ValueType::Object))             b = {
 ("a", Value(1.0))                          "a": 1.0,
 ("b", Value())                             "b": null,
 ("c", Value(false))                        "c": false,
 ("d", Value("xyz"))                        "d": "xyz",
 ("e", ValueType::Array, 3)                 "e": [
   (Value(2.3))                                    2.3,
   (Value("abc"))                                 "abc",
   (Value(true))()                                true ],
 ("f", ValueType::Object, 2)                "f": {
 ("hans", Value("Wurst"))                          "hans": "wurst",
 ("hallo", Value(3.141)();                         "hallo": 3.141 }
```

## Slice

## Parser

```cpp
Parser p;
std::string const json = "{\"a\":12}";
try {
  size_t nr = p.parse(json);
}
catch (std::bad_alloc const& e) {
  std::cout << "Out of memory!" << std::endl;
}
catch (Exception const& e) {
  std::cout << "Parse error: " << e.what() << std::endl;
  std::cout << "Position of error: " << p.errorPos() << std::endl;
}
Builder b = p.steal();

// p is now empty again and ready to parse more.
```

## Dumper
