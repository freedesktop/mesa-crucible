Crucible: a Vulkan test suite
=============================

  cru·ci·ble
  ˈkro͞osəb(ə)l/
  noun

  [literal] a ceramic or metal container in which metals or other substances
  may be melted or subjected to very high temperatures.

  [symbolic] a place or occasion of severe test or trial.
  ex: "the crucible of combat"


Feature Summary
---------------

- Crucible requires no window system, as all tests run headless. To allow
  inspection of test images, tests may optionally dumping them to PNG files.
  (See option '--dump' in crucible-run.1.txt).

- The test runner and the tests themselves reside in different processes. This
  protects the test runner from crashing tests.

- Crucible can run each test as a separate process, providing better isolation
  between tests and thus improving reproducibility of results. (See option
  '--isolation' in crucible-run.1.txt).

- Crucible can also run each test as a separate thread in a single process,
  providing better testing of concurrent use of Vulkan. (See option
  '--isolation' in crucible-run.1.txt).

- Tests are based on comparison of reference images to actual images.
  Most reference images are PNG files committed into the source repository or
  generated at build-time. Some are generated at runtime with CPU rasterization.

- Tests can optionally dump their reference images and actual images to PNG
  files to a temporary directory, allowing easy inspection when debugging
  failing tests.  (See option '--dump' in crucible-run.1.txt).


Supported Platforms
-------------------

- Crucible targets version 0.138.2 of vulkan.h.

- Crucible requires a C11 compiler. This language requirement may be lowered to
  C99 for the sake of portability. (The language requirement will *not* be
  lowered to C90).

- Operating Systems
    - Linux: Crucible has been exclusively developed on Fedora 22. Porting to other
      Linux distributions should require only minimal effort consisting of
      generalizing some header and library names. Crucible is independent of window
      system.

    - Other Unixen: Porting to Mac OS X or Android should be straightforward, as
      Crucible mostly conforms to POSIX.

    - Non-Unixy: Patches welcome.

- Hardware and Drivers
    - Intel: Crucible has been exclusively developed against Intel's Mesa
      drivers for Broadwell and Ivybridge. (That is, the drivers directly
      developed by Intel, not by LunarG).

    - Other: Not yet tested.


Contributing
------------

Crucible uses merge requests in https://gitlab.freedesktop.org/mesa/crucible
for code review.

Read the HACKING file for information about how the code is organized.


License
-------

Crucible is opensource software. See the LICENSE file for details.
