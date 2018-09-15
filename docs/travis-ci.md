# Travis-CI and Optiboot

## What is Travis?

https://travis-ci.com is site which allows to make checking builds after
every push of code to git repository. It could also check pull request.

It nicely integrates with Github, so to enable it's enough to just
install Travis' Github App from: https://github.com/marketplace/travis-ci

Travis have also free plan for Open Source projects, so there is no
cost involved.

When installed, after every push a build is started on Travis-CI
servers. After build finish, status of commit is updated, so everyone
can see if build passed or failed. Clicking on details gives more
information about which job(s) within all build failed and even direct
output from virtual machine, so it's easy to check why something went
wrong.

Of course, Travis-CI also needs configuration to define what should
be run to test code, so every project must have *.travis.yml* file
and some others if external scripts are needed.

## Files

### .travis.yml

Standard Travis-CI file with definitions of jobs.

There are 2 stages of tests:
- *check sizes*: there is only one build in this stage which compiles ALL
targets specified in *.travis.yml* (variable *OPTIBOOT_TARGET* in matrix)
agains ALL compilers marked with 'yes' from *arduino-gcc-versions.md* file
plus latest Microchip's compiler for avr8. There are 2 tables produced
at the end of build: one is output of current compilation and resulting
sizes for every target or 'x' if build failed. Second one is based on
online database of builds and gives comparison between current build
and last commited into repository. Output is markdown compatible, so
it could be just copied and pasted into Github comment to produce
pretty table.
- test: stage with separate build for every *OPTIBOOT_TARGET* against only
one compiler specified in variable *TOOLS_VERSION*. In 'allow_failures'
section there are targets known to fail and ignored in overall status
of test.

### scripts/travis-build.sh

Script used to build single target during 'test' stage.

## scripts/travis-check-sizes.sh

Script used to build all targets using all compilers during 'check sizes'
stage.

## scripts/travis-download.inc.sh

Include script used to download and unpack tools. Used by *travis-build.sh*
and *travis-check-sizes.sh* scripts.

## scripts/travis-env.inc.sh

Helper script for local testing (on Linux) builds. It sets all necessary
environment variables to make *travis-build.sh* or *travis-check-sizes.sh*
on local machine. Need to be sourced from repository parent directory:
```bash
. scripts/travis-env.inc.sh
```

## scripts/travis-fill-cache.sh

Helper script used by *travis-check-sizes.sh* to download and extract all
needed compilers. It uses *docs/arduino-gcc-versions.md* file to find
which Arduino version should be downloaded.
