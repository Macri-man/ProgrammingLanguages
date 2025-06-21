newtype MaybeT m a = MaybeT { runMaybeT :: m (Maybe a) }

import Control.Monad.Trans.Maybe
import Control.Monad.IO.Class (liftIO)

example :: MaybeT IO ()
example = do
  liftIO $ putStrLn "Trying to fetch a value..."
  x <- MaybeT $ return (Just 42)
  liftIO $ putStrLn ("Fetched value: " ++ show x)