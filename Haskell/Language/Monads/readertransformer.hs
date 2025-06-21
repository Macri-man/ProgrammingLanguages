newtype ReaderT r m a = ReaderT { runReaderT :: r -> m a }

import Control.Monad.Reader
import Control.Monad.IO.Class (liftIO)

type MyReader = ReaderT String IO

example :: MyReader ()
example = do
  config <- ask
  liftIO $ putStrLn ("Config value: " ++ config)