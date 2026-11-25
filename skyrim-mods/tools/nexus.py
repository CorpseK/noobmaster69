"""Search Skyrim SE mods on Nexus through its public GraphQL API (no login, no API key).

Adult-flagged mods are included: the public API returns them like any other mod.

  py -I nexus.py search dress --category Armour --adult any -n 20
  py -I nexus.py search "noble outfit" --sort downloads
  py -I nexus.py info 9651
  py -I nexus.py files 9651

--adult: any (default), only, none.  --sort: endorsements (default), downloads, updated.
Search matches words in the mod name (stemmed, so "dresses" finds "dress").
"""
import argparse
import html
import json
import re
import sys
import urllib.request

API = "https://api.nexusmods.com/v2/graphql"
GAME_ID = "1704"  # skyrimspecialedition
SORTS = {"endorsements": "endorsements", "downloads": "downloads", "updated": "updatedAt"}


def gql(query, variables=None):
    body = json.dumps({"query": query, "variables": variables or {}}).encode()
    req = urllib.request.Request(API, body, {"Content-Type": "application/json", "User-Agent": "skyrim-mod-tools"})
    with urllib.request.urlopen(req, timeout=60) as r:
        data = json.load(r)
    if data.get("errors"):
        sys.exit("API error: " + "; ".join(e.get("message", "?") for e in data["errors"]))
    return data["data"]


def clean(text):
    """BBCode/HTML description -> plain text."""
    text = html.unescape(text or "")
    text = re.sub(r"<br\s*/?>", "\n", text)
    text = re.sub(r"\[url=([^\]]+)\](.*?)\[/url\]", r"\2 (\1)", text, flags=re.S)
    text = re.sub(r"\[img\].*?\[/img\]", "", text, flags=re.S | re.I)
    text = re.sub(r"\[/?[a-z]+(=[^\]]*)?\]", "", text, flags=re.I)
    text = re.sub(r"<[^>]+>", "", text)
    return re.sub(r"\n{3,}", "\n\n", text).strip()


def search(a):
    flt = {"gameId": [{"value": GAME_ID}], "nameStemmed": [{"value": a.terms}]}
    if a.category:
        flt["categoryName"] = [{"value": a.category}]
    if a.adult != "any":
        flt["adultContent"] = [{"value": a.adult == "only"}]
    q = """query($f: ModsFilter, $n: Int, $s: [ModsSort!]) {
      mods(filter: $f, count: $n, sort: $s) { totalCount
        nodes { modId name author category adultContent endorsements downloads updatedAt summary } } }"""
    d = gql(q, {"f": flt, "n": a.n, "s": [{SORTS[a.sort]: {"direction": "DESC"}}]})["mods"]
    print(f"{d['totalCount']} result(s), showing {len(d['nodes'])}")
    for m in d["nodes"]:
        flag = " [adult]" if m["adultContent"] else ""
        print(f"\n{m['modId']:>7}  {m['name']}{flag}")
        print(f"         {m['category']} | by {m['author']} | {m['endorsements']} endorsements | "
              f"{m['downloads']} downloads | updated {m['updatedAt'][:10]}")
        print(f"         https://www.nexusmods.com/skyrimspecialedition/mods/{m['modId']}")
        if m["summary"]:
            print("         " + clean(m["summary"])[:300])


def info(a):
    q = """query($f: ModsFilter) { mods(filter: $f, count: 1) {
      nodes { modId name author category version adultContent endorsements downloads updatedAt summary description } } }"""
    nodes = gql(q, {"f": {"gameId": [{"value": GAME_ID}], "modId": [{"value": str(a.id)}]}})["mods"]["nodes"]
    if not nodes:
        sys.exit("not found")
    m = nodes[0]
    print(f"{m['name']} (v{m['version']}) by {m['author']} | {m['category']}"
          f"{' | adult' if m['adultContent'] else ''} | updated {m['updatedAt'][:10]}")
    print(f"https://www.nexusmods.com/skyrimspecialedition/mods/{m['modId']}\n")
    print(clean(m["summary"]) + "\n")
    print(clean(m["description"])[: a.chars])


def files(a):
    q = """query($id: ID!, $g: ID!) { modFiles(modId: $id, gameId: $g) {
      name version category sizeInBytes description } }"""
    for f in gql(q, {"id": str(a.id), "g": GAME_ID})["modFiles"]:
        if f["category"] in ("REMOVED", "ARCHIVED") and not a.all:
            continue
        mb = int(f["sizeInBytes"] or 0) / 1e6
        print(f"[{f['category']}] {f['name']} v{f['version']} ({mb:.1f} MB)")
        if f["description"]:
            print("    " + clean(f["description"])[:300])


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("search")
    s.add_argument("terms")
    s.add_argument("--category", help='e.g. "Armour", "Clothing and Accessories"')
    s.add_argument("--adult", choices=["any", "only", "none"], default="any")
    s.add_argument("--sort", choices=list(SORTS), default="endorsements")
    s.add_argument("-n", type=int, default=15)
    s.set_defaults(fn=search)
    i = sub.add_parser("info")
    i.add_argument("id", type=int)
    i.add_argument("--chars", type=int, default=4000, help="max description length")
    i.set_defaults(fn=info)
    f = sub.add_parser("files")
    f.add_argument("id", type=int)
    f.add_argument("--all", action="store_true", help="include removed/archived files")
    f.set_defaults(fn=files)
    a = p.parse_args()
    sys.stdout.reconfigure(encoding="utf-8")
    a.fn(a)


if __name__ == "__main__":
    main()
