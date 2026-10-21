.PHONY: setup configure build test web site
setup:
	python3 -m venv .venv
	.venv/bin/python -m pip install -r requirements-dev.txt
configure:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
build: configure
	cmake --build build --config Release
test: build
	ctest --test-dir build -C Release --output-on-failure
	.venv/bin/python -m pytest -q
web:
	.venv/bin/python -m uvicorn web.main:app --host 127.0.0.1 --port 8000
site:
	.venv/bin/python scripts/build_site.py site-dist
