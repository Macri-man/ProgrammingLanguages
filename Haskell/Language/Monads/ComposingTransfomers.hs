type MyStack = StateT Int (MaybeT IO)

import Control.Monad.State
import Control.Monad.Trans.Maybe
import Control.Monad.IO.Class (liftIO)

type MyMonad = StateT Int (MaybeT IO)

increment :: MyMonad ()
increment = do
  count <- get
  put (count + 1)

example :: MyMonad ()
example = do
  liftIO $ putStrLn "Starting..."
  increment
  count <- get
  liftIO $ putStrLn ("Current count: " ++ show count)
  return ()

runExample :: IO ()
runExample = do
  result <- runMaybeT (runStateT example 0)
  case result of
    Just ((), finalState) -> putStrLn ("Final state: " ++ show finalState)
    Nothing -> putStrLn "Computation failed."