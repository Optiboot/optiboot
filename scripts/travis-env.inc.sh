# source it from parent directory to mimic Travis-CI
# environmental variables needed in scripts

if [[ -d .git ]]; then
    export TRAVIS_BUILD_DIR=`pwd`
else
    echo "ERROR: include it from repository parent directory!!!"
    return
fi

export TRAVIS_COMMIT="THIS_IS_FAKE_COMMIT_HASH_FOR_TESTS_ONLY1"
export TRAVIS_COMMIT_MESSAGE="Example commit message"
export TRAVIS_EVENT_TYPE="push"
export TRAVIS_OS_NAME="linux"
export TRAVIS_PULL_REQUEST="false"
export TRAVIS_PULL_REQUEST_BRANCH=""
export TRAVIS_PULL_REQUEST_SHA=""
export TRAVIS_PULL_REQUEST_SLUG=""

export TRAVIS_REPO_SLUG=$(git config --get travis.slug)
if [[ -z "$TRAVIS_REPO_SLUG" ]]; then
    export TRAVIS_REPO_SLUG=$(git config --get remote.origin.url|sed 's/.*github\.com.//'|sed 's/\.git$//')
fi

export TRAVIS_SUDO="false"
export TRAVIS_BRANCH=$(git rev-parse --abbrev-ref HEAD)
