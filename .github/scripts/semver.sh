#!/bin/bash


function patchpropertyfiles {

  old_tag=$1
  new_tag=$2

  echo "Patching property/json/tag files $old_tag => $new_tag"

  if [ -f "$GITHUB_WORKSPACE/library.json" ]; then
    sed -i -e "s/\"$old_tag\"/\"$new_tag\"/g" $GITHUB_WORKSPACE/library.json
    cat $GITHUB_WORKSPACE/library.json
  fi

  if [ -f "$GITHUB_WORKSPACE/library.properties" ]; then
    sed -i -e "s/version=$old_tag/version=$new_tag/g" $GITHUB_WORKSPACE/library.properties
    cat $GITHUB_WORKSPACE/library.properties
  fi

  if [ -f "$GITHUB_WORKSPACE/src/gitTagVersion.h" ]; then
    # sed -i -e "s/\"$old_tag\"/\"$new_tag\"/g" $GITHUB_WORKSPACE/src/gitTagVersion.h

    oldversionbits=( ${old_tag//./ } )
    oldmajorversion=${oldversionbits[0]//[!0-9]/};
    oldminorversion=${oldversionbits[1]//[!0-9]/};
    oldpatchversion=${oldversionbits[2]//[!0-9]/};

    newversionbits=( ${new_tag//./ } )
    newmajorversion=${newversionbits[0]//[!0-9]/};
    newminorversion=${newversionbits[1]//[!0-9]/};
    newpatchversion=${newversionbits[2]//[!0-9]/};

    sed -i -e "s/LGFX_VERSION_MAJOR $oldmajorversion/LGFX_VERSION_MAJOR $newmajorversion/g" $GITHUB_WORKSPACE/src/gitTagVersion.h
    sed -i -e "s/LGFX_VERSION_MINOR $oldminorversion/LGFX_VERSION_MINOR $newminorversion/g" $GITHUB_WORKSPACE/src/gitTagVersion.h
    sed -i -e "s/LGFX_VERSION_PATCH $oldpatchversion/LGFX_VERSION_PATCH $newpatchversion/g" $GITHUB_WORKSPACE/src/gitTagVersion.h

    cat $GITHUB_WORKSPACE/src/gitTagVersion.h
  fi

}



if [ $GITHUB_EVENT_NAME == "workflow_dispatch" ]; then

  echo "Workflow dispatched event, guessing version from properties file"
  localtag=`cat $GITHUB_WORKSPACE/library.properties | grep version`
  RELEASE_TAG=${localtag//version=/ }
  minor=true

else

  if [ ! $GITHUB_EVENT_NAME == "release" ]; then
      echo "Wrong event '$GITHUB_EVENT_NAME'!"
      exit 1
  fi

  EVENT_JSON=`cat $GITHUB_EVENT_PATH`

  action=`echo $EVENT_JSON | jq -r '.action'`
  if [ ! $action == "published" ]; then
      echo "Wrong action '$action'. Exiting now..."
      exit 0
  fi

  draft=`echo $EVENT_JSON | jq -r '.release.draft'`
  if [ $draft == "true" ]; then
      echo "It's a draft release. Exiting now..."
      exit 0
  fi

  RELEASE_PRE=`echo $EVENT_JSON | jq -r '.release.prerelease'`
  RELEASE_TAG=`echo $EVENT_JSON | jq -r '.release.tag_name'`
  RELEASE_BRANCH=`echo $EVENT_JSON | jq -r '.release.target_commitish'`
  RELEASE_ID=`echo $EVENT_JSON | jq -r '.release.id'`
  RELEASE_BODY=`echo $EVENT_JSON | jq -r '.release.body'`

  echo "Event: $GITHUB_EVENT_NAME, Repo: $GITHUB_REPOSITORY, Path: $GITHUB_WORKSPACE, Ref: $GITHUB_REF"
  echo "Action: $action, Branch: $RELEASE_BRANCH, ID: $RELEASE_ID"
  echo "Tag: $RELEASE_TAG, Draft: $draft, Pre-Release: $RELEASE_PRE"

fi


# Increment a version string using Semantic Versioning (SemVer) terminology.
major=false;
minor=false;
patch=false;

while getopts ":MmpF:" Option
do
  case $Option in
    M ) major=true;;
    m ) minor=true;;
    p ) patch=true;;
    F )
      version=${OPTARG}
      if [ "$version" == "auto" ]; then
        # default to patch
        patch=true
      else
        forcedversion=true
        echo "Forcing version to $version"
      fi
    ;;
    * ) echo "NOTHING TO DO";;
  esac
done

if [ $OPTIND -eq 1 ]; then
  echo "No options were passed, assuming patch level"
  patch=true
fi


if [ -z ${RELEASE_TAG} ]
then
    echo "Couldn't determine version"
    exit 1
fi


if [ "$forcedversion" != "true" ]; then

  # Build array from version string.

  a=( ${RELEASE_TAG//./ } )
  major_version=0
  # If version string is missing or has the wrong number of members, show usage message.

  if [ ${#a[@]} -ne 3 ]; then
    echo "usage: $(basename $0) [-Mmp] major.minor.patch"
    exit 1
  fi

  # Increment version numbers as requested.

  if [ $major == "true" ]; then
    echo "Raising MAJOR"
    # Check for v in version (e.g. v1.0 not just 1.0)
    if [[ ${a[0]} =~ ([vV]?)([0-9]+) ]]; then
      v="${BASH_REMATCH[1]}"
      major_version=${BASH_REMATCH[2]}
      ((major_version++))
      a[0]=${v}${major_version}
    else
      ((a[0]++))
      major_version=a[0]
    fi

    a[1]=0
    a[2]=0
  fi

  if [ $minor == "true" ]; then
    echo "Raising MINOR"
    ((a[1]++))
    a[2]=0
  fi

  if [ $patch == "true"  ]; then
    echo "Raising PATCH"
    ((a[2]++))
  fi

  version=$(echo "${a[0]}.${a[1]}.${a[2]}")

fi

patchpropertyfiles $RELEASE_TAG $version

