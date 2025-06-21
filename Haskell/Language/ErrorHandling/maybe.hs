data Maybe a = Nothing | Just a

safeDivide :: Int -> Int -> Maybe Int
safeDivide _ 0 = Nothing
safeDivide x y = Just (x `div` y)

result :: Maybe Int
result = safeDivide 10 2
-- result is Just 5

result :: Maybe Int
result = do
  x <- safeDivide 10 2
  y <- safeDivide x 2
  return y