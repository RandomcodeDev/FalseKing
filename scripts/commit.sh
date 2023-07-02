#!/usr/bin/env sh

if [ "$1" = "" ]; then exit; fi

COMMIT="\"$(git log | head -n1 | sed 's|commit ||')\""
if [ -e $1/commit.txt ]; then LAST=$(cat $1/commit.txt); fi
if [ ! "$COMMIT" = "$LAST" ]; then
    echo "Updating commit hash ($COMMIT vs $LAST)"
    printf $COMMIT > $1/commit.txt
fi

