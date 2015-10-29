API documentation for the Jason libraries
=========================================

## JasonBuffer

## JasonBuilder

This class organizes the buildup of a Jason object. It manages
the memory allocation and allows convenience methods to build
the object up recursively.

Use as follows, to build a Jason like on the right hand side:

```cpp
JasonBuilder b;
b.add(Jason(5, JasonType::Object));    b = {
b.add("a", Jason(1.0));                      "a": 1.0,
b.add("b", Jason());                         "b": null,
b.add("c", Jason(false));                    "c": false,
b.add("d", Jason("xyz"));                    "d": "xyz",
b.add("e", Jason(3, JasonType::Array));      "e": [
b.add(Jason(2.3));                                   2.3,
b.add(Jason("abc"));                                 "abc",
b.add(Jason(true));                                  true
b.close();                                        ],
b.add("f", Jason(2, JasonType::Object));     "f": {
b.add("hans", Jason("Wurst"));                       "hans": "wurst",
b.add("hallo", Jason(3.141));                        "hallo": 3.141
b.close();                                        }
```

Or, if you like fancy syntactic sugar:

```cpp
JasonBuilder b;
b(Jason(5, JasonType::Object))        b = {
 ("a", Jason(1.0))                          "a": 1.0,
 ("b", Jason())                             "b": null,
 ("c", Jason(false))                        "c": false,
 ("d", Jason("xyz"))                        "d": "xyz",
 ("e", JasonType::Array, 3)                 "e": [
   (Jason(2.3))                                    2.3,
   (Jason("abc"))                                 "abc",
   (Jason(true))()                                true ],
 ("f", JasonType::Object, 2)                "f": {
 ("hans", Jason("Wurst"))                          "hans": "wurst",
 ("hallo", Jason(3.141)();                         "hallo": 3.141 }
```

## JasonSlice

## JasonParser

```cpp
JasonParser p;
std::string const json = "{\"a\":12}";
try {
  size_t nr = p.parse(json);
}
catch (std::bad_alloc const& e) {
  std::cout << "Out of memory!" << std::endl;
}
catch (JasonException const& e) {
  std::cout << "Parse error: " << e.what() << std::endl;
  std::cout << "Position of error: " << p.errorPos() << std::endl;
}
JasonBuilder b = p.steal();

// p is now empty again and ready to parse more.
```

## JasonDumper
