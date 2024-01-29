#
# Basic script to compile / link code to test dataset listing services
#

# Use set -e to force failure if something doesn't build
set -e
set -x

export AS=as
export ASFLAGS='--SYSPARM\(AMODE64\),GOFF,LIST,SUPRWARN\(425,434\)'
export CC=xlc
export CFLAGS='-Wc,lp64,SUPP\(CCN3764\)'
export CPPFLAGS='-DAMODE=64'
export LD=xlc
export LDFLAGS='-Wc,lp64'

(cd src && make)
(cd test && make)
