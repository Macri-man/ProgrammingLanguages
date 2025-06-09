class IMovable {
public:
  virtual void move(int x, int y) = 0;
  virtual ~IMovable() = default;
};

class IResizable {
public:
  virtual void resize(int width, int height) = 0;
  virtual ~IResizable() = default;
};

class ResizableMovableObject : public IMovable, public IResizable {
public:
  void move(int x, int y) override {
    std::cout << "Moving object to (" << x << ", " << y << ")" << std::endl;
  }

  void resize(int width, int height) override {
    std::cout << "Resizing object to " << width << "x" << height << std::endl;
  }
};