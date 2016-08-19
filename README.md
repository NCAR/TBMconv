© 2016 University Corporation for Atmospheric Research

This work was performed under the auspices of the National Center for
Atmospheric Research (NCAR) Earth Observing Laboratory (EOL) Summer
Undergraduate Projects in Engineering Research (SUPER) program, which is
managed by the University Corporation for Atmospheric Research (UCAR) and is
funded by the National Science Foundation (NSF) (www.eol.ucar.edu).

# NCAR Terabit Memory System

The NCAR Terabit Memory System was a tape storage system used at NCAR from
approximately the mid-1970s to the mid-1980s. It was based around the
Ampex TMS-4 Terabit Memory System, and included software which ran on a
PDP/11 and CDC 6600/7600. Data stored on high-density TMS-4 tapes was stored
in the TBM format; this repository includes utilities to examine and extract
the contents of TBM archives.

### Getting the source

```
$ git clone https://github.com/NCAR/TBMconv.git
```

### Compiling

`tbmconv` and `tbmexplore` have no external dependencies. Compile with:

```
$ cd TBMconv
$ make
```

### Documentation

A PDF document describing the TBM file format is available as a part of this
project: [tbm.pdf](https://ncar.github.io/TBMconv/files/tbm.pdf).
