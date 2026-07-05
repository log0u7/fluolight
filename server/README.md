# Pipeline Status Server

`pipeline_server.py` is a production-ready server that polls GitHub Actions
and GitLab CI pipelines and exposes their state to the board via the same
`GET /lights/<mac>` protocol. Each board (identified by its MAC address) is
mapped to one CI/CD project. `test_server.py` remains available for
offline testing.

## Requirements

- Python 3.6+ (stdlib only, no extra packages)
- A GitHub personal access token (for GitHub targets)
- A GitLab personal access token (for GitLab targets)

## Configuration

Copy `targets.example.json` to `targets.json` (gitignored) and fill in your
targets:

```json
{
  "port": 8080,
  "poll_interval": 30,
  "targets": {
    "5410ecee3ea0": {
      "provider": "github",
      "repo": "owner/repository",
      "branch": "main"
    },
    "aabbccddeeff": {
      "provider": "gitlab",
      "host": "gitlab.com",
      "project": "group/repository",
      "branch": "main"
    }
  }
}
```

The MAC address key must match the board's MAC in lowercase without
separators (printed at boot: `INF:ETH:INIT: 5410ecee3ea0`).

For self-hosted GitLab instances, set `host` to your instance hostname.

## Authentication

Set tokens as environment variables - never put them in `targets.json`:

```bash
export GITHUB_TOKEN=ghp_...
export GITLAB_TOKEN=glpat-...
```

For multiple self-hosted GitLab instances, use per-host variables
(dots replaced by underscores):

```bash
export GITLAB_TOKEN_gitlab_example_com=glpat-...
```

## Running

```bash
python3 pipeline_server.py                    # uses targets.json on port 8080
python3 pipeline_server.py --config /path/to/cfg.json
python3 pipeline_server.py --help             # list all options
```

Open the firewall if needed:

```bash
sudo ufw allow from 192.168.1.0/24 to any port 8080 proto tcp
```

## Status code mapping

| Code | Color | GitHub Actions | GitLab CI |
|------|-------|---------------|-----------|
| `0` | Blinking red | API error / network failure / MAC not in config | same |
| `1` | Green | completed / success | success |
| `4` | Red | completed / failure or timed_out | failed |
| `6` | Blinking orange | in_progress / queued / pending | running / pending / created |
| `8` | White | canceled / skipped / neutral | canceled / skipped / manual |
| `9` | Off | no run found | no pipeline found |

On transient API errors the last known state is preserved so the board
does not flash red on a momentary network blip.
