#!/bin/bash

PRJDIR=$(cd $(dirname $0)/..;pwd)
LOGDIR=$PRJDIR/tmp/testlogs
OS_DIST=$($PRJDIR/scripts/detectos.sh)
CMD=$PRJDIR/dist/$OS_DIST/xclock_overlay
BROWSER=google-chrome
BR_OPT=""
if [ ! -x "$CMD" ]; then
    echo "ERROR: can not found xclock_overlay:$CMD"
    exit 1
fi
if ! type Xvnc >/dev/null 2>&1; then
    echo "ERROR: can not found Xvnc"
    exit 1
fi
if ! type $BROWSER >/dev/null 2>&1; then
    echo "ERROR: can not found $BROWSER"
    exit 1
fi

mkdir -p $LOGDIR
rm -rf $LOGDIR/*

function fn_cleanup() {
  if [ -n "$pid_vnc" ]; then
    kill -9 "$pid_vnc"
  fi
  if [ -n "$pid_browser" ]; then
    kill -9 "$pid_browser"
  fi
}
trap fn_cleanup EXIT

Xvnc :7 -quiet -ac -fp unix/:-1 -geometry 1366x768 -desktop=TestRun -FrameRate 24 -SecurityTypes=None -AcceptSetDesktopSize=0 -AlwaysShared=1 >$LOGDIR/vnc.log 2>&1 &
pid_vnc=$!
sleep 1
origDISPLAY="$DISPLAY"

export DISPLAY=:7
#if type mutter >/dev/null 2>&1; then
#    mutter >$LOGDIR/mutter.log 2>&1 &
#fi
$CMD >$LOGDIR/xclock_overlay.log 2>&1 &
export MOZ_DISABLE_GPU_ACCEL=1
export MOZ_WEBRENDER=0
$BROWSER $BR_OPT >$LOGDIR/browser.log 2>&1 &
pid_browser=$!

export DISPLAY="$origDISPLAY"
vncviewer :7 >$LOGDIR/viewer.log 2>&1
