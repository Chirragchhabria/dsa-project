#!/usr/bin/env python3
"""Create a deterministic 10,000-flow CIDDS-001 correlation sample."""
import argparse
import csv
import io
import random
import zipfile
from datetime import datetime, timezone
from pathlib import Path
from urllib.request import Request, urlopen

URL = "https://www.hs-coburg.de/wp-content/uploads/2024/11/CIDDS-001.zip"
MEMBER = "CIDDS-001/traffic/OpenStack/CIDDS-001-internal-week1.csv"
FIELDS = ["event_id", "timestamp", "source_ip", "destination_ip", "username", "hostname",
          "event_type", "severity", "description", "source", "scenario_label"]


class RemoteZip(io.RawIOBase):
    """Seekable HTTP range reader; keeps one 4 MiB block in memory."""
    def __init__(self, url):
        self.url = url
        response = urlopen(Request(url, method="HEAD"), timeout=60)
        self.size = int(response.headers["Content-Length"])
        self.position = 0
        self.start = -1
        self.block = b""

    def readable(self): return True
    def seekable(self): return True
    def tell(self): return self.position

    def seek(self, offset, whence=0):
        self.position = (offset if whence == 0 else
                         self.position + offset if whence == 1 else self.size + offset)
        return self.position

    def read(self, size=-1):
        if size < 0:
            size = self.size - self.position
        result = bytearray()
        while len(result) < size:
            if not self.start <= self.position < self.start + len(self.block):
                end = min(self.size - 1, self.position + 4 * 1024 * 1024 - 1)
                response = urlopen(Request(self.url, headers={
                    "Range": f"bytes={self.position}-{end}"}), timeout=90)
                if response.status != 206:
                    raise OSError("server does not support HTTP ranges")
                self.start, self.block = self.position, response.read()
            count = min(size - len(result), self.start + len(self.block) - self.position)
            result.extend(self.block[self.position - self.start:self.position - self.start + count])
            self.position += count
        return bytes(result)


def sample(zip_source, output, per_class=5000, seed=42):
    rng = random.Random(seed)
    selected = {"normal": [], "attack": []}
    seen = {"normal": 0, "attack": 0}
    with zipfile.ZipFile(zip_source) as archive, archive.open(MEMBER) as raw:
        reader = csv.DictReader(io.TextIOWrapper(raw, encoding="utf-8-sig", newline=""))
        required = {"Date first seen", "Src IP Addr", "Dst IP Addr", "class", "attackType", "attackID"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError("CIDDS-001 columns are missing")
        for index, row in enumerate(reader, 1):
            label = row["attackID"].strip()
            category = "attack" if label not in ("", "---") else "normal"
            # Only publisher-labeled normal flows serve as negatives.
            if category == "normal" and row["class"].strip() != "normal":
                continue
            seen[category] += 1
            bucket = selected[category]
            item = (index, row)
            if len(bucket) < per_class:
                bucket.append(item)
            else:
                replacement = rng.randrange(seen[category])
                if replacement < per_class:
                    bucket[replacement] = item
    if any(seen[key] < per_class for key in seen):
        raise ValueError(f"insufficient rows: {seen}")
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(FIELDS)
        for index, row in sorted(selected["normal"] + selected["attack"]):
            label = row["attackID"].strip()
            attack = label not in ("", "---")
            kind = row["attackType"].strip().lower()
            event_type = ("FAILED_LOGIN" if "brute" in kind else
                          "PORT_SCAN" if "scan" in kind else
                          "DENIAL_OF_SERVICE" if kind == "dos" else
                          "UNKNOWN" if attack else "NORMAL_ACTIVITY")
            timestamp = int(datetime.fromisoformat(row["Date first seen"].strip())
                            .replace(tzinfo=timezone.utc).timestamp())
            writer.writerow([f"CIDDS-W1-{index}", timestamp, row["Src IP Addr"].strip(),
                             row["Dst IP Addr"].strip(), "", "", event_type,
                             "HIGH" if attack else "INFORMATIONAL",
                             kind if attack else "normal flow", "CIDDS-001 internal week 1",
                             f"attack-{label}" if attack else "normal"])
    return seen


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--zip", type=Path, help="local CIDDS-001.zip; otherwise use HTTP ranges")
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()
    source = args.zip or RemoteZip(URL)
    print(sample(source, args.output, seed=args.seed), flush=True)


if __name__ == "__main__":
    main()
