#!/usr/bin/env python3

import json
import jsonschema
import os
import pathlib
import requests
import sys

try:
    from xpload_config import *
except ImportError:
    __version__ = "0.0.0-notinstalled"
    XPLOAD_CONFIG_SEARCH_PATHS = [".", "config"]
    pass


general_schema = {
    "definitions" : {
        "entry": {
            "properties" : {
                "id" : {"type" : "integer"},
                "name" : {"type" : "string"}
            },
            "required": ["id"]
        }
    },

    "type": "array",
    "items": {
        "$ref": "#/definitions/entry"
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


class DbConfig(namedtuple('DbConfig', ['cfgf', 'host', 'port', 'apiroot', 'apiver', 'path'])):
    __slots__ = ()

    def __new__(cls, cfgf, **kwargs):
        kwargs = {k: v for k, v in kwargs.items() if k in cls._fields}
        kwargs['cfgf'] = cfgf
        return super().__new__(cls, **kwargs)

    def url(self):
        return "http://" + self.host + ':' + self.port + self.apiroot


def config_db(config_name):
    """ Read database parameters from a json config file """

    # Use user supplied config as is if it looks like a "path"
    if "." in config_name or "/" in config_name:
        with open(f"{config_name}") as cfgf:
            return json.load(cfgf, object_hook=lambda d: DbConfig(os.path.realpath(cfgf.name), **d))

    XPLOAD_CONFIG_DIR = os.getenv('XPLOAD_CONFIG_DIR', "").rstrip("/")
    XPLOAD_CONFIG = os.getenv('XPLOAD_CONFIG', "prod")

    search_paths = [XPLOAD_CONFIG_DIR] if XPLOAD_CONFIG_DIR else XPLOAD_CONFIG_SEARCH_PATHS

    if config_name:
        config_file = f"{config_name}.json"
    else:
        config_file = f"{XPLOAD_CONFIG}.json"

    for config_path in search_paths:
        try:
            with open(f"{config_path}/{config_file}") as cfgf:
                return json.load(cfgf, object_hook=lambda d: DbConfig(os.path.realpath(cfgf.name), **d))
        except:
            pass

    print(f"Error: Cannot find config file {config_file} in", search_paths)
    return []


def _post_data(endpoint: str, params: dict):
    """ Post data to the endpoint """

    if endpoint not in ['gttype', 'gtstatus', 'gt', 'pt', 'pl', 'piov']:
        print(f"Error: Wrong endpoint {endpoint}")
        return None

    url = db.url() + "/" + endpoint

    try:
        response = requests.post(url=url, json=params)
        respjson = response.json()
    except:
        print(f"Error: Something went wrong while posting {json.dumps(params)} to {url}")
        return None

    try:
        jsonschema.validate(respjson, general_schema['definitions']['entry'])
    except:
        print(f"Error: Encountered invalid response while posting {json.dumps(params)} to {url}")
        return None

    return respjson['name']


def create_tag_type(name="test"):
    return _post_data('gttype', {"name": name})

def create_tag_status(name="test"):
    return _post_data('gtstatus', {"name": name})

def create_tag(name, type_name: str, status_name: str):
    return _post_data('gt', {"name": name, "status": status_name, "type": type_name})

def create_domain(name):
    return _post_data('pt', {"name": name})

def create_domain_list(tag_name, domain_name):
    return _post_data('pl', {"global_tag": tag_name, "payload_type": domain_name})

def create_payload(name, domain_list_name, start):
    return _post_data('piov', {"payload_url": name, "payload_list": domain_list_name, "major_iov": 0, "minor_iov": start})


def form_api_url(component: str, uid: int = None):
    url = db.url()

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
    prefixes = db.path if isinstance(db.path, list) else [db.path]

    for prefix in prefixes:
        payload_file = pathlib.Path(prefix)/payload_name
        if payload_file.exists():
            return payload_file

    return None


def fetch_payloads(tag: str, domain: str, start: int):

    url = f"{db.url()}/payloadiovs/?gtName={tag}&majorIOV=0&minorIOV={start}"

    try:
        response = requests.get(url)
        respjson = response.json()
    except:
        print(f"Error: Something went wrong while looking for tag {tag} and start time {start}. Check", url)
        return []

    # Always return a list
    respjson = respjson if isinstance(respjson, list) else [respjson]

    if domain:
        respjson = [e for e in respjson if e['payload_type'] == domain]

    return respjson


def insert_payload(tag: str, domain: str, payload: str, start: int = 0):
    """ Inserts an entry into corresponding tables """

    # Select the last entry if exists or create a new default one
    tag_types = fetch_entries("tag_types")
    tag_type_name = create_tag_type() if not tag_types else tag_types[-1]['name']

    tag_statuses = fetch_entries("tag_statuses")
    tag_status_name = create_tag_status() if not tag_statuses else tag_statuses[-1]['name']

    # Get all tags
    tags = fetch_entries("tags")
    # Select the last matching entry
    existing_tag = next((e for e in reversed(tags) if e['name'] == tag), None)

    # If the tag does not exist create one
    if existing_tag is None:
        tag_name = create_tag(tag, tag_type_name, tag_status_name)
        print(f"Tag {tag} does not exist. Created {tag_name}")
    else:
        tag_name = existing_tag['name']

    # Get all domains
    domains = fetch_entries("domains")
    # Select the last matching entry
    existing_domain = next((e for e in reversed(domains) if e['name'] == domain), None)

    # If the domain does not exist create one
    if existing_domain is None:
        domain_name = create_domain(domain)
        print(f"Domain {domain} does not exist. Created {domain_name}")
    else:
        domain_name = existing_domain['name']

    # Check if domain_list with tag_name and domain_name exists
    domain_lists = fetch_entries("domain_lists")
    # Select the last matching entry
    existing_domain_list = next((e for e in reversed(domain_lists) if e['global_tag'] == tag_name and e['payload_type'] == domain_name), None)

    # If either tag or domain did not exist, create a new domain_list
    if existing_tag is None or existing_domain is None or existing_domain_list is None:
        domain_list_name = create_domain_list(tag_name, domain_name)
    else:
        domain_list_name = existing_domain_list['name']

    payload_name = create_payload(payload, domain_list_name, start)
    print(f"Payload {payload} does not exist. Created {payload_name}")


def act_on(args):
    if args.action == 'config':
        config_dict = db._asdict()
        try:
            for ix in args.field:
                config_dict = config_dict[int(ix) if ix.isnumeric() else ix]
        except: pass
        print(config_dict)

    if args.action == 'show':
        respjson = fetch_entries(args.component, args.id)
        pprint_tags(respjson, args.dump)

    if args.action == 'insert':
        insert_payload(args.tag, args.domain, args.payload, args.start)

    if args.action == 'fetch':
        respjson = fetch_payloads(args.tag, args.domain, args.start)
        try:
            pprint_payload(respjson, args.dump)
        except FileExistsError as e:
            print(e)
            sys.exit(os.EX_OSFILE)


def pprint_tags(respjson, dump: bool):
    """ Pretty print tag entries """

    if dump:
        print(json.dumps(respjson, indent=4))
    else:
        objs = nestednamedtuple(respjson)
        for o in objs:
            print(f"{o.name}")


def pprint_payload(respjson, dump: bool):
    """ Pretty print payload entries """

    if dump:
        print(json.dumps(respjson, indent=4))
    else:
        objs = nestednamedtuple(respjson)
        paths = [str(payload_exists(p.payload_url)) for o in objs for p in o.payload_iov if payload_exists(p.payload_url)]
        if paths:
            print("\n".join(paths))
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
    parser.add_argument("-v", "--version", action='version', version=__version__)

    # Parse various actions
    subparsers = parser.add_subparsers(dest="action", required=True, help="Choose one of the actions")

    # Action: show
    parser_show = subparsers.add_parser("show", help="Show entries")
    parser_show.add_argument("component", type=str, choices=['tags', 'domains'], help="Pick a list to show available entries")
    parser_show.add_argument("--id", type=int, default=None, help="Unique id")

    # Action: insert
    parser_insert = subparsers.add_parser("insert", help="Insert an entry")
    parser_insert.add_argument("tag", type=str, help="Tag for the payload file")
    parser_insert.add_argument("domain", type=str, help="Domain of the payload file")
    parser_insert.add_argument("payload", type=str, help=f"Payload file name")
    parser_insert.add_argument("-s", "--start", type=int, default=0, help="Start of interval when the payload is applied")

    # Action: fetch
    parser_fetch = subparsers.add_parser("fetch", help="Fetch one or more payload entries")
    parser_fetch.add_argument("tag", type=NonEmptyStr, help="Tag for the payload file")
    parser_fetch.add_argument("-d", "--domain", type=NonEmptyStr, default=None, help="Domain for the payload file")
    parser_fetch.add_argument("-s", "--start", type=NonNegativeInt, default=sys.maxsize, help="A non-negative integer representing the start of interval when the payload is applied")

    # Action: config
    parser_config = subparsers.add_parser("config", help="Config")
    parser_config.add_argument("field", type=NonEmptyStr, nargs='*', help="Print configuration field value")

    args = parser.parse_args()

    global db
    db = config_db(args.config)

    if not db:
        sys.exit(os.EX_CONFIG)

    act_on(args)
