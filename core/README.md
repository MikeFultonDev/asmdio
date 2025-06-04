# Core Code

The core contains the code that provides low-level (core) functions used
by higher-level (services) functions.
The core functions are not typically called directly from user code.

## Build, Test, Clean

- Build with: `gmake -f Makefile`
- Test with: `gmake -f Makefile check`
- Clean generated files with: `gmake -f Makefile clean`
