An outline of the packages that comprise the standard library.

## Global

Always visible by default in `Object`.

 * `Global` mixin, including core methods like `require`
 * classes for literals: `String`, `Integer`, `Float`, `Boolean`, `Block`, `Symbol`.
 * `Object` and `Bare` parent classes.
 * `FileScope` class, an instance of which is available as the parent scope in any source file.

## Collections

Always visible by default.

 * `Pair`, a key-value pair
 * `List`, a facade class
  * `Array` used for "*" splats.
  * `LinkedList`
 * `SortedList`
 * `Map`, a facade class
  * `LinkedMap` used for "**" splats.
  * `HashMap`
  * `SymbolMap`
 * `BloomFilter`

## IO

 * `filesystem`
 * `path`
 * `socket`

### HTTP

 * `Url`

## Package

 * `Package` model.
 * `semver` for reasoning about and comparing semantic versions.
 * `manifest` for describing package manifests.
 * `fetch` for downloading packages.
 * `install` for installing packages.
 * `init` for initializing new packages.

## REPL

 * `zz`

## Parser

Expose the parser API to programmatic manipulation.

## Formatter

Walk an AST and generate a well-formatted, canonical source code
representation.
