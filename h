#!/usr/bin/env bash
set +x
set -euo pipefail

H_DEBUG="${H_DEBUG:-false}"
HX_PATH="$HOME/.hx"
mkdir -p "$HX_PATH"

case "$OSTYPE" in
  *arwin*)
    OS="darwin"
    ;;
  *in32* | *indows*)
    OS="windows"
    ;;
  *)
    OS="linux"
esac

CPU_ARCHITECTURE=$(uname -m)
case $CPU_ARCHITECTURE in
  armv5*) CPU_ARCHITECTURE="armv5";;
  armv6*) CPU_ARCHITECTURE="armv6";;
  armv7*) CPU_ARCHITECTURE="arm";;
  aarch64) CPU_ARCHITECTURE="arm64";;
  x86) CPU_ARCHITECTURE="386";;
  x86_64) CPU_ARCHITECTURE="amd64";;
  i686) CPU_ARCHITECTURE="386";;
  i386) CPU_ARCHITECTURE="386";;
esac

function log {
  if [ ${H_DEBUG} == true ]; then
    echo "h: $*" >&2
  fi
}

function get-helm-version {
  local version=$1
  local helm_path=$HX_PATH/helm-$version
  local helm_tar="$helm_path.tar.gz"

  if [ ! -f ${helm_path} ]; then
    log "installing ${helm_path} ..."
    curl -L -o "$helm_tar" "https://get.helm.sh/helm-$version-$OS-$CPU_ARCHITECTURE.tar.gz"
    tar --extract --file="$helm_tar" --strip=1 --directory="$HX_PATH" $OS-$CPU_ARCHITECTURE/helm
    mv "$HX_PATH/helm" "$helm_path"
    rm "$helm_tar"
  else
    log "${helm_path} already installed ..."
  fi
}

DEFAULT_VERSION="${DEFAULT_VERSION:-"v2.13.1"}"
LOCAL_HELM=$HX_PATH/helm-$DEFAULT_VERSION
HELM_PATH=$(command -v helm)

if [ -n "$HELM_PATH" ] && $HELM_PATH version | grep -q "Server:"; then
  LOCAL_HELM=$HELM_PATH
else
  log "${HELM_PATH} is available but is helm3. Avoiding..."
fi

if [ ! -f "$LOCAL_HELM" ]; then
  get-helm-version $DEFAULT_VERSION
fi

log "using ${LOCAL_HELM} to determine Tiller version ..."

TARGET_VERSION=$($LOCAL_HELM version --template '{{ .Server.SemVer }}' || echo $KNOWN_VERSION)
TARGET="$HX_PATH/helm-$TARGET_VERSION"

if [ ! -f "$TARGET" ]; then
  get-helm-version "$TARGET_VERSION"
fi

log "using ${TARGET} ..."
$TARGET "$@"
