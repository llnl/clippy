class Foo {
  int x;

public:
  int get_x() const { return x; }

  int get_x() { return x + 1; }
};
