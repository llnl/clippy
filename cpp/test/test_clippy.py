# This should mirror test_clippy.py from the llnl-clippy repo.
import pytest
import sys

sys.path.append("src")

import jsonlogic as jl
import clippy
from clippy.error import ClippyValidationError, ClippyInvalidSelectorError

import logging

clippy.logger.setLevel(logging.WARN)
logging.getLogger().setLevel(logging.WARN)


@pytest.fixture()
def examplebag():
    return clippy.ExampleBag()


@pytest.fixture()
def exampleset():
    return clippy.ExampleSet()


@pytest.fixture()
def testfun():
    return clippy.ExampleFunctions()


@pytest.fixture()
def testsel():
    return clippy.ExampleSelector()


@pytest.fixture()
def examplegraph():
    return clippy.ExampleGraph()

def test_imports():
    assert "ExampleBag" in clippy.__dict__

def test_bag(examplebag):

    examplebag.insert(41)
    assert examplebag.size() == 1
    examplebag.insert(42)
    assert examplebag.size() == 2
    examplebag.insert(41)
    assert examplebag.size() == 3
    examplebag.remove(41)
    assert examplebag.size() == 2
    examplebag.remove(99)
    assert examplebag.size() == 2


def test_clippy_call_with_string(testfun):
    assert testfun.call_with_string("Seth") == "Howdy, Seth"
    with pytest.raises(ClippyValidationError):
        testfun.call_with_string()


def test_expression_gt_gte(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    assert examplebag.size() == 6
    examplebag.remove_if(examplebag.value > 51)
    assert examplebag.size() == 5
    examplebag.remove_if(examplebag.value >= 50)
    assert examplebag.size() == 3
    examplebag.remove_if(examplebag.value >= 99)
    assert examplebag.size() == 3


def test_expression_lt_lte(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if(examplebag.value < 42)
    assert examplebag.size() == 4
    examplebag.remove_if(examplebag.value <= 51)
    assert examplebag.size() == 1


def test_expression_eq_neq(examplebag):
    examplebag.insert(10).insert(11).insert(12)
    assert examplebag.size() == 3
    examplebag.remove_if(examplebag.value != 11)
    assert examplebag.size() == 1
    examplebag.remove_if(examplebag.value == 11)
    assert examplebag.size() == 0


def test_expresssion_add(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if(examplebag.value + 30 > 70)
    assert examplebag.size() == 1


def test_expression_sub(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if(examplebag.value - 30 > 0)
    assert examplebag.size() == 1


def test_expression_mul_div(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if(examplebag.value * 2 / 4 > 10)
    assert examplebag.size() == 1


def test_expression_or(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if((examplebag.value < 41) | (examplebag.value > 49))
    assert examplebag.size() == 2  # 41, 42


def test_expression_and(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if((examplebag.value > 40) & (examplebag.value < 50))
    assert examplebag.size() == 4  # 10, 50, 51, 52


# TODO: not yet implemented
# def test_expression_floordiv(examplebag):
#     examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
#     examplebag.remove_if(examplebag.value * 2 // 4.2 > 10)
#     assert examplebag.size() == 1


def test_expression_mod(examplebag):
    examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    examplebag.remove_if(examplebag.value % 2 == 0)
    assert examplebag.size() == 2


# TODO: not yet implemented
# def test_expression_pow(examplebag):
#     examplebag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
#     examplebag.remove_if(examplebag.value**2 > 1000)
#     assert examplebag.size() == 2


def test_clippy_returns_int(testfun):
    assert testfun.returns_int() == 42


def test_clippy_returns_string(testfun):
    assert testfun.returns_string() == "asdf"


def test_clippy_returns_bool(testfun):
    assert testfun.returns_bool()


def test_clippy_returns_dict(testfun):
    d = testfun.returns_dict()
    assert len(d) == 3
    assert d.get("a") == 1
    assert d.get("b") == 2
    assert d.get("c") == 3


def test_clippy_returns_vec_int(testfun):
    assert testfun.returns_vec_int() == [0, 1, 2, 3, 4, 5]


def test_clippy_returns_optional_string(testfun):
    assert testfun.call_with_optional_string() == "Howdy, World"
    assert testfun.call_with_optional_string(name="Seth") == "Howdy, Seth"


def test_selectors(testsel):
    assert hasattr(testsel, "nodes")

    testsel.add(testsel.nodes, "b", desc="docstring for nodes.b").add(
        testsel.nodes.b, "c", desc="docstring for nodes.b.c"
    )
    assert hasattr(testsel.nodes, "b")
    assert hasattr(testsel.nodes.b, "c")
    assert testsel.nodes.b.__doc__ == "docstring for nodes.b"
    assert testsel.nodes.b.c.__doc__ == "docstring for nodes.b.c"

    assert isinstance(testsel.nodes.b, jl.Variable)
    assert isinstance(testsel.nodes.b.c, jl.Variable)

    with pytest.raises(ClippyInvalidSelectorError):
        testsel.add(testsel.nodes, "_bad", desc="this is a bad selector name")

    # with pytest.raises(ClippyInvalidSelectorError):
    #     testsel.add(testsel, 'bad', desc="this is a top-level selector")


def test_graph(examplegraph):
    examplegraph.add_edge("a", "b").add_edge("b", "c").add_edge("a", "c").add_edge(
        "c", "d"
    ).add_edge("d", "e").add_edge("e", "f").add_edge("f", "g").add_edge("e", "g")

    assert examplegraph.nv() == 7
    assert examplegraph.ne() == 8

    examplegraph.add_series(examplegraph.node, "degree", desc="node degrees")
    examplegraph.degree(examplegraph.node.degree)
    c_e_only = examplegraph.dump2(examplegraph.node.degree, where=examplegraph.node.degree > 2)
    assert "c" in c_e_only and "e" in c_e_only and len(c_e_only) == 2
