# New to trema-edge directory structure

New to trema-edge are the following subdirectories containing implementation files
for the Ruby controller. Because retrospection is used to dynamically dispatch methods
or define accessor attributes the contents of those files should not carelessly modified.
All classes can be referenced using by prefixing the module name or by just their name.
For example `Trema::Instructions::WriteAction` is equivalent to `WriteAction`.

## Messages
All messages that the Ruby controller supports can be found under
the directory `messages`. Messages are logically separated into requests and
replies. For each message there is a ruby file that holds the attributes that 
identifies the message. This message object can be instantiated by calling
its constructor supplying a hash of key/value arguments. The message C file
provides pack and unpack functions to be able to transmit and receive the message
to/from lower layers.

## Actions
This directory contains a list of all of what we call basic and flexible actions.
The basic actions map to actions of type `OFPAT_xxx` and flexible actions are
the extensible actions of `OXM_xxx` type.
Action objects can be instantiated and passed as argements to instructions or
match objects.

## Instructions
This directory contains a list of all instructions. Please note although the
`meter` instruction is defined its implementation is not supported.
