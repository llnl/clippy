"""Keypath subpath bookkeeping."""

import pytest

import clippy
from clippy.keypaths import Keypath

# G is the fake test-only backend class; see the comment in test_instances.py.
G = clippy.G


@pytest.fixture
def node():
    kp = Keypath(None, "node", "nodes")
    kp._add_subpath("pagerank", "pr")
    kp._add_subpath("degree", "deg")
    return kp


def test_del_subpath_removes_attribute_and_entry(node):
    pagerank = node.pagerank
    node._del_subpath("pagerank")
    assert not hasattr(node, "pagerank")
    assert pagerank not in node._subpaths
    assert "node.pagerank" not in node._describe()
    # siblings are untouched
    assert node.degree in node._subpaths
    assert "node.degree" in node._describe()


def test_del_missing_subpath_raises(node):
    with pytest.raises(AttributeError):
        node._del_subpath("nope")
    assert {s._name for s in node._subpaths} == {"pagerank", "degree"}


def test_clear_after_del(node):
    node._del_subpath("pagerank")
    node._clear_subpaths()
    assert node._subpaths == set()
    assert not hasattr(node, "degree")


def test_backend_update_after_del(tmp_path):
    # a backend `_keypaths` update clears and re-imports subpaths;
    # it must still work after a local delete.
    g = G(str(tmp_path / "g"))
    g.add_series("pagerank")
    g.node._del_subpath("pagerank")
    g.add_series("degree")
    assert hasattr(g.node, "degree")
    assert hasattr(g.node, "pagerank")  # backend still reports it
