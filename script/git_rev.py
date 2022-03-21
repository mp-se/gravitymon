import subprocess

revision = (
    subprocess.check_output(["git", "rev-parse", "HEAD"])
    .strip()
    .decode("utf-8")
)
revision = revision[-6:]
print("-D CFG_GITREV='\"..%s\"'" % revision)
