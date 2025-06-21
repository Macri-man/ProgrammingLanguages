class Monad m => MonadError e m where
  throwError :: e -> m a
  catchError :: m a -> (e -> m a) -> m a

import Control.Monad.Trans.Except
import Control.Monad.IO.Class (liftIO)

type MyExceptT = ExceptT String IO

example :: MyExceptT ()
example = do
  liftIO $ putStrLn "Doing something..."
  throwError "An error occurred"

runExample :: IO ()
runExample = do
  result <- runExceptT example
  case result of
    Left err  -> putStrLn ("Error: " ++ err)
    Right _   -> putStrLn "Success!"