#!/bin/bash

PRJDIR=$(cd $(dirname $0)/..;pwd)
LOGDIR=$PRJDIR/tmp/testlogs
OS_DIST=$($PRJDIR/scripts/detectos.sh)
CMD=$PRJDIR/dist/$OS_DIST/xclock_overlay
if [ ! -x "$CMD" ]; then
    echo "ERROR: can not found xclock_overlay:$CMD"
    exit 1
fi
if ! type Xvnc >/dev/null 2>&1; then
    echo "ERROR: can not found Xvnc"
    exit 1
fi
if ! type google-chrome >/dev/null 2>&1; then
    echo "ERROR: can not found google-chrome"
    exit 1
fi

mkdir -p $LOGDIR
rm -rf $LOGDIR/*

function fn_cleanup() {
  if [ -n "$pid_vnc" ]; then
    kill -9 "$pid_vnc"
  fi
  if [ -n "$pid_chrome" ]; then
    kill -9 "$pid_chrome"
  fi
}
trap fn_cleanup EXIT

Xvnc :7 -quiet -ac -fp unix/:-1 -geometry 1366x768 -desktop=TestRun -FrameRate 24 -SecurityTypes=None -AcceptSetDesktopSize=0 -AlwaysShared=1 >$LOGDIR/vnc.log 2>&1 &
pid_vnc=$!
sleep 1
origDISPLAY="$DISPLAY"

export DISPLAY=:7
$CMD &
google-chrome >$LOGDIR/chrome.log 2>&1 &
pid_chrome=$!

export DISPLAY="$origDISPLAY"
vncviewer :7 >$LOGDIR/viewer.log 2>&1
