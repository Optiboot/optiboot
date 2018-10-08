#!/usr/bin/env bash

LOCAL_TOOLS_DIR=$HOME/avr-tools

if [ -z "$TRAVIS_BUILD_DIR" ]; then
    echo "This script should be run by Travis-CI environment"
    echo "If you want to simulate Travis build, please set TRAVIS_BUILD_DIR"
    echo "environment variable to directory where your code lives"
    exit 1
fi


# download all compilers
$TRAVIS_BUILD_DIR/scripts/travis-fill-cache.sh

# prepare output dir
OUTPUT_DIR="$TRAVIS_BUILD_DIR/sizes-out"
mkdir -p "$OUTPUT_DIR"
OUTPUT_TABLE="$OUTPUT_DIR/sizes.txt"
OUTPUT_JSON="$OUTPUT_DIR/sizes.json"

# compiler list
COMPILERS=$(cat $TRAVIS_BUILD_DIR/docs/arduino-gcc-versions.md |grep -i "| yes |"|cut -f 2 -d '|')
COMPILERS="$COMPILERS microchip"

# table header
echo -n "| target \ compiler |" >"$OUTPUT_TABLE"
for compiler in $COMPILERS; do
  echo -n " $compiler |" >>"$OUTPUT_TABLE"
done
echo >>"$OUTPUT_TABLE"
# table header separator
echo -n "|-|" >>"$OUTPUT_TABLE"
for compiler in $COMPILERS; do
    echo -n "-|">>"$OUTPUT_TABLE"
done
echo >>"$OUTPUT_TABLE"

# get repo and commit info for json output
if [[ "$TRAVIS_PULL_REQUEST" = "false" ]]; then
    REPO="$TRAVIS_REPO_SLUG"
    BRANCH="$TRAVIS_BRANCH"
else
    REPO="$TRAVIS_PULL_REQUEST_SLUG"
    BRANCH="$TRAVIS_PULL_REQUEST_BRANCH"
fi

# start json
echo "{\"slug\":\"$REPO\",\"branch\":\"$BRANCH\",\"commit\":\"$TRAVIS_COMMIT\",\"emoji\":\"true\",\"builds\":[" >"$OUTPUT_JSON"

# build everything
cat $TRAVIS_BUILD_DIR/.travis.yml|grep "    - OPTIBOOT_TARGET="|cut -f 2- -d '=' \
  |tr -d '"'|sort|while read target; do
    echo -n "| $target |" >>"$OUTPUT_TABLE"
    echo "{\"t\":\"$target\",\"v\":[">>"$OUTPUT_JSON"
    for compiler in $COMPILERS; do
	echo "Checking size for $target @ $compiler"
	size=$($TRAVIS_BUILD_DIR/scripts/travis-build.sh $compiler $target 2>/dev/null|grep -A 2 avr-size|tail -n1|awk '{ print $1;}')
	if [[ -z "$size" ]]; then
	    size="x"
	fi
	echo -n " $size |" >>"$OUTPUT_TABLE"
	echo "{\"c\":\"$compiler\",\"s\":\"$size\"}," >>"$OUTPUT_JSON"
    done
    echo >>"$OUTPUT_TABLE"
    sed -i '$ s/.$//' "$OUTPUT_JSON"
    echo "]}," >>"$OUTPUT_JSON"
done
sed -i '$ s/.$//' "$OUTPUT_JSON"
echo "]}">>"$OUTPUT_JSON"

echo "========= OUTPUT SIZES START ============="
cat "$OUTPUT_TABLE"
echo "========== OUTPUT SIZES END =============="

echo "Checking results against last commit"
echo "========= OUTPUT SIZES COMPARE START ============="
curl -H "Content-Type: application/json" --data @$OUTPUT_JSON https://api.travisjoin.w7i.pl/tj/compare/$REPO/$BRANCH/last
echo "========== OUTPUT SIZES COMPARE END =============="

echo "Uploading results to TravisJoin"
curl -H "Content-Type: application/json" --data @$OUTPUT_JSON https://api.travisjoin.w7i.pl/tj/add/$REPO/$BRANCH/$TRAVIS_COMMIT

exit 0
