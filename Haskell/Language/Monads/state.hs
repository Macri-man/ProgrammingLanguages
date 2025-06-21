import Control.Monad.State

type Counter = State Int

increment :: Counter ()
increment = do
  count <- get
  put (count + 1)

runCounter :: Int -> ((), Int)
runCounter initialState = runState (increment >> increment) initialState
-- runCounter 0 results in ((), 2)