# Meld

A project for exploring how to meet DUNE's framework needs.

## Motivation

Existing data-processing frameworks for HEP experiments are largely based on
collider-physics concepts, which may be based on rigid, event-based data hierarchies.
These data organizations are not always helpful for neutrino experiments, which must
sometimes work around such restrictions by manually splitting apart events into constructs
that are better suited for neutrino physics.

The purpose of Meld is to explore more flexible data organizations by treating a
frameworks job as:

1. A graph of data-product sequences connected by...
2. User-defined functions that serve as operations to...
3. Framework-provided higher-order functions.

Each of these aspects is discussed below:

- [Data-centric graph processing](https://github.com/knoepfel/meld/wiki/Data-centric-graph-processing)
- [Higher-order functions](https://github.com/knoepfel/meld/wiki/Higher-order-functions)

## Building with MPD

Setting up your Spack installation (once per machine)

```console
$ cd <some scratch area>
$ wget https://github.com/FNALssi/fermi-spack-tools/raw/refs/heads/fnal-develop/bin/bootstrap
$ bash bootstrap $PWD/spack-fnal
```

Setting up MPD for the Spack installation (once per Spack installation)

```console
$ source <some scratch area/>spack-fnal/share/spack/setup-env.sh
$ spack mpd init
```

Creating a new MPD project

```console
$ source <some scratch area/>spack-fnal/share/spack/setup-env.sh
$ spack mpd new-project --name meld-devel -T <some/path/to/>meld-devel cxxstd=20 %gcc@11
$ spack mpd clone https://github.com/Framework-R-D/meld.git
$ spack mpd refresh
$ spack mpd build -j12
$ spack mpd test -j12
```

Working on an existing MPD project from a new shell

```console
$ source <some scratch area/>spack-fnal/share/spack/setup-env.sh
$ spack mpd select meld-devel
$ spack mpd build -j12
$ ...
$ spack mpd test -j12
```
