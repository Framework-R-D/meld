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

To build Meld using Fermilab's Spack bootstrap script and MPD extension, follow these steps.

1. Set up your Spack installation (once per machine)

On Linux, you can install and configure the right version of Spack and associated tools using our bootstrap script.

```console
# export TOP_DIR=/path/to/workspace
cd ${TOP_DIR}
wget https://github.com/FNALssi/fermi-spack-tools/raw/refs/heads/fnal-develop/bin/bootstrap
bash bootstrap ${PWD}/spack-fnal
source ${PWD}/spack-fnal/share/spack/setup-env.sh
```

The `bootstrap` script does not work on macOS.
To get going with macOS, do the following instead.
Note that if you have a previous installation of spack, you may have a directory `$HOME/.spack`.
These instructions have only been tested with a fresh installation, meaning there is no such directory.

```console
# export TOP_DIR=/path/to/workspace
cd ${TOP_DIR}
git clone -b fnal_develop git@github.com:FNALssi/spack.git spack-fnal  # named to match the instructions above
git clone -b develop git@github.com:FNALssi/fnal_art.git
git clone git@github.com:FNALssi/spack-mpd.git
source ${PWD}/spack-fnal/share/spack/setup-env.sh
spack repo add $PWD/fnal_art   # Fermilab's Spack recipes
```
Then you need to edit the Spack configuration using `spack config --scope site edit config`.
Add the following to your configuration:

```yaml
config:
  extensions:
    - <path to your spack-mpd clone>
```

2. Set up MPD for the Spack installation (once per Spack installation).
   We expect you're doing this in the same shell session used in step (1).
   If that is not the case, repeat the `source` of the `setup-env.sh` file.

```console
spack mpd init
```

3. Establish compilers

```console
spack compiler find
spack compilers
```

This will find, and then report, what compilers you have available.

3. Create a new MPD project

```console
# export TOP_DIR=/path/to/workspace
source ${TOP_DIR}/spack-fnal/share/spack/setup-env.sh
spack mpd new-project --name meld-devel -T ${TOP_DIR}/meld-devel cxxstd=20 %gcc@11
spack mpd clone https://github.com/Framework-R-D/meld.git
spack mpd refresh
spack mpd build -j12
spack mpd test -j12
```

4. Work on an existing MPD project from a new shell

```console
export TOP_DIR=/path/to/workspace
source ${TOP_DIR}/spack-fnal/share/spack/setup-env.sh
spack mpd select meld-devel
spack mpd build -j12
spack mpd test -j12
```

## Additional notes

The `spack mpd` commands will activate and deactive an environment as needed.
If you want to activate the environment manually, use the `spack env activate` command, giving it the path to the environment.
The `spack mpd build` command will notify you if you are using an already-activated environment or one it activates for you.
