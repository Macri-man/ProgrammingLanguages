import System.IO.Error (catchIOError, ioError, userError)

safeReadFile :: FilePath -> IO (Either String String)
safeReadFile path = do
  contentOrError <- catchIOError (readFile path >>= return . Right) handleError
  return contentOrError
  where
    handleError :: IOError -> IO (Either String String)
    handleError e
      | isDoesNotExistError e = return (Left "File does not exist.")
      | isPermissionError e   = return (Left "Permission denied.")
      | otherwise             = return (Left "Unknown error.")