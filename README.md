# abc-playground

Personal playground for ABC (System for Sequential Logic Synthesis and Formal Verification)

### Prerequisites

The `gcc` alias on macOS is actually `clang`. For compatibility, I choose to use GNU `gcc` instead of `clang` provided by macOS.
So first, I need to install `gcc` using `homebrew`.

```bash
brew install gcc@6
```

Since I am also using `filesystem` in Boost C++ Libraries, I also need to install `boost`
via `homebrew`. Make sure that `--cc=gcc-6` flag is added for the installation command. 
Otherwise `clang` compiled version will be installed and strange mistakes will happen.

```bash
brew install boost --cc=gcc-6
```


### Reference

Berkeley Logic Synthesis and Verification Group, ABC: A System for Sequential Synthesis and Verification. http://www.eecs.berkeley.edu/~alanmi/abc/
