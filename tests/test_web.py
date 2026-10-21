import json
from fastapi.testclient import TestClient
from scripts.build_site import build
from web.main import app

client = TestClient(app)

def test_page_and_assets():
    page = client.get('/')
    assert page.status_code == 200
    assert 'Download for macOS' in page.text
    assert 'Download for Windows' in page.text
    for asset, content_type in [('app.js', 'javascript'), ('style.css', 'css')]:
        response = client.get(f'/static/{asset}')
        assert response.status_code == 200
        assert content_type in response.headers['content-type']
    assert client.get('/health').json() == {'status': 'ok'}

def test_download_configuration(monkeypatch):
    monkeypatch.setenv('RELEASE_REPOSITORY', '')
    assert client.get('/download/macos').status_code == 503
    assert client.get('/config.json').status_code == 503
    assert client.get('/download/linux').status_code == 404
    monkeypatch.setenv('RELEASE_REPOSITORY', 'learner/text-echo')
    for platform, asset in [('macos', 'macos-arm64'), ('windows', 'windows-x64')]:
        response = client.get(f'/download/{platform}', follow_redirects=False)
        assert response.status_code == 307
        assert response.headers['location'] == f'https://github.com/learner/text-echo/releases/latest/download/TextEcho-{asset}.zip'
    assert client.get('/config.json').json()['repository'] == 'learner/text-echo'

def test_invalid_repository_cannot_redirect_elsewhere(monkeypatch):
    for repo in ['https://evil.test/x', '../other', 'owner/..', 'owner/.', 'owner/repo?bad', 'owner/repo/extra']:
        monkeypatch.setenv('RELEASE_REPOSITORY', repo)
        assert client.get('/download/windows').status_code == 503

def test_static_export_has_matching_configuration(tmp_path, monkeypatch):
    monkeypatch.setenv('RELEASE_REPOSITORY', 'learner/example')
    build(tmp_path)
    assert json.loads((tmp_path / 'config.json').read_text()) == client.get('/config.json').json()
    assert (tmp_path / 'index.html').read_bytes() == (tmp_path / 'static/index.html').read_bytes()
    assert (tmp_path / 'static/app.js').is_file()
