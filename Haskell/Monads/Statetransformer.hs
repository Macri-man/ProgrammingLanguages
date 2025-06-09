newtype StateT s m a = StateT { runStateT :: s -> m (a, s) }

import Control.Monad.State
import Control.Monad.IO.Class (liftIO)

type MyState = StateT Int IO

increment :: MyState ()
increment = do
  count <- get
  put (count + 1)

example :: MyState ()
example = do
  liftIO $ putStrLn "Incrementing state..."
  increment
  count <- get
  liftIO $ putStrLn ("Current count: " ++ show count)