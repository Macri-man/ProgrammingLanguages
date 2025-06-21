type Error = String

safeDivide :: Int -> Int -> Either Error Int
safeDivide _ 0 = Left "Division by zero"
safeDivide x y = Right (x `div` y)

compute :: Either Error Int
compute = do
  x <- safeDivide 10 2
  y <- safeDivide x 2
  return (y + 1)