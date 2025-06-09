data Either a b = Left a | Right b

data DivisionError = DivideByZero | NegativeValue

safeDivide :: Int -> Int -> Either DivisionError Int
safeDivide _ 0 = Left DivideByZero
safeDivide x y
  | y < 0     = Left NegativeValue
  | otherwise = Right (x `div` y)

result :: Either DivisionError Int
result = safeDivide 10 2
-- result is Right 5

handleResult :: Either DivisionError Int -> String
handleResult (Left DivideByZero) = "Error: Divide by zero."
handleResult (Left NegativeValue) = "Error: Negative value."
handleResult (Right value)        = "Success: " ++ show value