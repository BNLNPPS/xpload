#!/usr/bin/env python3

import hashlib
import json
import jsonschema
import os
import pathlib
import requests
import shutil
import stat
import sys

try:
    from xpload_config import *
except ImportError:
    __version__ = "0.0.0-notinstalled"
    XPLOAD_CONFIG_SEARCH_PATHS = [".", "config"]


general_schema = {
    "definitions" : {
        "entry": {
            "properties" : {
                "id" : {"type" : "integer"},
                "name" : {
                    "oneOf": [
                        {"type": "string"},
                        {"type": "array", "items": {"type": "string"}}
                    ]
                },
                "detail" : {"type" : "string"}
            }
        }
    },
    "oneOf": [
        {"type": "object", "$ref": "#/definitions/entry"},
        {"type": "array", "items": {"$ref": "#/definitions/entry"}}
    ]
}

tags_schema = {
    "type": "array",
    "items": {
        "properties" : {
            "name" : {"type" : "string"},
            "type" : {"type" : "string"},
            "status" : {"type" : "string"},
            "domains" : {"type" : "array", "items": {"type": "string"}}
        },
        "required": ["name", "type", "status", "domains"]
    }
}


from collections import namedtuple


def nestednamedtuple(obj):
    """ Returns nested list of namedtuples """

    if isinstance(obj, dict):
        fields = obj.keys()
        namedtuple_type = namedtuple(typename='NNTuple', field_names=fields)
        field_value_pairs = {str(field): nestednamedtuple(obj[field]) for field in fields}
        try:
            return namedtuple_type(**field_value_pairs)
        except TypeError:
            # If namedtuple cannot be created fallback to dict
            return dict(**field_value_pairs)
    elif isinstance(obj, (list, set, tuple, frozenset)):
        return [nestednamedtuple(item) for item in obj]
    else:
        return obj


def _vlprint(minverb, msg):
    if cfg.verbosity >= 3:
        print(f"VL{minverb}:", end="")
    if cfg.verbosity >= minverb:
        print(msg)


import types
class Config(types.SimpleNamespace):

    def __init__(cls, cfgf, **kwargs):
        kwargs['cfgf'] = cfgf
        super().__init__(**kwargs)

    def url(self):
        return "http://" + self.host + ':' + self.port + self.apiroot


def read_config(config_name, verbosity: int = None):
    """ Read database parameters from a json config file """

    # Use user supplied config as is if it looks like a "path"
    if "." in config_name or "/" in config_name:
        with open(f"{config_name}") as cfgf:
            return json.load(cfgf, object_hook=lambda d: Config(os.path.realpath(cfgf.name), **d))

    XPLOAD_CONFIG_DIR = os.getenv('XPLOAD_CONFIG_DIR', "").rstrip("/")
    XPLOAD_CONFIG = os.getenv('XPLOAD_CONFIG', "prod")

    search_paths = [XPLOAD_CONFIG_DIR] if XPLOAD_CONFIG_DIR else XPLOAD_CONFIG_SEARCH_PATHS

    if config_name:
        config_file = f"{config_name}.json"
    else:
        config_file = f"{XPLOAD_CONFIG}.json"

    config = None
    for config_path in search_paths:
        try:
            with open(f"{config_path}/{config_file}") as cfgf:
                config = json.load(cfgf, object_hook=lambda d: Config(os.path.realpath(cfgf.name), **d))
        except:
            pass

    if config:
        if verbosity:
            config.verbosity = verbosity
    else:
        print(f"Error: Cannot find config file {config_file} in", search_paths)

    return config


def _get_data(endpoint: str, params: dict = {}):
    """ Get data from the endpoint """

    endpoints = ['gtPayloadLists']
    for ep in endpoints:
        if ep not in endpoint:
            raise RuntimeError(f"Wrong endpoint {endpoint}")

    url = cfg.url() + "/" + endpoint
    _vlprint(3, f"-H 'Content-Type: application/json' -X GET -d '{json.dumps(params)}' {url}")

    try:
        response = requests.get(url=url, json=params)
        response.raise_for_status()
        respjson = response.json()
    except Exception as e:
        respmsg = f"{json.dumps(respjson)} " if respjson else ""
        raise RuntimeError(f"Unexpected response for GET {json.dumps(params)} {url}: " + respmsg + repr(e))

    return respjson


