# This file is kept in sync with both the website and git, this is
# why a preprocessing step is required to get rid of links to
# doc/* when producing the same document for git instead of
# the website

{{Quick starting guide}}

{Picobat} is an open source command prompt meant to implement a superset of
{cmd.exe} script language (known as {batch} by most developers). It is
designed to be as simple to use as possible, reliable, portable and
lightweight.

{{Getting started with Picobat}}

Firstly, if you have never used cmd or an equivalent interpreter before, you
should definitely consider reading a tutorial about batch programming. On the
other hand if you are already familiar with batch, just open {pbat} and
start typing some commands ! Picobat batch dialect is almost compatible with cmd.

If you use Picobat on windows, the only file extension provided is {.pbat} by
default (to avoid trashing your OS with potential conflicts). However,
calling a {.bat} or {.cmd} script from inside Picobat, leads them to get
executed by Picobat.

{{Downloading Picobat}}

The latest release of picobat is available {https://github.com/darkbatcher/picobat/releases|here}.
Extract the archive and start playing with {pbat} in the extracted folder.

{{Picobat changes}}

Once installed, it is quite straightforward to play with Picobat, especially if
you are quite familiar with cmd. However, there is a couple of thing out there
you have to know about.

On one hand, there is a couple of differences between Picobat and cmd :

- The {doc/help|HELP} system is different from the original cmd help system. On the
  first time you run the command, you need to genérate the whole documentation
  using:

${help /b}

@- Once all the data has been generated you can use it as the {help} command,
  but beware, it's fairly improved.

- There is a few {doc/for|FOR} modifications:

  -- Empty line are processed by {doc/for|FOR /F}. This behaviour can be disabled
    using:

${SETLOCAL EnableCmdlyCorrect}

  -- Multiples lines can be used as input for {FOR /F}.

  -- Tokens can be specified in reverse order and can also overlap without bug.

- Some undocumented but useful variables from cmd are still lacking (Though
  {%=EXITCODEASCII%} is now supported).

- Picobat has no inconsistencies with escaped characters with {^}, it requires
  only one escape.

- {doc/dir|DIR /b} does not automatically return absolute paths.

- Support of {doc/start|START} is a bit tricky under some platforms and some of the
  options or the whole command might not be provided depending on your system
  configuration if you use *nix.

On the other hand, Picobat also provides you with with a bunch of extensions:

- A full set of extensions to support floating points arithmetics through
  {doc/set|SET} and {doc/if|IF}:

  -- New {doc/seta|SET /a} commands extensions to perform operations on
     floating point numbers.

  -- Extensions for {IF} command to automatically detect floating-point
  and compare floating-point numbers.

- The amazing possibility to define functions and procedures in batch using
 the Picobat specific {doc/def|DEF} command:

${DEF ADD=(set /a $1=$2+$3)
ADD result 4 3
ECHO result = %result%}

- The ground-breaking possibility to specify logical expressions using
  {AND} and {OR} and the new {doc/if|IF} extensions, like in the following
  example:

${IF [ [ !ok! EQU 1 ] and [ defined file ] ] (
 :: some code
)}

- A module system allowing extension loading at run-time. This offer numbers
  of possibilities from graphical user interfaces to networking extensions.
  Currently, the only module provided is the {doc/batbox|BATBOX} module.

- To speed up parsing, Picobat loads files entirely in memory at startup. If the
  file gets modified during its execution, it is reloaded and Picobat restarts on
  the next line (counting lines from the begining). This can also be disabled
  using:

${SETLOCAL EnableCmdlyCorrect}

- As can be seen in the previous code, {::-style} comments can be used
  inside {doc/spec/cmdline|blocks}.

- All the {doc/for|FOR} modifications described above.

- Enhanced {doc/goto|GOTO} and {doc/call|CALL} that can use a file and a label at the same
  time and ignore errors:

${GOTO :mylabel myfile.bat /Q
CALL /e :mylabel myfile}

- Extended {doc/help|HELP} providing search capabilities and help in various formats
  including {HTML}.

- An extension to the {doc/find|FIND} command to use simple regular expressions:

${echo match my regular expression | FIND /e "match * regular expression"}

- The {doc/find|FIND} and {doc/more|MORE} commands are provided as internal commands.

- New internal commands {doc/xargs|XARGS} and {doc/wc|WC} inspired from their *nix
  counterparts:

  -- {doc/xargs|XARGS} runs a command taking command parameters on the standard input.

  -- {doc/wc|WC} counts line or words or bytes in a file.

${:: Count lines in subdirs
dir /s /b /a:-D . | xargs wc /l}

- New external command {doc/dump|DUMP} to dump hexadecimal code.

- New {doc/shift|SHIFT} extensions and new {%+} variable containing the remaining
  arguments.

{{Troubleshooting}}

If you have some questions about or need some help with Picobat, please feel free
to join {Picobat}'s official discord at:
{https://discord.gg/w4KtdCv|https://discord.gg/w4KtdCv}
