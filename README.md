hipo-tools
==========

[![Build Status](https://travis-ci.com/JeffersonLab/hipo_tools.svg?branch=master)](https://travis-ci.com/JeffersonLab/hipo_tools)

"Official" C++ based analysis tools for hipo files.

[Hipo-file
Documentation](https://userweb.jlab.org/~gavalian/docs/sphinx/hipo/html/index.html)
(external project)

This code was adopted from source code on the jlab CUE
`/group/clas12/packages/hipo-io`.

Installing
----------
First choose where to install hipo_tools.

For bash/zsh:
```
export HIPO_TOOLS=/path/to/hipo_tools
```
or for csh/tcsh:
```
setenv HIPO_TOOLS /path/to/hipo_tools
```

**Note**: These instructions are for Linux. For MacOS, refer to the next subsection.
```
git clone --recurse-submodules https://github.com/JeffersonLab/hipo_tools.git
cd hipo_tools && mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HIPO_TOOLS
make && make install
```

### Installing on MacOS

For some reason XCode does not currently ship with the necessary C++17
filesystem libraries (even though they have been available in LLVM for several
versions). As temporary workaround, we suggest to install llvm through Homebrew:
```
brew install llvm
```
Homebrew will skip the linking step when you install llvm (as to not screw up
your system), so you will have to tell cmake to use the newer version of
llvm/clang. The installation instructions for MacOS are:
```
git clone https://github.com/JeffersonLab/hipo_tools.git
cd hipo_tools && mkdir build
cd build
CXX=/usr/local/opt/llvm/bin/clang++ cmake ../. -DCMAKE_INSTALL_PREFIX=$HIPO_TOOLS
make && make install
```

### Setting up the environment

Once installed add this to the end of your .bashrc/.zshrc to get hipo_tools to work:

```
#Hipo_tools
export HIPO_TOOLS=/path/to/hipo_tools
export PATH=$PATH:$HIPO_TOOLS/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$HIPO_TOOLS/share/pkgconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HIPO_TOOLS/lib
export PYTHONPATH=$PYTHONPATH:$HIPO_TOOLS/lib
```
Or for csh/tcsh add:
```
#Hipo_tools
setenv HIPO_TOOLS /path/to/hipo_tools
setenv PATH $PATH:$HIPO_TOOLS/bin
setenv PKG_CONFIG_PATH $PKG_CONFIG_PATH:$HIPO_TOOLS/share/pkgconfig
setenv LD_LIBRARY_PATH $LD_LIBRARY_PATH:$HIPO_TOOLS/lib
setenv PYTHONPATH $PYTHONPATH:$HIPO_TOOLS/lib
```

### Some tips

* Use the latest version of root with the latest compiler (e.g. gcc8)
* Use **C++17** because you have no reason not to!
* Compile root with "-Droot7:bool=ON -Dcxx17:BOOL=ON"
* Don't write loops! Use [RDataFrame](https://root.cern.ch/doc/master/group__tutorial__dataframe.html)


Converting hipo files
---------------------

### toohip4root

```
$ toohip4root -h
SYNOPSIS
        toohip4root (<input file> | [-i <input file>]) [-o <output file>] [-f] [-v] [-N <N events>] [-h]

OPTIONS
        -f, --force force the output to overwrite existing files

        -v, --verbose
                    turn on verbose output

        -h, --help  print help

```

### dst2root

A clas12 specific converter to extract the DST banks from a hipo file and
put them into a root file. Useful for analysis especially if downloading the
files to an offsite location or a personal computer.

```
$ dst2root -h
SYNOPSIS
        dst2root [-h] [-mc] [-b] [-r] [-e] [-c] <inputFile.hipo> [<outputFile.root>]

OPTIONS
        -h, --help  print help
        -mc, --MC   Convert dst and mc banks
        -b, --batch Don't show progress and statistics
        -r, --rec   Only save events where number of partilces in the event > 0
        -e, --elec  Only save events with good electron as first particle
        -c, --cov   Save Covariant Matrix for kinematic fitting
```

The contents of the file have been re-ordered from the original file and places
into banks specific to each detector system. This is done to reduce the amount
of looping increase the speed for the end user. There are also a few banks
which are added for connivence to the user such as the total number of
photoelectrons for a particle, the total energy deposited for a particle, as
well as the magnitude of the momentum and the momentum squared.

For particles with missing banks, due to the particle not being detected by that detector/layer/etc.
then the information in the bank is filled with a flag value. For integers the flag value is set to
be -1 and for floats the flag value is set to be std::nan.

Here are a few examples of equivalent java and c++ code to complete the same tasks:

##### Sampling Fraction
```C++
// C++ example filling sampling fraction histogram
sf_hist->Fill(p->at(ipart), ec_tot_energy->at(ipart) / p->at(ipart));
```

```Java
// Java example to get get EC energy:
double energy=0;
for (int icalo=0; icalo<calos.getMaxSize(); icalo++) {
    if (calos.getNode("pindex").getShort(icalo) == ipart) {
          energy += calos.getNode("energy").getFloat(icalo);
    }
}
// and then fill the sampling fraction histogram
if (energy>0) {
    final float px   = parts.getNode("px").getFloat(ipart);
    final float py   = parts.getNode("py").getFloat(ipart);
    final float pz   = parts.getNode("pz").getFloat(ipart);
    final float p = Math.sqrt(px*px+py*py+pz*pz);
    hist.fill(p, energy/p);
}
```

##### FTOF time and path
```C++
// get FTOF path length and time
float time = sc_ftof_1b_time->at(ipart);
float path = sc_ftof_1b_path->at(ipart);
```

```Java
// get FTOF path length and time
for (int iscin=0; iscin<scins.getNode("pindex").getShort().size(); iscin++) {
    if (scins.getNode("pindex").getShort(iscin) == ipart) {
        if (scins.getNode("detector").getByte(iscin) == 12) {
            final float path = scins.getNode("path").getFloat(iscin);
            final float time = scins.getNode("time").getFloat(iscin);
            break;
        }
    }
}
```
For more detailed examples of using dst2root converted files checkout the more detailed [examples](examples/dst2root).

[java examples](https://userweb.jlab.org/~gavalian/docs/sphinx/hipo/html/chapters/java_groovy_analysis.html#ec-sampling-fraction)


Reading hipo files in python
---------------------
### hipopy

Easily read files in python using hipopy, which is build automatically if Cython is found.
An example of reading a hipo4 file in python can be found [here](examples/hipo4/clas12_event.py).

Availible hipopy
```python
beamCharge
beta
cal_chi2
cal_detector
cal_du
cal_dv
cal_dw
cal_energy
cal_hx
cal_hy
cal_hz
cal_index
cal_layer
cal_len
cal_lu
cal_lv
cal_lw
cal_m2u
cal_m2v
cal_m2w
cal_m3u
cal_m3v
cal_m3w
cal_path
cal_pindex
cal_sector
cal_status
cal_time
cal_x
cal_y
cal_z
category
charge
chern_chi2
chern_detector
chern_dphi
chern_dtheta
chern_index
chern_len
chern_nphe
chern_path
chern_phi
chern_pindex
chern_sector
chern_status
chern_theta
chern_time
chern_x
chern_y
chern_z
chi2pid
event_len
ft_chi2
ft_detector
ft_dx
ft_dy
ft_energy
ft_len
ft_path
ft_pindex
ft_radius
ft_size
ft_status
ft_time
ft_x
ft_y
ft_z
helicity
helicityRaw
liveTime
mc_atarget
mc_btype
mc_ebeam
mc_lund_daughter
mc_lund_energy
mc_lund_index
mc_lund_lifetime
mc_lund_mass
mc_lund_parent
mc_lund_pid
mc_lund_px
mc_lund_py
mc_lund_pz
mc_lund_type
mc_lund_vx
mc_lund_vy
mc_lund_vz
mc_npart
mc_pbeam
mc_pid
mc_processid
mc_ptarget
mc_px
mc_py
mc_pz
mc_targetid
mc_vt
mc_vx
mc_vy
mc_vz
mc_weight
mc_ztarget
pid
procTime
px
py
pz
sc_chi2
sc_component
sc_detector
sc_energy
sc_hx
sc_hy
sc_hz
sc_index
sc_layer
sc_len
sc_path
sc_pindex
sc_sector
sc_status
sc_time
sc_x
sc_y
sc_z
startTime
status
topology
trk_NDF
trk_NDF_nomm
trk_chi2
trk_chi2_nomm
trk_detector
trk_index
trk_len
trk_pindex
trk_px_nomm
trk_py_nomm
trk_pz_nomm
trk_q
trk_sector
trk_status
trk_vx_nomm
trk_vy_nomm
trk_vz_nomm
vd_cx1
vd_cx2
vd_cy1
vd_cy2
vd_cz1
vd_cz2
vd_index1
vd_index2
vd_len
vd_r
vd_x
vd_x1
vd_x2
vd_y
vd_y1
vd_y2
vd_z
vd_z1
vd_z2
vx
vy
vz
```

Todo
----

- [ ] Isolate json parser from converter
- [ ] Create clas12 specific helpers (maybe different repo for clas12 specific json?)
- [ ] Setup CI with test data.
- [x] Debug build on mac (#7)
- [ ] Test library in external cmake and pkgconfig projects


### Install structure:

```
.
├── bin
│   ├── dst2root
│   ├── dst2root_3
│   ├── dst2root_examples
│   │   ├── MonteCarlo
│   │   ├── WvsQ2
│   │   ├── deltat
│   │   ├── pvsb
│   │   └── samplingFraction
│   ├── hipo3_examples
│   │   ├── benchmark
│   │   ├── hipo
│   │   ├── pulse_compression
│   │   ├── read_event_file
│   │   ├── read_file
│   │   ├── read_file_advanced
│   │   └── write_file
│   ├── hipo2root
│   └── hipo2root_mc
├── include
│   ├── hipo3
│   │   ├── data.h
│   │   ├── dictionary.h
│   │   ├── event.h
│   │   ├── hipoexceptions.h
│   │   ├── node.h
│   │   ├── reader.h
│   │   ├── record.h
│   │   ├── text.h
│   │   ├── utils.h
│   │   └── writer.h
│   └── hipo4
│       ├── bank.h
│       ├── dictionary.h
│       ├── event.h
│       ├── hipoexceptions.h
│       ├── reader.h
│       ├── record.h
│       ├── recordbuilder.h
│       └── utils.h
├── lib
│   ├── clas12.so
│   ├── hipo
│   │   ├── HipoToolsConfig.cmake
│   │   └── HipoToolsConfigVersion.cmake
│   ├── hipopy3.so
│   ├── hipopy4.so
│   ├── libhipocpp3.dylib
│   ├── libhipocpp3_static.a
│   ├── libhipocpp4.dylib
│   └── libhipocpp4_static.a
└── share
    └── pkgconfig
        ├── hipocpp3.pc
        └── hipocpp4.pc

```
