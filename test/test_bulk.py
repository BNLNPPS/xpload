#!/usr/bin/env python3

# Usage:
#
# $ test_bulk.py bulk
#
# Fill database:
#
# $ curl -X POST -H "Content-Type: application/json" -d @bulk_pils.json http://localhost:8000/api/cdb_rest/pil
# $ curl -X POST -H "Content-Type: application/json" -d @bulk_tags.json http://localhost:8000/api/cdb_rest/tag
#
# Read back:
#
# $ curl -s http://localhost:8000/api/cdb_rest/pils | python -m json.tool --indent 1 > bulk_pils_readback.json
# $ curl -s http://localhost:8000/api/cdb_rest/pils | python -m json.tool --indent 1 > bulk_pils_readback.json

import json

from collections import defaultdict

commits = range(1, 11)
domains = range(1, 101)
intervals = range(1, 1001)


def calc_hash(domain_entry):
    """ Make sure this function is the same as the one defined on the server side """
    import hashlib
    m = hashlib.sha1()
    m.update(domain_entry['domain'].encode())

    for d in domain_entry['payloads']:
        for v in d.values():
            m.update(str(v).encode())

    return m.hexdigest()


def create_pils_tags(file_id):
    entries = []
    tags = defaultdict(list)

    for c in commits:
        for d in domains:
            domain_name = f"Domain_{d}"
            payloads = []
            for i in intervals:
                payload = {
                    'name': f"Payload_{i}_Commit_{c}_{domain_name}",
                    'start': i
                }
                payloads.append(payload)

            entry = {'domain': domain_name, 'payloads': payloads}
            hexhash = calc_hash(entry)

            entries.append(entry)

            # Fill the tags dictionary, {tag1: [commit1_domain1, commit1_domain2, ...], tag2: [...]}
            tags[f"Tag_{c}"].append({'domain': domain_name, 'hexhash': hexhash})

    # Convert the tags dictionary to a list of tag objects
    tags = [dict(tag=tag, pils=pils) for tag, pils in tags.items()]
    with open(f"{file_id}_pils.json", 'w') as f:
        json.dump(entries, f, indent=1)

    with open(f"{file_id}_tags.json", 'w') as f:
        json.dump(tags, f, indent=1)


def create_tags(file_id):
    with open(f"{file_id}_readback.json", 'r') as f:
        entries = json.load(f)

    # Create a map of PILs keyed by each domain
    ll = defaultdict(list)
    for e in entries:
        ll[e['domain']].append(e['hexhash'])

    tags = []
    tag_id = 0

    has_values = True
    while has_values:
        has_values = False
        tag_id = tag_id + 1
        pils = []
        for domain in ll:
            if not ll[domain]:
                continue
            pils.append(dict(domain=domain, hexhash=ll[domain].pop()))
            if ll[domain]:
                has_values = True
        if pils:
            tags.append(dict(tag=f"Tag_{tag_id}", pils=pils))

    with open(f"{file_id}_tags.json", 'w') as f:
        json.dump(tags, f, indent=1)


import argparse

parser = argparse.ArgumentParser(description="Create payload interval lists")
parser.add_argument("outname", nargs="?", type=str, default="bulk", help="Output file base")

args = parser.parse_args()
create_pils_tags(args.outname)
