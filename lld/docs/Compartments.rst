Compartments
============

.. warning::

   This feature is experimental and its interface will change over
   time.

LLD permits dividing an ouput file (either an executable or a shared
library) into one or more compartments.  Each compartment contains
its own text and data output sections, as well as GOTs and PLTs.
Other sections such as symbol tables, program headers, and debug
information are not split into per-compartment sections.  The
placement of input text and data sections into compartments is
dictated by one or more policy files supplied as additional inputs
to LLD.

Usage
-----

Compartment policy files are supplied on the command line via the
``--compartment-policy=<filename>`` flag, where ``<filename>`` is
the name of a policy file.  The flag may be specified multiple
times to include policies from multiple files.  All of the
policy fragments from individual policies are merged to generate
the effective policy.  If a policy contains contradictory requirements either within a single policy file or across multiple policy files, LLD will report an error.  Policy files are written
as JSON objects which define two distinct properties: compartment composition and access control.

Compartment Composition
```````````````````````

The first property in a policy object is the ``compartments`` object.  The object is a map of compartment definitions.  For each compartment definition, the property name (which must be non-empty) is the name of the compartment.  The value of a compartment property is a tuple containing one or more of the following named arrays.  The compartment contains the union of all of the symbols described by these arrays.  Each array lists a collection of things that have the same type, and individual entries in each array are evaluated as regular shell globs.  The ``symbols`` array contains a list of ELF symbol names.  The ``files`` array contains a list of static linkage input files (object files).  An entry in the ``files`` array is matched against the basename of input files if the entry does not contain any path separator characters; otherwise it is matched against whatever path is provided on the linker command line.

Sample syntax::

  "compartments": {
    "crc32": { "symbols": ["calculate_crc32c"] },
    "one": { "files": ["one.*o", "oneb*.o"] },
    "two": { "symbols": ["counter_str"] }
  }


.. note::

   The static linker assigns input sections from object files to
   compartments.  It is a link time error to assign symbols from the
   same input section to different named compartments.  However, if
   a symbol is defined in an input section but not explicitly
   assigned to a compartment, it will be assigned to whatever
   compartment contains that input section.  If no symbols in an
   input section are assigned to a compartment, the input section is
   assigned to the "default" compartment.  Since the static linker
   can only work at the granularity of input sections, symbols that
   the policy wishes to place in different compartments must reside
   in different input sections.  This can be accomplished either by
   refactoring source code or using compiler flags such as
   ``-ffunction-sections`` and ``-fdata-sections``.

Access Control
``````````````

The access control rules in a policy constrain inter-compartment access within an executable or shared library.  Symbols within a given compartment are always allowed to access other symbols within the same compartment.  In fact, this assumption is encoded in code generation by the compiler in that the compiler may elide indirection for accesses to symbols within the same input section.

Rule Syntax
'''''''''''

An ACL rule encodes a subject (a compartment that wishes to perform an access), one or more objects (the target symbol[s] being accessed) and a mask of permissions.  The current mask of permissions are ``r``, ``w``, and ``x``.  Execute permission is required to access function symbols.  For data symbols, at least one permission is required.  For CHERI architectures, the mask of permissions is ANDed with the permissions derived from the symbol type and section to determine the effective permissions for a GOT entry.  Objects may be named either as individual symbols or, to
permit compression, compartments.  If a compartment is used as an object, the rule applies to all symbols belonging to the named compartment.


The ACL is encoded as an array of rules in a top-level ``acls`` property.  The subject of a rule is specified in a ``subject`` property whose value is a compartment name.  The ``permissions`` object is a string containing the set of granted permissions. Objects of a rule are specified in a pair of properties each of which holds an array: ``compartments`` is an array of compartment names, and ``symbols`` is an array of symbol names.  At least one object must be specified for each rule.  Compartment and symbol names in rule subjects and objects are evaluated as regular shell globs.

Rule Evaluation
'''''''''''''''

Note that the ACL only constrains access from compartments defined in the shared library to other symbols (including both symbols in the same shared library and undefined symbols resolved at runtime). The ACL does not impose restrictions on how other shared libraries or executables might access symbols exported by this shared library.

To facilitate incremental adoption of ACL rules, if a compartment does not match the subject of any rules in the ACL, all accesses from that compartment are permitted. If at least one rule does match the compartment, then the ACL rules will constrain all accesses from that compartment.  To restrict a compartment to no external access, a single ACL rule with an empty permissions string and an object with a symbol glob of ``*`` may be used.

When evaluating the ACL for a specific access, the union of all explicit ACLs is used to determine the effective permissions. If no ACL describes the access and the compartment matches at least one ACL rule, the access is denied.

Sample Syntax::

  "acls": [
    { "subject": "two", "permissions": "r", "symbols": ["counter"] },
    { "subject": "*", "permissions": "x",
      "compartments": ["crc32", "one", "two"] }
  ]

Here, the first rule permits functions in the ``two`` compartment to read the value of the ``counter`` symbol defined in the ``one`` compartment.  The second rule permits other compartments in the same library to call functions in the ``crc32``, ``one``, and ``two`` compartments.  Since no other rules are given for other compartments, those other compartments may not directly read or write symbols defined in any other compartments.

The "default" Compartment
`````````````````````````

Symbols that are not assigned to an explicit compartment as well as certain output sections generated by the static linker for use by the runtime loader (such as ``.dynamic``) belong to the "default" compartment.  Shared objects or executables linked without an explicit policy place all symbols in the default compartment.  The policy used for the "default" compartment is not yet clearly defined.  It is an error to use the default compartment's name (which is the empty string) in the ``compartments object``.  ACL rules can match the default compartment either by the wildcard glob of ``*`` or the explicit name of an empty string.

Restrictions
------------

This feature is currently only supported in the ELF linker.

Compartments are only lightly tested with linker scripts.

The current implementation only supports up to 254 compartments due
to implementation limitations.  This limit may be relaxed in the
future.
