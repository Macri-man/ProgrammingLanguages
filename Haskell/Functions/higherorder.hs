applyFunction :: (a -> b) -> a -> b
applyFunction f x = f x

result = applyFunction (\x -> x * 2) 5  -- result is 10