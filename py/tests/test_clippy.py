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
def testbag():
    return clippy.TestBag()


@pytest.fixture()
def testset():
    return clippy.TestSet()


@pytest.fixture()
def testfun():
    return clippy.TestFunctions()


@pytest.fixture()
def testsel():
    return clippy.TestSelector()


@pytest.fixture()
def testgraph():
    return clippy.TestGraph()


def test_imports():
    assert "TestBag" in clippy.__dict__


def test_bag(testbag):
    testbag.insert(41)
    assert testbag.size() == 1
    testbag.insert(42)
    assert testbag.size() == 2
    testbag.insert(41)
    assert testbag.size() == 3
    testbag.remove(41)
    assert testbag.size() == 2
    testbag.remove(99)
    assert testbag.size() == 2


def test_clippy_call_with_string(testfun):
    assert testfun.call_with_string("Seth") == "Howdy, Seth"
    with pytest.raises(ClippyValidationError):
        testfun.call_with_string()


def test_expression_gt_gte(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    assert testbag.size() == 6
    testbag.remove_if(testbag.value > 51)
    assert testbag.size() == 5
    testbag.remove_if(testbag.value >= 50)
    assert testbag.size() == 3
    testbag.remove_if(testbag.value >= 99)
    assert testbag.size() == 3


def test_expression_lt_lte(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if(testbag.value < 42)
    assert testbag.size() == 4
    testbag.remove_if(testbag.value <= 51)
    assert testbag.size() == 1


def test_expression_eq_neq(testbag):
    testbag.insert(10).insert(11).insert(12)
    assert testbag.size() == 3
    testbag.remove_if(testbag.value != 11)
    assert testbag.size() == 1
    testbag.remove_if(testbag.value == 11)
    assert testbag.size() == 0


def test_expresssion_add(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if(testbag.value + 30 > 70)
    assert testbag.size() == 1


def test_expression_sub(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if(testbag.value - 30 > 0)
    assert testbag.size() == 1


def test_expression_mul_div(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if(testbag.value * 2 / 4 > 10)
    assert testbag.size() == 1


def test_expression_or(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if((testbag.value < 41) | (testbag.value > 49))
    assert testbag.size() == 2  # 41, 42


def test_expression_and(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if((testbag.value > 40) & (testbag.value < 50))
    assert testbag.size() == 4  # 10, 50, 51, 52


# TODO: not yet implemented
# def test_expression_floordiv(testbag):
#     testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
#     testbag.remove_if(testbag.value * 2 // 4.2 > 10)
#     assert testbag.size() == 1


def test_expression_mod(testbag):
    testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
    testbag.remove_if(testbag.value % 2 == 0)
    assert testbag.size() == 2


# TODO: not yet implemented
# def test_expression_pow(testbag):
#     testbag.insert(10).insert(41).insert(42).insert(50).insert(51).insert(52)
#     testbag.remove_if(testbag.value**2 > 1000)
#     assert testbag.size() == 2


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


def test_graph(testgraph):
    testgraph.add_edge("a", "b").add_edge("b", "c").add_edge("a", "c").add_edge("c", "d").add_edge("d", "e").add_edge(
        "e", "f"
    ).add_edge("f", "g").add_edge("e", "g")

    assert testgraph.nv() == 7
    assert testgraph.ne() == 8

    testgraph.add_series(testgraph.node, "degree", desc="node degrees")
    testgraph.degree(testgraph.node.degree)
    c_e_only = testgraph.dump2(testgraph.node.degree, where=testgraph.node.degree > 2)
    assert "c" in c_e_only and "e" in c_e_only and len(c_e_only) == 2
