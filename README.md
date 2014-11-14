make\_pcre is a plugin for GNU make providing ability to use perl compatible
regular expressions. It requires make 4.0 or higher (tested with 4.1)
and libpcre3 (tested with 8.30).

Installation
============

Prerequisites
-------------

- GNU make 4.0+
- libpcre3 8.30+

Build
-----

To get plugin built, simply type

    $ make

in source directory. Optionally, type

    $ make check

to run self-tests.

Install
-------

Copy pcre.so into the directory where your makefile is stored.

Usage
=====

Currently only one function `pcre_find` (with shorthand `m`) is implemented.
It is similar to builtin `findstring` function, but it takes PCRE pattern
instead substring as first argument:

    $(pcre_find PATTERN,IN)
    $(m PATTERN,IN)

It searches IN for matching PATTERN. If it occurs, the matched substring is
returned; otherwise returned string is empty. Note that normally PATTERN is
not expanded, but IN is expanded before search.

Capturing strings
-----------------

When matching found, `pcre_find` sets variable `$(0)` to whole matched string
and variables `$(1)`, `$(2)`, ... to substrings captured by round brackets
(like perl does). Maximum number of strings that can be captured is 256 (`$(0)`
to `$(255)`). These variables can be used until the next `pcre_find` call
because it will reset them.

Options
-------

`pcre_find` can take optional third argument consisting of one ore more
characters, each of which enables some option:

    $(pcre_find PATTERN,IN,emsuUxX8)
    $(m PATTERN,IN,emsuUxX8)

The following options are implemented:

- `e` enables expansion of pattern before compilation. Note that you will need
  to use `$$` instead `$` for matching end of line in this case;
- `m` makes regexp treating string as multi-line, i. e. `^` and `$` will match
  immediately after or immediately before internal newlines. The same
  as in Perl;
- `s` forces `.` metacharacter to match any character including newline.
  The same as in Perl;
- `u` changes the way of processing \B, \b, \D, \d, \S, \s, \W, \w and some
  of the POSIX character classes forsing them to use Unicode properties;
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
