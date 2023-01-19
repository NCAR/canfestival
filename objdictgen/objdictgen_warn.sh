#!/bin/sh
echo "==========="
echo "ERROR: CanFestival's real \"objdictgen\" was not installed because"
echo "it requires the default Python to be version 2."

# If they gave a source object dictionary file and target, let them
# know the target will not be generated.
if [ "$#" -eq 2 ]; then
	echo
	echo "$2 will NOT be generated from $1"
fi

echo "==========="

exit 1
