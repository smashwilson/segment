Most programmers, at some point, daydream about writing their own programming language. This one's mine.

## What's all this then

**Segment** is a dynamic, interpreted language in the spirit of Ruby and Python. It's a little closer to Smalltalk than Ruby: notably, general-purpose blocks are a first-class concept, and it has no built-in control structures or keywords.

Dependency management is a central concept, used to version the standard library. Method lookup is dependent on receiver class and lexical scope, both preventing "dependency hell" - two packages can depend on different versions of a common dependency without conflict - and offering a class extension facility similar to Ruby's refinements.

## Features

Considering that the interpreter doesn't even have a `main` yet, this is all pure speculation.

 * Proper keyword arguments.
 * Inheritance by mixin, which, okay, I'm basically lifting wholesale from Ruby. Although I did have some thoughts about isolating instance variables within each mixin, and I'm going to try to keep conventional subclassing out of the language, if I can.
 * No `null`, `nil`, or `nothing`! There will be an `Optional` mixin in the core library, included in the `Some` and `None` classes, with handy methods for dealing with presence or absence, or iterating like a one-element collection.
 * Define your own operators and operator-like methods. Operator methods end with one of the standard `*`, `+`, `/` operator characters and have the precedence of their trailing character. This will let me write an `int/` method, for example.
 * More standard data structures than `Hash` and `Array`. I'll likely steal something like Scala's map construction syntax.
 * Unicode support from the get-go, because it's a giant pain to roll it in later.
 * Garbage collection with a proper generational garbage collector, because I've always wanted to write one. Statistics and functionality exposed through an in-language API.
 * Maybe JIT compilation with LLVM.

## Great, But What Does It Look Like?

Define classes and methods with the `class` and `method` methods:

```
class :Box {
  constructor { |value|
    @value = value
  }

  def :value { @value }

  # Which is really just a method call:
  # self.def(:value, { @value })
}

%box = Box.new(10)
puts %box.value()  # => 10

# The %-sigil is for temporary variables.
# It closes that weird Ruby syntax hole where:
#
#   foo = 10
#
# ... could mean either "assign 10 to foo", or "call self.foo=(10)".

if (%box.value() == 10) then: {
  puts "Hooray!"
} else: {
  puts "Oh no!"
}

# Which is really the method call:
self.if(%box.value().==(10), { puts("Hooray!") }, { puts("Oh no!") })
```

See the [examples](examples/) directory for more samples of what it'll look like.

## Getting Started

If you're on a Mac with Homebrew, or a Linux box with apt, you can clone it and run:

```bash
script/bootstrap
```

If you don't have those things, [`script/bootstrap`](script/bootstrap) is good documentation on what dependencies you'll need to install on your platform of choice. I'm using [lemon](http://www.hwaci.com/sw/lemon/) as a parser generator and [ragel](http://www.complang.org/ragel/) for the lexer.

Once you've got that, build it with:

```bash
make
```

...and watch it break. :wink:
