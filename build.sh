#!/bin/sh

COMPILER_PATH=${COMPILER_PATH:-"/opt/gcc-7.3.0-x86_64/bin"}
PATH=${COMPILER_PATH}:$PATH

VERSION=`make ARCH=x86_64 kernelversion`
VERSION_PATCH=${VERSION%\.*}
LOAD_DIR=/home/public/RND/loads/linux/${VERSION_PATCH}

function findLatestBuild
{
    LAST_ID=0
    for i in {1..200}; do
        CHECK_VERSION=${VERSION}.solos$i
        if [[ -e ${LOAD_DIR}/${CHECK_VERSION} ]] ; then
            LAST_ID=$i
        fi
    done
    echo ${LAST_ID}
}

function setVersion
{
    if [[ -z $VERSION ]]; then
        echo "Couldn't determine current version"
        exit
    fi
    LAST_BUILD_ID=0
    if [[ -e ${LOAD_DIR} ]]; then
        LAST_BUILD_ID=$(findLatestBuild)
        echo "Last build id ${LAST_BUILD_ID}"
    fi
    NEW_BUILD_ID=$(( ${LAST_BUILD_ID} + 1 ))
    NEW_VERSION=$VERSION.solos$NEW_BUILD_ID
    echo NEW_VERSION is $NEW_VERSION
    mv -f .config old.config
    sed -e "s/^CONFIG_LOCALVERSION=.*\$/CONFIG_LOCALVERSION=\".solos$NEW_BUILD_ID\"/" <old.config >.config
}

echo "Publish Location is $LOAD_DIR"
setVersion

if [ -n "$BUILD_NUMBER" ] ; then
    git tag -a v$NEW_VERSION -m "Incrementing build number to solos$NEW_BUILD_ID before the build"
    git push origin v$NEW_VERSION
    mkdir -p $LOAD_DIR
    mkdir "$LOAD_DIR/$NEW_VERSION" || exit 1
    OLD_PWD="$PWD"
    echo "Copying kernel to $LOAD_DIR/$NEW_VERSION"
    cp -diR "$OLD_PWD/." "$LOAD_DIR/$NEW_VERSION" || exit 1
    cd "$LOAD_DIR/$NEW_VERSION" || exit 1
    make -C "$LOAD_DIR/$NEW_VERSION" ARCH=x86_64 $@
    RC=$?
else
    make ARCH=x86_64 $@
    RC=$?
fi

if [ -n "$BUILD_NUMBER" ] ; then
    git archive v$NEW_VERSION| gzip > $LOAD_DIR/$NEW_VERSION/kernel-$NEW_VERSION-src.tar.gz
    if [ $? -ne 0 ] ; then
        echo "Creation of $LOAD_DIR/$NEW_VERSION/kernel-$NEW_VERSION-src.tar.gz failed!"
        exit 1
    fi
    touch $LOAD_DIR/$NEW_VERSION/.keepme
    echo "Changing directory back to $OLD_PWD"
    cd "$OLD_PWD" || exit 1
    echo Done!
fi

exit $RC