def _post_data(endpoint: str, params: dict):
    """ Post data to the endpoint """

    if endpoint not in ['gttype', 'gtstatus', 'gt', 'pt', 'pl', 'piov', 'pil', 'tag']:
        raise RuntimeError(f"Wrong endpoint {endpoint}")

    url = cfg.url() + "/" + endpoint
    _vlprint(3, f"-H 'Content-Type: application/json' -X POST -d '{json.dumps(params)}' {url}")

    respjson = None
    try:
        response = requests.post(url=url, json=params)
        try:
            response.raise_for_status()
        except requests.exceptions.HTTPError as e:
            if e.response.status_code < 400 or e.response.status_code >= 500:
                raise
        respjson = response.json()
        jsonschema.validate(respjson, general_schema)
    except Exception as e:
        respmsg = f"{json.dumps(respjson)} " if respjson else ""
        raise RuntimeError(f"Unexpected response for POST '{json.dumps(params)}' {url}: " + respmsg + repr(e))

    return respjson


def _put_data(endpoint: str, params: dict):
    """ Put data to the endpoint """

    if endpoint not in ['pl_attach', 'piov_attach', 'gt_change_status']:
        raise RuntimeError(f"Wrong endpoint {endpoint}")

    url = cfg.url() + "/" + endpoint
    _vlprint(3, f"-H 'Content-Type: application/json' -X PUT -d '{json.dumps(params)}' {url}")

    respjson = None
    try:
        response = requests.put(url=url, json=params)
        response.raise_for_status()
        respjson = response.json()
        jsonschema.validate(respjson, general_schema)
    except Exception as e:
        respmsg = f"{json.dumps(respjson)} " if respjson else ""
        raise RuntimeError(f"Unexpected response for PUT '{json.dumps(params)}' {url}: " + respmsg + repr(e))

    return respjson['name'] if 'name' in respjson else respjson['id']


def create_and_link_tag(tag_name: str, tag_type: str, tag_status: str, domains: list):
    _post_data('gttype', {"name": tag_type})
    _post_data('gtstatus', {"name": tag_status})
    response = _post_data('gt', {"name": tag_name, "status": tag_status, "type": tag_type})['name']
    for domain in domains:
        _post_data('pt', {"name": domain})
        try:
            pill = _get_data(f'gtPayloadLists/{tag_name}')[domain]
        except:
            pill = _post_data('pl', {"payload_type": domain})['name']
        _put_data('pl_attach', {"global_tag": tag_name, "payload_list": pill})
    # Entities created if not existed but it is not an error
    return 'ok' if response == tag_name else response


def create_and_link_pil(tag_name: str, domain: str, name: str, start: int, end: int, dry_run: bool):
    try:
        pill = _get_data(f'gtPayloadLists/{tag_name}')[domain]
    except Exception as e:
        raise RuntimeError(f"{tag_name}/{domain} does not exist: " + repr(e))

    if dry_run: return

    params = {"payload_url": name, "minor_iov": start, "major_iov": 0}
    if end:
        params["minor_iov_end"] = end
    piov_id = _post_data('piov', params)['id']
    _put_data('piov_attach', {"piov_id": piov_id, "payload_list": pill})
    return 'ok'


def form_api_url(component: str, uid: int = None):
    url = cfg.url()

    if component == 'tags':
        url += "/gt"
    elif component == 'tag_types':
        url += "/gttype"
    elif component == 'tag_statuses':
        url += "/gtstatus"
    elif component == 'domains':
        url += "/pt"
    elif component == 'domain_lists':
        url += "/pl"
    elif component == 'payloads':
        url += "/piov"
    else:
        print(f"Error: Wrong component {component}. Cannot form valid URL")

    if uid is not None:
        url += f"/{uid}"

    return url


def fetch_entries(component: str, uid: int = None):
    """ Fetch entries using respective endpoints """
    url = form_api_url(component, uid)

    try:
        response = requests.get(url)
        respjson = response.json()
    except:
        print(f"Error: Something went wrong while looking for {component} at {url}")
        return []

    # Always return a list
    entries = respjson if isinstance(respjson, list) else [respjson]

    try:
        jsonschema.validate(entries, general_schema)
    except:
        error_details = f": {component} may not contain entry with id={uid}" if uid else ""
        print(f"Error: Encountered invalid response from {url}", error_details)
        return []

    return entries


def payload_exists(payload_name: str) -> pathlib.Path:
    prefixes = cfg.path if isinstance(cfg.path, list) else [cfg.path]

    for prefix in prefixes:
        payload_file = pathlib.Path(prefix)/payload_name
        if payload_file.exists():
            return payload_file

    return None


