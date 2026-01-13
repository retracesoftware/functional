import retracesoftware.functional as fn


def test_memoize_one_arg_caches_by_identity():
    calls = []

    def target(x):
        calls.append(id(x))
        return f"val-{id(x)}"

    memo = fn.memoize_one_arg(target)
    obj = object()

    assert memo(obj) == f"val-{id(obj)}"
    assert memo(obj) == f"val-{id(obj)}"
    assert calls == [id(obj)]


def test_cache_stores_non_none_and_recomputes_none_results():
    lookups = []

    def lookup(x):
        lookups.append(x)
        return None if x == "miss" else f"hit-{x}"

    cache = fn.Cache(lookup)

    assert cache("abc") == "hit-abc"
    assert cache("abc") == "hit-abc"
    assert lookups == ["abc"]

    assert cache("miss") is None
    assert cache("miss") is None
    assert lookups == ["abc", "miss", "miss"]

