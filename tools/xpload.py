#!/usr/bin/env python3

import json
import jsonschema
import requests

db_params_json = """
{
  "host":    "localhost",
  "port":    "8000",
  "apiroot": "/api/cdb_rest",
  "path":    "/path/to/payload/data"
}
"""

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

db = json.loads(db_params_json, object_hook=lambda d: DbConfig(**d))



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


def act_on(args):
    if args.action == 'show':
        result = fetch_entries(args.component, args.id)
        if result:
            print(f"Found {len(result)} entries")
            print(json.dumps(result, indent=4))


if __name__ == "__main__":
    """ Main entry point for xpload utility """
    import argparse
    parser = argparse.ArgumentParser(description="Manipulate payload entries")

    # Parse various actions
    subparsers = parser.add_subparsers(dest="action", required=True)

    # Action: show
    parser_show = subparsers.add_parser("show", help="Show entries")
    parser_show.add_argument("component", type=str, choices=['tags', 'domains', 'payloads'], help="Pick a list to show available entries")
    parser_show.add_argument("--id", type=int, default=None, help="Unique id")

    args = parser.parse_args()

    act_on(args)
