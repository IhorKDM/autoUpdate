'use strict';
document.querySelector('#demo-form').addEventListener('submit', (event) => {
  event.preventDefault();
  document.querySelector('#demo-output').textContent = document.querySelector('#demo-input').value;
  document.querySelector('#demo-output-second').textContent = document.querySelector('#demo-input-second').value;
});

async function loadRelease() {
  const status = document.querySelector('#release-status');
  try {
    const configResponse = await fetch('./config.json');
    if (!configResponse.ok) throw new Error('Configuration unavailable');
    const { repository, assets } = await configResponse.json();
    if (!/^[A-Za-z0-9_.-]+\/[A-Za-z0-9_.-]+$/.test(repository)) throw new Error('Invalid repository');
    // Set download links before querying GitHub: downloads still work if its API is rate limited.
    for (const link of document.querySelectorAll('[data-platform]')) {
      const asset = assets[link.dataset.platform];
      if (asset) link.href = `https://github.com/${repository}/releases/latest/download/${encodeURIComponent(asset)}`;
    }
    const response = await fetch(`https://api.github.com/repos/${repository}/releases/latest`);
    if (response.status === 404) {
      status.textContent = 'First release is being prepared. Downloads will be available shortly.';
      return;
    }
    if (!response.ok) throw new Error('Release API unavailable');
    const release = await response.json();
    const date = new Date(release.published_at).toLocaleDateString(undefined, { year: 'numeric', month: 'short', day: 'numeric' });
    status.textContent = `${release.tag_name} · Released ${date} · Free to download`;
  } catch {
    status.textContent = 'Download the latest release above. All versions are available on GitHub.';
  }
}
loadRelease();
