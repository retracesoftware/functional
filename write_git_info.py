# scripts/write_git_info.py
#python - <<'PY'
import subprocess, pathlib
root = pathlib.Path(__file__).resolve().parents[1]      # repo root
pkg  = root / "yourpkg"
pkg.mkdir(exist_ok=True)
sha  = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
(path := pkg / "_build_info.py").write_text(f'GIT_SHA = "{sha}"\n', encoding="utf-8")
print("Wrote", path)
#PY