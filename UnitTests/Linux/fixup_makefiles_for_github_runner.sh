#!/bin/bash

sed_replace_home ()
{
	echo "Fixing makefile paths in: $1"
	sed -i 's/home\/tristan\/github\/tacent/home\/runner\/work\/tacent\/tacent/' $1
}
export -f sed_replace_home

sed_replace_usrbin ()
{
	echo "Fixing makefile paths in: $1"
	sed -i 's/\/usr\/bin\/llvm-ar/ar/' $1
}
export -f sed_replace_usrbin

find -name '*.mk' -exec /bin/bash -c 'sed_replace_home "$0"' {} \;
find -name 'Makefile' -exec /bin/bash -c 'sed_replace_home "$0"' {} \;

find -name '*.mk' -exec /bin/bash -c 'sed_replace_usrbin "$0"' {} \;
find -name 'Makefile' -exec /bin/bash -c 'sed_replace_usrbin "$0"' {} \;

