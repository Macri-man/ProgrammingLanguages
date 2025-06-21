class Eq a where
  (==) :: a -> a -> Bool

instance Eq Person where
  (Person n1 a1) == (Person n2 a2) = n1 == n2 && a1 == a2