def payload_copy(payload_file: pathlib.Path, prefixes: list, domain: str, dry_run=False) -> pathlib.Path:
    """ Copies `payload_file` to the first valid `prefix` from the `prefixes` list """

    # Check if file exists
    if not payload_file.exists():
        raise FileExistsError("File not found: " + str(payload_file))

    # Check if at least one prefix exists and is writeable
    good_prefixes = [prefix for prefix in prefixes if prefix.exists() and os.stat(prefix).st_mode & (stat.S_IXUSR | stat.S_IWUSR) ]
    if not good_prefixes:
        raise RuntimeError("No writable prefix provided: " + ":".join(map(str, prefixes)))

    # The first good prefix is the prefix
    prefix = good_prefixes[0]

    # Extract basename, create payload name
    md5sum = hashlib.md5(payload_file.open('rb').read()).hexdigest()
    payload_name = f"{md5sum}_{payload_file.name}"
    destination = prefix/domain/payload_name

    if dry_run:
        return destination

    destination.parent.mkdir(parents=True, exist_ok=True)
    # XXX Check destination file already exists?
    shutil.copyfile(payload_file, destination)
    # Verify the copy
    md5sum_dst = hashlib.md5(destination.open('rb').read()).hexdigest()
    if md5sum != md5sum_dst:
        raise RuntimeError("Failed to copy payload file to ", destination)

    return destination


def add_tag(tag_name: str, tag_type: str, tag_status: str, tag_domains: list = []):
    # Remove duplicates in tag_domains
    tag_domains = list(set(tag_domains))
    # Use staged tags if exist
    tags_file = pathlib.Path.cwd()/'.xpload'/'tags.json'
    # A list of payload intervals loaded from tags_file
    tags = []

    try:
        tags = json.load(tags_file.open())
    except OSError: # File not found. Create it silently
        tags_file.parent.mkdir(exist_ok=True)
    except json.JSONDecodeError as e: # json is not valid
        raise RuntimeError("Found invalid tags stage. Fix or remove and try again: " + repr(e))

    # Select a tag to update from the existing tags if any
    existing_tags = [indx for indx, tag in enumerate(tags) if tag['name'] == tag_name]

    if len(existing_tags) >= 2:
        raise RuntimeError(f"Found invalid tags stage. Only one entry for \"{tag_name}\" can be staged")

    # Remove tags with name tag_name if exist
    tags = [tag for tag in tags if tag['name'] != tag_name]
    # Insert updated tag
    tag_entry = dict(name=tag_name, type=tag_type, status=tag_status, domains=tag_domains)
    tags.append(tag_entry)

    tags_file.write_text(json.dumps(tags, indent=2))


def add_pil(tag: str, domain: str, payload: pathlib.Path, start: int, end: int = None):
    # Make assumptions about input values
    if end:
        assert start > 0 and end > start, "start must be greater than zero, end must be greater than start"
    # Use staged pils if exist
    pils_file = pathlib.Path.cwd()/'.xpload'/'pils.json'
    # A list of payload intervals loaded from pils_file
    pils = []

    try:
        pils = json.load(pils_file.open())
    except OSError: # File not found. Create it silently
        pils_file.parent.mkdir(exist_ok=True)
    except json.JSONDecodeError as e: # json is not valid
        raise RuntimeError("Found invalid pils stage. Fix or remove and try again: " + repr(e))

    # Select a pil to update from the existing pils if any
    existing_pils = [indx for indx, pil in enumerate(pils) if pil['tag'] == tag and pil['domain'] == domain]

    if len(existing_pils) >= 2:
        raise RuntimeError(f"Found invalid pils stage. Only one entry for '{tag}' and '{domain}' can be staged")

    # Get payloads from the existing pil if any and update
    payloads = pils.pop(existing_pils[0])['payloads'] if existing_pils else []
    # Remove all payloads with the requested start and end values
    payloads = [pld for pld in payloads if pld['start'] != start or pld['end'] != end]
    # Finally, insert new payload with the requested parameters
    payloads.append(dict(path=str(payload), start=start, end=end))
    # Insert updated pil
    pil_entry = dict(tag=tag, domain=domain, payloads=payloads)
    pils.append(pil_entry)

    pils_file.write_text(json.dumps(pils, indent=2))


