data Maybe a = Nothing | Just a

safeDivide :: Int -> Int -> Maybe Int
safeDivide _ 0 = Nothing
safeDivide x y = Just (x `div` y)

result = Just 10 >>= \x -> safeDivide x 2
-- result is Just 5