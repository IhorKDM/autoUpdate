# DigitalOcean deployment

Site: http://167.99.87.65/ (the actual FastAPI application).
Static frontend mirror: https://ihorkdm.github.io/autoUpdate/ .

## What is installed

- A standalone Docker Compose project named `text-echo`.
- Source: `/home/ihor/text-echo/source`.
- Restricted deployment command: `/home/ihor/.local/lib/text-echo/deploy`.
- A dedicated GitHub Actions SSH public key appended to `ihor`'s authorized keys.
- The app container serves port 80 and runs as an unprivileged container user,
  with a read-only filesystem, dropped Linux capabilities, and a small temporary filesystem.

The deployment uses the existing Docker installation. It does not replace SSH
keys, change sudo permissions, or modify other Docker projects.

## How automatic CD works

1. A push to main starts CI.
2. After successful CI, `deploy-server.yml` connects over SSH using encrypted
   `DEPLOY_SSH_KEY` and `DEPLOY_KNOWN_HOSTS` repository secrets.
3. Its dedicated SSH key has `restrict` and a forced command: arbitrary remote
   commands, terminal sessions, and port forwarding are unavailable to this key.
4. The command validates the requested SHA, verifies it belongs to main, and
   deploys that exact commit. Deployment is locked so concurrent runs cannot clash.
5. Docker builds an image tagged with that SHA and starts it with health checks.
6. If startup fails, the previous image is restored when one is available.
7. Actions checks the public `/health` endpoint.

The SSH key inherits this account's ability to run the Docker deployment command;
keep repository write access and Actions secret administration limited to trusted users.
Only pushes from this repository trigger the privileged workflow, not pull requests.

GitHub repository variables: `DEPLOY_ENABLED=true`, `DEPLOY_HOST=167.99.87.65`,
`DEPLOY_USER=ihor`. Set `DEPLOY_ENABLED=false` to stop automatic server deployment.
The key stays valid until its specific line is removed from the server's
`~/.ssh/authorized_keys`; deleting the Actions secret alone prevents future
workflow use but does not revoke a leaked key.

## Inspect the running service

```sh
ssh ihor@167.99.87.65
cd ~/text-echo/source
docker compose -f compose.production.yaml ps
docker compose -f compose.production.yaml logs --tail 100 web
curl --fail http://127.0.0.1/health
```

## Manual deployment

From this project on your Mac, deploy a tested main commit with your existing
personal SSH access (the dedicated Actions key is restricted):

```sh
REVISION=$(git rev-parse HEAD)
ssh ihor@167.99.87.65 "SSH_ORIGINAL_COMMAND='deploy $REVISION' ~/.local/lib/text-echo/deploy"
```

Use the GitHub Actions **Deploy FastAPI to DigitalOcean → Run workflow** button
to retry the current main commit through the same automated path.

## Rollback

In Actions, first set `DEPLOY_ENABLED=false` to keep later CI runs from replacing
your manual rollback. Find a known good commit on main, then use the manual
deployment command above with that SHA. Existing images are retained; disk usage
can be inspected with `docker system df`. Avoid global prune commands on a shared
server. To stop this service, use `docker compose -f compose.production.yaml down`
in its source directory; that targets only this Compose project.

## HTTPS

The supplied address is an IP, so this initial deployment uses HTTP and accepts
no passwords or private user data. Point a domain at the server to add a reverse
proxy with managed TLS. The static GitHub Pages mirror already uses HTTPS.
