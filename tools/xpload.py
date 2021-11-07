#!/usr/bin/env python3

import json
import jsonschema
import os
import requests
import sys

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

class DbConfig(namedtuple('DbConfig', ['host', 'port', 'apiroot', 'path'])):
    def url(self):
        return "http://" + self.host + ':' + self.port + self.apiroot


def config_db(config_name):
    """ Read database parameters from a json config file """

    # Use user supplied config as is if it looks like a "path"
    if "." in config_name or "/" in config_name:
        with open(f"{config_name}") as cfgf:
            return json.load(cfgf, object_hook=lambda d: DbConfig(**d))

    XPLOAD_DIR = os.getenv('XPLOAD_DIR', "")
    XPLOAD_CONFIG_NAME = os.getenv('XPLOAD_CONFIG_NAME', "test")
    XPLOAD_CONFIG_SEARCH_PATHS = [".", "config"]

    if XPLOAD_DIR:
        search_paths = [f"{XPLOAD_DIR.rstrip('/')}/{cfgpath}" for cfgpath in XPLOAD_CONFIG_SEARCH_PATHS] + XPLOAD_CONFIG_SEARCH_PATHS
    else:
        search_paths = XPLOAD_CONFIG_SEARCH_PATHS

    if config_name:
        config_file = f"{config_name}.json"
    else:
        config_file = f"{XPLOAD_CONFIG_NAME}.json"

    for config_path in search_paths:
        try:
            with open(f"{config_path}/{config_file}") as cfgf:
                return json.load(cfgf, object_hook=lambda d: DbConfig(**d))
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
        print(f"Error: Something went wrong while posting data to {url}")
        return None

    try:
        jsonschema.validate(respjson, general_schema['definitions']['entry'])
    except:
        print(f"Error: Encountered invalid response")
        return None

    return respjson['id']


def create_tag_type(name="test"):
    return _post_data('gttype', {"name": name, "id": 1})

def create_tag_status(name="test"):
    return _post_data('gtstatus', {"name": name, "id": 1})

def create_tag(name):
    return _post_data('gt', {"name": name})

def create_domain(name, id):
    return _post_data('pt', {"name": name, "id": id})

def create_domain_list(name, tag_id, domain_id):
    return _post_data('pl', {"name": name, "global_tag": tag_id, "payload_type": domain_id})

def create_payload(name, domain_list_id, start):
    return _post_data('piov', {"payload_url": name, "payload_list": domain_list_id, "major_iov": 0, "minor_iov": start})


def fetch_entries(component: str, tag_id: int = None):
    """ Fetch and print entries from respective table """
    if component == 'tags':
        url = db.url() + "/gt"
    elif component == 'domains':
        url = db.url() + "/pt"
    elif component == 'domain_lists':
        url = db.url() + "/pl"
    elif component == 'payloads':
        url = db.url() + "/piov"
    else:
        print(f"Error: Wrong component {component}")
        return []

    if tag_id is not None:
        url += f"/{tag_id}"

    try:
        response = requests.get(url)
        respjson = response.json()
    except:
        print(f"Error: Something went wrong while looking for tag id {tag_id}. Check", db.url())
        return []

    # Always return a list
    entries = respjson if isinstance(respjson, list) else [respjson]

    try:
        jsonschema.validate(entries, general_schema)
    except:
        print(f"Error: Encountered invalid response. Tag id {tag_id} may not exist")
        return []

    return entries


def push_payload(tag: str, domain: str, payload: str, start: int = 0):
    """ Inserts an entry into corresponding tables """

    # Get all tags
    tags = fetch_entries("tags")
    # Select the last matching entry
    existing_tag = next((e for e in reversed(tags) if e['name'] == tag), None)

    # Get all domains
    domains = fetch_entries("domains")
    last_domain_id = int(domains[-1]['id']) if domains else 0
    # Select the last matching entry
    existing_domain = next((e for e in reversed(domains) if e['name'] == domain), None)

    # If the tag does not exist create one
    if existing_tag is None:
        tag_id = create_tag(tag)
        print(f"Tag {tag} does not exist. Created {tag_id}")
    else:
        tag_id = existing_tag['id']

    # If the domain does not exist create one
    if existing_domain is None:
        domain_id = create_domain(domain, last_domain_id+1)
        print(f"Domain {domain} does not exist. Created {domain_id}")
    else:
        domain_id = existing_domain['id']

    # Check if domain_list with tag_id and domain_id exists
    domain_lists = fetch_entries("domain_lists")
    # Select the last matching entry
    existing_domain_list = next((e for e in reversed(domain_lists) if e['global_tag'] == tag_id and e['payload_type'] == domain_id), None)

    # If either tag or domain did not exist, create a new domain_list
    if existing_tag is None or existing_domain is None or existing_domain_list is None:
        name = f"{tag}_{domain}"
        domain_list_id = create_domain_list(name, tag_id, domain_id)
    else:
        domain_list_id = existing_domain_list['id']

    # Get all payloads
    payloads = fetch_entries("payloads")
    # Select the last matching entry
    existing_payload = next((e for e in reversed(payloads) if e['payload_url'] == payload and e['payload_list'] == domain_list_id), None)

    if existing_payload is None:
        payload_id = create_payload(payload, domain_list_id, start)
        print(f"Payload {payload} does not exist. Created {payload_id}")


def act_on(args):
    if args.action == 'show':
        result = fetch_entries(args.component, args.id)
        if result:
            print(f"Found {len(result)} entries")
            print(json.dumps(result, indent=4))

    if args.action == 'push':
        push_payload(args.tag, args.domain, args.payload, args.start)


if __name__ == "__main__":
    """ Main entry point for xpload utility """
    import argparse
    parser = argparse.ArgumentParser(description="Manipulate payload entries")
    parser.add_argument("-c", "--config", type=str, default="", help="Config file with database connection parameters")

    # Parse various actions
    subparsers = parser.add_subparsers(dest="action", required=True)

    # Action: show
    parser_show = subparsers.add_parser("show", help="Show entries")
    parser_show.add_argument("component", type=str, choices=['tags', 'domains', 'payloads'], help="Pick a list to show available entries")
    parser_show.add_argument("--id", type=int, default=None, help="Unique id")

    # Action: push
    parser_push = subparsers.add_parser("push", help="Insert entry")
    parser_push.add_argument("tag", type=str, help="Tag for the payload file")
    parser_push.add_argument("domain", type=str, help="Domain of the payload file")
    parser_push.add_argument("payload", type=str, help=f"Payload file name")
    parser_push.add_argument("-s", "--start", type=int, default=0, help="Start of interval when the payload is applied")

    args = parser.parse_args()

    global db
    db = config_db(args.config)

    if not db:
        sys.exit(os.EX_CONFIG)

    act_on(args)
