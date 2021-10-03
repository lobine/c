# C

Repository to dump my Code.

## Basics & utils

[c.h](c.h)

Define type aliases, memory allocation macros and a couple of utilitary functions. Hopefully the only place that imports third party libraries.

## String builder

[string\_builder.h](string_builder.h)

A simple string builder implementation. The builder (`string_make_builder`) allocates a buffer, that can be filled (`string_write_XXX`) and converted to a normal string (`string_copy_builder`, `string_builder_to_c`).

## JSON parser

[json.h](json.h)

A JSON parser. Requires implementing callback functions that get called recursively for objects and arrays.
