factorial :: Int -> Int
factorial 0 = 1
factorial n = n * factorial (n - 1)

length :: [a] -> Int
length [] = 0
length (_:xs) = 1 + length xs

data Tree a = Empty | Node a (Tree a) (Tree a)

sumTree :: Num a => Tree a -> a
sumTree Empty = 0
sumTree (Node value left right) = value + sumTree left + sumTree right