listMonad :: [Int] -> [Int]
listMonad xs = xs >>= \x -> [x, x + 1]
-- listMonad [1, 2] results in [1, 2, 2, 3]