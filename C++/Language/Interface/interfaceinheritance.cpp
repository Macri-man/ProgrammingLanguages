class IShape {
public:
  virtual void draw() const = 0;
  virtual ~IShape() = default;
};

class IColoredShape : public IShape {
public:
  virtual void setColor(const std::string &color) = 0;
};

class ColoredCircle : public IColoredShape {
public:
  void draw() const override {
    std::cout << "Drawing a colored circle." << std::endl;
  }

  void setColor(const std::string &color) override {
    this->color = color;
    std::cout << "Circle color set to " << color << std::endl;
  }

private:
  std::string color;
};