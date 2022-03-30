import hashlib
import pathlib
import pytest
import shutil
import xpload


@pytest.fixture
def payload_copy_params(request):
    payload_file = pathlib.Path('/tmp/file.data')
    prefixes = [pathlib.Path('/tmp/data0'), pathlib.Path('/tmp/data1')]
    modes = request.param[:2]
    domain = 'some_domain'

    payload_file.write_bytes(b'012345')
    for prefix, mode in zip(prefixes, modes):
        prefix.mkdir()
        prefix.chmod(mode) # Set access bits, e.g. rwxrwxrwx = 0o777

    yield (payload_file, prefixes, domain)

    # Clean up
    payload_file.unlink()
    for prefix in prefixes:
        shutil.rmtree(prefix, ignore_errors=True)


@pytest.mark.parametrize('payload_copy_params', [[0o700, 0o700], [0o400, 0o700], [0o700, 0o400]], indirect=True)
def test_payload_copy(payload_copy_params):
    (src, prefixes, domain) = payload_copy_params

    dst = xpload.payload_copy(src, prefixes, domain)
    md5sum = hashlib.md5(dst.open('rb').read()).hexdigest()

    assert dst.exists() and \
           (dst == prefixes[0]/domain/f'{md5sum}_{src.name}' or dst == prefixes[1]/domain/f'{md5sum}_{src.name}')


@pytest.mark.parametrize('payload_copy_params', [[0o400, 0o400]], indirect=True)
def test_payload_copy_fail(payload_copy_params):
    (src, prefixes, domain) = payload_copy_params

    with pytest.raises(Exception) as e_info:
        dst = xpload.payload_copy(src, prefixes, domain)
