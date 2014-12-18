make\_pcre is a plugin for GNU make providing ability to use Perl compatible
regular expressions. It requires make 4.0 or higher (tested with 4.0 and 4.1)
and libpcre3 (tested with 8.12 and 8.30).

[![Build Status](https://travis-ci.org/mikhirev/make_pcre.svg)](https://travis-ci.org/mikhirev/make\_pcre)

Installation
============

Prerequisites
-------------

- GNU make 4.x+
- libpcre3 8.x+

Build
-----

To get plugin built, simply type

    $ make

in source directory. Optionally, type

    $ make check

to run self-tests. Please report me if it fails on your system.

Install
-------

Copy `pcre.so` into the directory where your makefile is stored.

Usage
=====

Load the plugin by adding

    load pcre.so

to your makefile.

The plugin provides two functions: `pcre_find` (with shorthand `m`)
and `pcre_subst` (with shorthand `s`).

### `pcre_find` function ###

`pcre_find` is similar to builtin `findstring` function, but it takes PCRE
pattern instead substring as first argument:

    $(pcre_find PATTERN,IN)
    $(m PATTERN,IN)

It searches *IN* for matching *PATTERN*. If it occurs, the matched substring is
returned; otherwise returned string is empty. Note that normally *PATTERN* is
not expanded, but *IN* is expanded before search.

### `pcre_subst` function ###

`pcre_subst` is similar to builtin `subst` function, but it takes PCRE pattern
instead substring as first argument:

    $(pcre_subst PATTERN,REPLACEMENT,TEXT)
    $(s PATTERN,REPLACEMENT,TEXT)

It searches *TEXT* for matching *PATTERN*. If it occurs, the matched substring
is replaced with *REPLACEMENT*, and the resulting string is returned. If pattern
does not match, *TEXT* is returned unchanged. *PATTERN* is not expanded by default,
*TEXT* is expanded before search, and *REPLACEMENT* string is expanded just before
substitution.

Capturing strings
-----------------

### Capture by number ###

When matching found, both `pcre_find` or `pcre_subst` set variable `$(0)`
to the whole matched string and variables `$(1)`, `$(2)`, ... to substrings
captured by round brackets (like Perl does). Maximum number of strings that can
be captured is 256 (`$(0)` to `$(255)`). These variables can be used until
the next `pcre_find` or `pcre_subst` call that will reset them.

### Capture by name ###

`make_pcre` also provides ability to set named variables to matched substrings.
It can be useful if you want to preserve captured value after another matching
function called or if your pattern is quite complicated, and it is difficult
to handle substring numbers.

In PCRE, a subpattern can be named in one of three ways: `(?<name>...)`
or `(?'name'...)` as in Perl, or `(?P<name>...)` as in Python. If pattern
matches, variable `$(name)` will be set to matched substring.

Options
-------

Both `pcre_find` and `pcre_subst` can take an optional argument consisting
of one ore more characters, each of which enables some option:

    $(pcre_find PATTERN,IN,EgimsuUxX8)
    $(m PATTERN,IN,EgimsuUxX8)
    $(pcre_subst PATTERN,REPLACEMENT,TEXT,EgimsuUxX8)
    $(s PATTERN,REPLACEMENT,TEXT,EgimsuUxX8)

The following options are implemented:

- `E` enables expansion of pattern before compilation. Note that you will need
  to use `$$` instead `$` for matching end of line in this case;
- `g` enables global search, like in Perl. `pcre_find` will return space
  separated list of all matched substrings; `pcre_subst` will replace all
  matched substrings expanding replacement string before each substitution;
- `i` makes search case insensitive. The same as in Perl;
- `m` makes regexp treating string as multi-line, i. e. `^` and `$` will match
  immediately after or immediately before internal newlines. The same
  as in Perl;
- `s` forces `.` metacharacter to match any character including newline.
  The same as in Perl;
- `u` changes the way of processing `\B`, `\b`, `\D`, `\d`, `\S`, `\s`, `\W`,
  `\w` and some of the POSIX character classes forsing them to use Unicode
  properties;
- `U` ungreedies quantifiers by default (they still can be made greedy
  if followed by `?`);
- `x` forces regexp to ignore unescaped whitespaces and comments after `#`.
  The same as in Perl;
- `X` enables extra PCRE functionality making the pattern incompatible to Perl.
  See PCRE documentation for additional information;
- `8` makes both pattern and subject string treated as UTF8.

See also
--------

See `pcrepattern(3)` and `pcresyntax(3)` man pages for more information
on PCRE pattern syntax.