def push_tags():
    """ Push all staged tags. The policy is to push all or nothing. If there is
    an error nothing should be pushed but the current API for tag creation never
    fails.
    """
    tags_file = pathlib.Path.cwd()/'.xpload'/'tags.json'

    try:
        tags = json.load(tags_file.open())
        jsonschema.validate(tags, tags_schema)
    except OSError as e:
        raise RuntimeError("No stage found. Use \"add\" action to stage tags: " + repr(e))
    except (json.JSONDecodeError, jsonschema.exceptions.ValidationError) as e:
        raise RuntimeError("Invalid stage found. Fix or remove the stage and try again: " + repr(e))

    for tag in tags:
        response = create_and_link_tag(tag['name'], tag['type'], tag['status'], tag['domains'])
        _vlprint(1, f'Creating tag {tag["name"]}... {response}')

    # Assume all tags were pushed succesfully and remove the file
    tags_file.unlink()


def push_pils():
    """ Push all staged payload intervals. The policy is to push all or nothing.
    If there is an error nothing should be pushed.
    """
    pils_file = pathlib.Path.cwd()/'.xpload'/'pils.json'

    try:
        pils = json.load(pils_file.open())
        # XXX validate pils json schema
    except OSError as e:
        raise RuntimeError("No stage found. Use \"add\" action to stage intervals: " + repr(e))
    except json.JSONDecodeError as e:
        raise RuntimeError("Invalid stage found. Fix or remove the stage and try again: " + repr(e))

    prefixes = cfg.path if isinstance(cfg.path, list) else [cfg.path]
    prefixes = [pathlib.Path(prefix) for prefix in prefixes]

    def push_pil(dry_run=True):
        for pil in pils:
            for payload in pil['payloads']:
                copy_args = (pathlib.Path(payload['path']), prefixes, pil['domain'])
                destination = payload_copy(*copy_args, dry_run)
                _vlprint(2+int(dry_run), f"Copying payload file {destination}...")
                create_and_link_pil(pil['tag'], pil['domain'], destination.name, payload['start'], payload['end'], dry_run)

    # Do a dry run first
    _vlprint(3, "Dry run")
    push_pil(True)
    # Can proceed if the above passed without exceptions
    _vlprint(3, "Real run")
    push_pil(False)
    # Assume all pils were pushed succesfully and remove the file
    pils_file.unlink()


def push():
    tags_file = pathlib.Path.cwd()/'.xpload'/'tags.json'
    pils_file = pathlib.Path.cwd()/'.xpload'/'pils.json'

    if not tags_file.exists() and not pils_file.exists():
        raise RuntimeError("No stage found. Use 'add' action to stage tags and/or pils")
    if tags_file.exists(): push_tags()
    if pils_file.exists(): push_pils()


def fetch_payloads(tag: str, domain: str, treq: int):

    url = f"{cfg.url()}/payloadiovs/?gtName={tag}&majorIOV=0&minorIOV={treq}"

    try:
        response = requests.get(url)
        respjson = response.json()
    except:
        print(f"Error: Something went wrong while looking for tag {tag} and time {treq}. Check", url)
        return []

    # Always return a list
    respjson = respjson if isinstance(respjson, list) else [respjson]

    if domain:
        respjson = [e for e in respjson if e['payload_type'] == domain]

    return respjson


def act_on(args):
    if args.action == 'config':
        config_dict = cfg.__dict__
        try:
            for ix in args.field:
                config_dict = config_dict[int(ix) if ix.isnumeric() else ix]
        except: pass
        print(config_dict)

    if args.action == 'show':
        respjson = fetch_entries(args.component)
        pprint_named_entries(respjson, args.dump)

    if args.action == 'add':
        try:
            if args.subaction == 'tag':
                add_tag(args.name, args.type, args.status, args.domains)
            if args.subaction == 'pil':
                add_pil(args.tag, args.domain, args.payload, args.start, args.end)
        except Exception as e:
            print("Error:", type(e).__name__, e)
            sys.exit(os.EX_OSFILE)

    if args.action == 'push':
        try:
            push()
        except Exception as e:
            print("Error:", type(e).__name__, e)
            sys.exit(os.EX_OSFILE)

    if args.action == 'fetch':
        respjson = fetch_payloads(args.tag, args.domain, args.treq)
        try:
            pprint_payload(respjson, args.dump)
        except FileExistsError as e:
            print("Error:", type(e).__name__, e)
            sys.exit(os.EX_OSFILE)


def pprint_named_entries(respjson, dump: bool):
    """ Pretty print named entries """

    if dump:
        print(json.dumps(respjson, indent=4))
    else:
        objs = nestednamedtuple(respjson)
        tuples = [(o.name,) for o in objs]
        tuples = sorted(tuples, key=lambda tup: (tup[0],))
        for t in tuples:
            print(*t)


