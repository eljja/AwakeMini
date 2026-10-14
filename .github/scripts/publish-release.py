"""Publish verified packaged files; never execute the Windows executables.
Only new versions are published. Existing published releases are left intact.
An interrupted draft upload may be resumed by rerunning the workflow.
"""
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess, tempfile, zipfile


def prepare(root):
    report = json.loads((root / 'VERIFICATION.json').read_text())
    version = report['version']
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.]+)?', version):
        raise ValueError('Invalid version')
    directory = root / 'downloads' / ('v' + version)
    files = []
    for entry in report['executables']:
        path = directory / entry['filename']
        if path.parent != directory or not path.name.startswith('AwakeMini-v' + version + '-'):
            raise ValueError('Unexpected executable path')
        data = path.read_bytes()
        if len(data) != entry['bytes'] or hashlib.sha256(data).hexdigest() != entry['sha256']:
            raise ValueError('Executable checksum mismatch: ' + path.name)
        files.append(path)
    expected = {f'AwakeMini-v{version}-{arch}-{language}.exe'
                for arch in ('x86', 'x64') for language in ('auto', 'ko', 'en')}
    if len(files) != 6 or {p.name for p in files} != expected:
        raise ValueError('Six language/architecture builds are required')
    archive = directory / f'AwakeMini-v{version}-source.zip'
    with zipfile.ZipFile(archive) as bundle:
        if bundle.testzip() is not None:
            raise ValueError('Invalid archive')
        for path in files + [root / name for name in ('README.md', 'README.ko.md', 'README.en.md')]:
            if bundle.read(str(path.relative_to(root))) != path.read_bytes():
                raise ValueError('Archive is stale: ' + path.name)
    files.extend([archive, root / 'SHA256SUMS.txt'])
    notes = f'# Awake Mini {version}\n\n'
    for name, heading in [('README.ko.md', '## 최신 변경 사항'), ('README.en.md', "## What's new")]:
        text = (root / name).read_text()
        if heading in text:
            section = text.split(heading, 1)[1].split('\n## ', 1)[0].strip()
            notes += section + '\n\n'
    notes += ('EXE: x64/x86 × auto/ko/en. ZIP: six executables, source, Korean/English READMEs.\n\n'
              'Native Windows execution and SSO retention remain unverified. '
              '실제 Windows 실행과 SSO 유지 효과는 미검증입니다.\n\n'
              'Exit an older instance before launching. 기존 프로그램을 종료한 뒤 실행하세요.\n')
    return version, files, notes


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    version = json.loads((root / 'VERIFICATION.json').read_text())['version']
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.]+)?', version):
        raise ValueError('Invalid version')
    tag = 'v' + version
    if args.dry_run:
        version, files, notes = prepare(root)
        print(json.dumps({'tag': tag, 'assets': [p.name for p in files], 'checksums': 'passed'}))
        return
    repository = os.environ['GITHUB_REPOSITORY']
    commit = os.environ['GITHUB_SHA']
    if repository != 'eljja/AwakeMini' or not re.fullmatch(r'[0-9a-f]{40}', commit):
        raise ValueError('Unexpected repository/commit')
    existing = subprocess.run(['gh', 'release', 'view', tag, '--repo', repository,
                               '--json', 'isDraft,targetCommitish'], capture_output=True, text=True)
    if existing.returncode == 0:
        release = json.loads(existing.stdout)
        if not release['isDraft']:
            print('Already published; unchanged: ' + tag)
            return
        if release['targetCommitish'] != commit:
            raise ValueError('Existing draft targets another commit; inspect before retrying')
    elif 'release not found' not in existing.stderr.lower() and 'not found' not in existing.stderr.lower():
        raise RuntimeError('Cannot inspect existing release: ' + existing.stderr.strip())
    version, files, notes = prepare(root)
    prerelease = '-' in version or os.environ.get('GITHUB_REF_NAME') != os.environ.get('GITHUB_DEFAULT_BRANCH', 'experiment/presentation-media-state')
    with tempfile.TemporaryDirectory() as temporary:
        note_file = Path(temporary) / 'release-notes.md'
        note_file.write_text(notes)
        if existing.returncode != 0:
            command = ['gh', 'release', 'create', tag, '--repo', repository, '--target', commit,
                       '--title', 'Awake Mini ' + version, '--notes-file', str(note_file), '--draft']
            if prerelease:
                command += ['--prerelease']
            subprocess.run(command, check=True)
        subprocess.run(['gh', 'release', 'upload', tag, '--repo', repository, '--clobber',
                        *[str(p) for p in files]], check=True)
        subprocess.run(['gh', 'release', 'edit', tag, '--repo', repository, '--draft=false',
                        '--prerelease=' + str(prerelease).lower(),
                        '--latest=' + str(not prerelease).lower(), '--notes-file', str(note_file)], check=True)
    print('Published: ' + tag)


if __name__ == '__main__':
    main()
