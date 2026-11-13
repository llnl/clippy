"""Selectors must belong to instances, not to the generated class."""

import json

import pytest

import clippy
from clippy.backends.serialization import encode_clippy_json
from clippy.selectors import Selector

# G is a fake, test-only class. clippy's fs backend generates one class per
# directory on CLIPPY_BACKEND_PATH, named after the directory; conftest.py puts
# py/tests/backend on that path, so py/tests/backend/G/ becomes clippy.G.
# Its meta.json declares the top-level selectors (node, edge), and each
# executable in the directory (__init__, add_series -- the same stdlib-only
# script, dispatching on its own filename) becomes a method. add_series returns
# a `_selectors` update, which is what exercises per-instance selectors.
G = clippy.G


@pytest.fixture
def a(tmp_path):
    return G(str(tmp_path / "a"))


@pytest.fixture
def b(tmp_path):
    return G(str(tmp_path / "b"))


def test_selectors_are_distinct_objects(a, b):
    assert a.node is not b.node
    assert a.edge is not b.edge


def test_subselector_does_not_leak(a, b):
    a.add_series("pagerank")
    assert hasattr(a.node, "pagerank")
    assert not hasattr(b.node, "pagerank")


def test_cross_instance_isolation(tmp_path, a, b):
    a.add_series("pagerank")
    b.add_series("degree")
    assert not hasattr(a.node, "degree")
    assert not hasattr(b.node, "pagerank")
    c = G(str(tmp_path / "c"))
    assert not hasattr(c.node, "pagerank")
    assert not hasattr(c.node, "degree")


def test_subclass_gets_own_selectors(tmp_path, a):
    class Sub(G):
        pass

    s = Sub(str(tmp_path / "s"))
    assert isinstance(s.node, Selector)
    assert s.node is not a.node
    assert s.edge is not a.edge


def test_where_clause_serializes(a):
    a.add_series("pagerank")
    out = json.loads(json.dumps(a.node.pagerank > 3, default=encode_clippy_json))
    assert out["expression_type"] == "jsonlogic"
    assert {"var": "node.pagerank"} in out["rule"][">"]