def pprint_payload(respjson, dump: bool):
    """ Pretty print payload entries """

    if dump:
        print(json.dumps(respjson, indent=4))
    else:
        objs = nestednamedtuple(respjson)
        paths = [str(payload_exists(f'{o.payload_type}/{p.payload_url}')) for o in objs for p in o.payload_iov if payload_exists(f'{o.payload_type}/{p.payload_url}')]
        if paths:
            print("\n".join(sorted(paths)))
        else:
            raise FileExistsError("No payload file was found in any prefix")


def NonEmptyStr(value: str):
    value = value.strip()
    if not value:
        raise argparse.ArgumentTypeError("Must provide a non-empty string")
    return value


def NonNegativeInt(value: str):
    try:
        value = int(value)
        if value < 0: raise ValueError
    except ValueError:
        raise argparse.ArgumentTypeError("Must provide a non-negative integer value")
    return value


def FilePathType(value: str):
    value = pathlib.Path(value)
    if not value.exists():
        raise argparse.ArgumentTypeError(f"File not found")
    return value


if __name__ == "__main__":
    """ Main entry point for xpload utility """
    import argparse
    parser = argparse.ArgumentParser(description="Manipulate payload entries")
    parser.add_argument("-c", "--config", type=str, default="", help="Config file with database connection parameters")
    parser.add_argument("-d", "--dump", action='store_true', default=False, help="Dump response as json instead of pretty printing it")
    parser.add_argument(      "--version", action='version', version=__version__)
    parser.add_argument("-v", "--verbosity", type=NonNegativeInt, default=None, help="Verbosity level")

    # Parse various actions
    subparsers = parser.add_subparsers(dest="action", required=True, help="Choose one of the actions")

    # Action: show
    parser_show = subparsers.add_parser("show", help="Show entries")
    parser_show.add_argument("component", type=str, choices=['tags', 'domains'], help="Pick a list to show available entries")

    # Action: add
    parser_add = subparsers.add_parser("add", help="Stage payload intervals")
    subparsers_add = parser_add.add_subparsers(dest="subaction", required=True, help="Choose one")

    parser_add_tag = subparsers_add.add_parser("tag", help="Add a tag for payload intervals")
    parser_add_tag.add_argument("name", type=NonEmptyStr, help="Tag name")
    parser_add_tag.add_argument("-t", "--type", type=NonEmptyStr, default='online', help="Tag type")
    parser_add_tag.add_argument("-s", "--status", type=NonEmptyStr, default='unlocked', help="Tag status")
    parser_add_tag.add_argument("-d", "--domains", type=NonEmptyStr, nargs='+', default=[], help="Link new domains to the tag")

    parser_add_pil = subparsers_add.add_parser("pil", help="Add a payload interval")
    parser_add_pil.add_argument("tag", type=NonEmptyStr, help="Tag for payload intervals")
    parser_add_pil.add_argument("domain", type=NonEmptyStr, help="Domain of payload data")
    parser_add_pil.add_argument("payload", type=FilePathType, help=f"Payload file")
    parser_add_pil.add_argument("-s", "--start", type=NonNegativeInt, default=0, help="A non-negative integer representing the start of interval when the payload is applied")
    parser_add_pil.add_argument("-e", "--end", type=NonNegativeInt, default=None, help="A non-negative integer representing the end of interval when the payload is applied")

    # Action: push
    parser_push = subparsers.add_parser("push", help="Push staged payload interval list")

    # Action: fetch
    parser_fetch = subparsers.add_parser("fetch", help="Fetch one or more payload entries")
    parser_fetch.add_argument("tag", type=NonEmptyStr, help="Tag for payload intervals")
    parser_fetch.add_argument("-d", "--domain", type=NonEmptyStr, default=None, help="Domain of payload data")
    parser_fetch.add_argument("-t", "--treq", type=NonNegativeInt, default=sys.maxsize, help="A non-negative integer representing a time within a payload interval")

    # Action: config
    parser_config = subparsers.add_parser("config", help="Print configuration values")
    parser_config.add_argument("field", type=NonEmptyStr, nargs='*', help="Print configuration field value")

    args = parser.parse_args()

    global cfg
    cfg = read_config(args.config, args.verbosity)

    if not cfg:
        sys.exit(os.EX_CONFIG)

    act_on(args)
