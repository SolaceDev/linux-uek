#!/bin/sh
#DEVSERVERS=$(/opt/soldev/devtools/bin/reachable-dev-servers |tr "\n" " ")
#DISTCC_HOSTS=--randomize $(DEVSERVERS// /,cpp,lzo )

COMPILER_PATH=${COMPILER_PATH:-"/opt/soldev/toolchains/centos7-v7/targets/centos7-x86_64/bin"}
PATH=${COMPILER_PATH}:$PATH

VERSION=`make ARCH=x86_64 kernelversion`
VERSION_PATCH=${VERSION%\.*}
LOAD_DIR=/home/public/RND/loads/linux/${VERSION_PATCH}

STDIN_RUN_ON_BUILDHOST="xargs -0 bash -c"
#export CROSS_COMPILE=distcc 

DEBUG_SET=(
    "CONFIG_DEBUG_FS"
    "CONFIG_SATA_FAULT_INJECT"
          )

DEBUG_UNSET=(
            )

function prep_debug_build
{
	OLD_DIR=$1
	NEW_DIR=$2
        ENABLE_SUDO=$3 
        SUDO_CMD=""
        if [ -n "$ENABLE_SUDO" ]; then
            SUDO_CMD="sudo -u kernel"
        fi
	echo "Configuring for Debug build"
        # Enable the CONFIG options
        for each in ${DEBUG_SET[*]}; do
            parts=(${each//=/ })
            option=${parts[0]}
            value=${parts[1]}
            if [ -z "$value" ]; then 
                # Set it to the default value    
                value="y"
            fi
            if grep -q "# $option is not set" $NEW_DIR/.config > /dev/null 2>&1; then
                echo "Setting $option"
	        $SUDO_CMD sed -i "s/# $option is not set/$option=$value/" $NEW_DIR/.config
            elif grep -q "^$option" $NEW_DIR/.config > /dev/null 2>&1; then
                echo "updating $option"
	        $SUDO_CMD sed -i "s/^$option=.*/$option=$value/" $NEW_DIR/.config
            else
                echo "adding $option"
                # Was not found so add it
	        echo "echo \"$option=$value\" >> $NEW_DIR/.config" | $SUDO_CMD bash
            fi
        done
        # Disable the CONFIG options
        for option in ${DEBUG_UNSET[*]}; do
	    $SUDO_CMD sed -i "s/$option=.*/# $option is not set/" $NEW_DIR/.config
        done
}

function debug_build()
{
	OLD_DIR=$1
	NEW_DIR=$2
        ENABLE_SUDO=$3 
        SUDO_CMD=""
        if [ -n "$ENABLE_SUDO" ]; then
            SUDO_CMD="sudo -u kernel"
        fi
        echo "Starting Debug build"
	$SUDO_CMD mkdir "${NEW_DIR}_debug" || exit 1
	echo "yes \"\" | make -C \"$NEW_DIR\" ARCH=x86_64 oldconfig" | $STDIN_RUN_ON_BUILDHOST
	make -C "$NEW_DIR" ARCH=x86_64 -j8 bzImage
	RC=$?
	if [ $RC -eq 0 ]; then
		echo "Copying debug results to ${NEW_DIR}_debug"
		$SUDO_CMD cp ${NEW_DIR}/arch/x86_64/boot/bzImage ${NEW_DIR}_debug
		$SUDO_CMD cp ${NEW_DIR}/vmlinux ${NEW_DIR}_debug
		$SUDO_CMD cp ${NEW_DIR}/.config ${NEW_DIR}_debug
	fi
	return $RC
}

function clean_build
{
	NEW_DIR=$1
	make -C "$NEW_DIR" ARCH=x86_64 clean
}

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
	if [[ -e build_id.txt ]]; then
		BUILD_ID=$(<build_id.txt)
        fi
 	re='^[0-9]+$'
	if ! [[ -n ${BUILD_ID} ]] || [[ ${BUILD_ID} =~ $re ]]; then
		BUILD_ID=0
	fi
	NEW_BUILD_ID=$(( ${BUILD_ID} + 1 ))
	LAST_BUILD_ID=0
	if [[ -e ${LOAD_DIR} ]]; then
		LAST_BUILD_ID=$(findLatestBuild)
		echo "Last build id ${LAST_BUILD_ID}"
	fi
	if [[ ${NEW_BUILD_ID} -le ${LAST_BUILD_ID} ]]; then
		NEW_BUILD_ID=$(( ${LAST_BUILD_ID} + 1 ))
	fi
	NEW_VERSION=$VERSION.solos$NEW_BUILD_ID
	echo NEW_VERSION is $NEW_VERSION
	mv -f .config old.config
	sed -e "s/^CONFIG_LOCALVERSION=.*\$/CONFIG_LOCALVERSION=\".solos$NEW_BUILD_ID\"/" <old.config >.config
}

echo "Publish Location is $LOAD_DIR"
setVersion

if [ -n "$BUILD_NUMBER" ] ; then
	git tag -a v$NEW_VERSION -m "Incrementing build number to solos$NEW_BUILD_ID before the build"
	echo ${NEW_BUILD_ID} > build_id.txt
	git add build_id.txt
	git commit -m "Solace/loadbuild: Update solos build id"
	git push origin
	# Push the new tag
	git push origin v$NEW_VERSION
	mkdir -p $LOAD_DIR
	mkdir "$LOAD_DIR/$NEW_VERSION" || exit 1
	OLD_PWD="$PWD"
	#echo Binding to "$LOAD_DIR/$NEW_VERSION"
	#sudo mount --bind "$OLD_PWD" "$LOAD_DIR/$NEW_VERSION" || exit 1
	echo "Copying kernel to $LOAD_DIR/$NEW_VERSION"
	cp -diR "$OLD_PWD/." "$LOAD_DIR/$NEW_VERSION" || exit 1
	cd "$LOAD_DIR/$NEW_VERSION" || exit 1
	#make HOSTCC="gcc -m32 -static" HOSTCXX="g++ -m32 -static" ARCH=x86_64 $@
	prep_debug_build "$OLD_PWD" "$LOAD_DIR/$NEW_VERSION"
	debug_build "$OLD_PWD" "$LOAD_DIR/$NEW_VERSION"
	RC=$?
	if [ $RC -ne 0 ]; then
		exit $RC
	fi
	

	#clean_build "$LOAD_DIR/$NEW_VERSION"
	echo "Copy over the original non-debug .config"
	cp -f "$OLD_PWD/.config" "$LOAD_DIR/$NEW_VERSION" || exit 1
	make -C "$LOAD_DIR/$NEW_VERSION" ARCH=x86_64 $@
	RC=$?
        echo "Build perf"
        make -C tools/perf 
else
        if [ -n "$DEBUG" ]; then
            prep_debug_build "" "."
        fi
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
