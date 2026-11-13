"""Selector subselector bookkeeping."""

import pytest

import clippy
from clippy.selectors import Selector

# G is the fake test-only backend class; see the comment in test_instances.py.
G = clippy.G


@pytest.fixture
def node():
    sel = Selector(None, "node", "nodes")
    sel._add_subselector("pagerank", "pr")
    sel._add_subselector("degree", "deg")
    return sel


def test_del_subselector_removes_attribute_and_entry(node):
    pagerank = node.pagerank
    node._del_subselector("pagerank")
    assert not hasattr(node, "pagerank")
    assert pagerank not in node._subselectors
    assert "node.pagerank" not in node._describe()
    # siblings are untouched
    assert node.degree in node._subselectors
    assert "node.degree" in node._describe()


def test_del_missing_subselector_raises(node):
    with pytest.raises(AttributeError):
        node._del_subselector("nope")
    assert {s._name for s in node._subselectors} == {"pagerank", "degree"}


def test_clear_after_del(node):
    node._del_subselector("pagerank")
    node._clear_subselectors()
    assert node._subselectors == set()
    assert not hasattr(node, "degree")


def test_backend_update_after_del(tmp_path):
    # a backend `_selectors` update clears and re-imports subselectors;
    # it must still work after a local delete.
    g = G(str(tmp_path / "g"))
    g.add_series("pagerank")
    g.node._del_subselector("pagerank")
    g.add_series("degree")
    assert hasattr(g.node, "degree")
    assert hasattr(g.node, "pagerank")  # backend still reports it
