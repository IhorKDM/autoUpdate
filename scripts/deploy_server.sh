#!/usr/bin/env bash
# Installed as a forced SSH command outside the Git checkout.
# It accepts no remote commands or arguments and only deploys this repository's main branch.
set -euo pipefail
if [[ ! "${SSH_ORIGINAL_COMMAND:-}" =~ ^deploy[[:space:]]([0-9a-f]{40})$ ]]; then
  echo 'Only deploy followed by a full commit SHA is allowed.' >&2
  exit 1
fi
revision=${BASH_REMATCH[1]}
mkdir -p "$HOME/text-echo"
exec 9>"$HOME/text-echo/deploy.lock"
flock -w 180 9
checkout="$HOME/text-echo/source"
repository=https://github.com/IhorKDM/autoUpdate.git
if [[ ! -d "$checkout/.git" ]]; then
  git clone --depth=100 --branch main "$repository" "$checkout"
fi
cd "$checkout"
if [[ "$(git remote get-url origin)" != "$repository" ]]; then
  echo 'Unexpected deployment repository; refusing to continue.' >&2
  exit 1
fi
git fetch --depth=100 origin '+refs/heads/main:refs/remotes/origin/main' "$revision"
if ! git merge-base --is-ancestor "$revision" origin/main; then
  echo 'Deployment commit must belong to main.' >&2
  exit 1
fi
git reset --hard "$revision"
export TEXT_ECHO_IMAGE="text-echo-web:$revision"
export TEXT_ECHO_PORT=80
compose=(docker compose -f compose.production.yaml)
previous_container=$("${compose[@]}" ps -q web)
previous_image=''
if [[ -n "$previous_container" ]]; then
  previous_image=$(docker inspect --format '{{.Image}}' "$previous_container")
fi
"${compose[@]}" build --pull
if ! "${compose[@]}" up -d --no-build --wait --wait-timeout 90; then
  "${compose[@]}" logs --tail 30 web
  if [[ -n "$previous_image" ]]; then
    docker tag "$previous_image" text-echo-web:rollback
    export TEXT_ECHO_IMAGE=text-echo-web:rollback
    "${compose[@]}" up -d --no-build --wait --wait-timeout 90
    echo 'Restored the previous image after deployment failure.' >&2
  fi
  exit 1
fi
curl --fail --silent --show-error "http://127.0.0.1:$TEXT_ECHO_PORT/health"
printf '\nDeployed commit %s\n' "$revision"
