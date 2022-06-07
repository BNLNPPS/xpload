#!/usr/bin/env python3

tags = range(1, 10)
domains = range(1, 10)
intervals = range(1, 10)

tags = range(1, 101)
domains = range(1, 11)
intervals = range(1, 1001)

def write_sql(file):

    file.write(f'truncate "GlobalTag", "GlobalTagStatus", "GlobalTagType", "PayloadIOV", "PayloadList", "PayloadListIdSequence", "PayloadType" restart identity cascade;\n')
    file.write(f'insert into "GlobalTagStatus" values (1, \'unlocked\', null, current_timestamp);\n')
    file.write(f'insert into "GlobalTagType"   values (1, \'online\', null, current_timestamp);\n\n')

    for d in domains:
        file.write(f'insert into "PayloadType" values ({d}, \'Domain_{d}\', null, current_timestamp);\n')

    d_counter = 1
    i_counter = 1

    for t in tags:
        file.write(f'\ninsert into "GlobalTag" values ({t}, \'Tag_{t}\', \'\', current_timestamp, current_timestamp, 1, 1);\n')

        for d in domains:
            file.write(f'insert into "PayloadList" values ({d_counter}, \'Domain_{d}_Tag_{t}\', null, current_timestamp, current_timestamp, {t}, {d});\n')

            file.write(f'insert into "PayloadIOV" values\n')
            for i in intervals:
                file.write(f'\t({i_counter}, \'Payload_{i}_Commit_{t}_Domain_{d}\', 0, {i}, 0, 0, null, current_timestamp, current_timestamp, {d_counter})')
                file.write(',\n' if i < len(intervals) else ';\n')
                i_counter += 1

            d_counter += 1


with open("entries.sql", 'w') as f: pass
with open("entries.sql", 'a') as f:
    write_sql(f)
