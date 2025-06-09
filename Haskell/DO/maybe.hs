safeDivide :: Int -> Int -> Maybe Int
safeDivide _ 0 = Nothing
safeDivide x y = Just (x `div` y)

compute :: Maybe Int
compute = do
  x <- safeDivide 10 2
  y <- safeDivide x 2
  return (y + 1)