# Merge drivers for Git

This repository contains a (small) collection of custom Git merge drivers.
Indeed Git is able to call external process to proceed 3-way merge.
This feature allows us to implement per format merge procedures instead of
using text merge. It's especially interesting when you are using Git to
store and maintain serialized data and objects (e.g. JSON, YAML).

## Drivers

There are currently to driver implemented: JSON and YAML.
Even YAML 1.2 is JSON compliant since YAML is a superset of JSON,
I'm using 2 libraries: [libjson-cpp](https://github.com/nlohmann/json)
and [libyaml-cpp](https://github.com/jbeder/yaml-cpp).

There are many reasons of that. The first implementation in JSON was using
libjson-cpp (from https://github.com/nlohmann/json). Functions patch and diff
were already implemented. I used them to do 3-way merge. By the way, when
your are doing JSON, you don't need/want to do YAML.

### JSON driver

JSON driver is using a ["Modern C++ JSON library"](https://github.com/nlohmann/json)
to parse and generate JSON. It's using patch and diff functionnality to generate
3-way merge. I'm substracting "older" from "other" JSON (using diff) and I apply
it to "current" JSON (using patch).

The standard format use for diff and patch is describe here:
[https://tools.ietf.org/html/rfc6902](https://tools.ietf.org/html/rfc6902)

### YAML driver

I (partially) implemented the same patch format for YAML then
[JSON patch](https://tools.ietf.org/html/rfc6902).
Basically, it's using the same algorithm then JSON driver.

## Credits

 - Idea inspired from work of [Jonatan Pedersen](https://github.com/jonatanpedersen/git-json-merge).
 - YAML diff and patch implementation inspired from [JSON library](https://github.com/nlohmann/json).

## License

    Collection of merge drivers for Git.
    Copyright (C) 2016  Raphael Medaer (Escaux) <rme@escaux.com>

    git-merge-drivers is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    git-merge-drivers is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with git-merge-drivers.  If not, see <http://www.gnu.org/licenses/>.
