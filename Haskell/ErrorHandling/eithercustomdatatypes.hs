data MyError = FileNotFound | PermissionDenied | UnknownError String

readFileContents :: FilePath -> IO (Either MyError String)
readFileContents path = do
  contentOrError <- try (readFile path) :: IO (Either IOException String)
  return $ case contentOrError of
    Left _      -> Left FileNotFound
    Right content -> Right content